#pragma once

class TickTimerBT;

enum class NodeStatus : uint8_t
{
    SUCCESS,
    FAILURE,
    RUNNING
};

class BehaviorNode
{
public:
    virtual ~BehaviorNode()noexcept = default;
    virtual NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker)noexcept = 0;
    virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept = 0;
};

class ConditionNode
    : public BehaviorNode
{
public:
    NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker)noexcept override = 0;
private:
    virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept override {}
};

class ActionNode :
    public BehaviorNode
{
public:
    NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker)noexcept override = 0;
private:
    virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept override = 0;
};

class DecoratorNode 
    : public BehaviorNode 
{
public:
    DecoratorNode(BehaviorNode* const child_node)noexcept :m_childNode{ child_node } {}
    ~DecoratorNode()noexcept { ServerCore::xdelete<BehaviorNode>(m_childNode); }
    virtual NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker)noexcept override = 0;
    virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept override = 0;
protected:
    BehaviorNode* const m_childNode;
};

class RepeaterNode
    : public DecoratorNode
{
public:
    RepeaterNode(BehaviorNode* const child_node)noexcept :DecoratorNode{ child_node } {}
    virtual NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker)noexcept override { return ReapeatTick(m_childNode->Tick(owner_comp_sys, bt_root_timer, awaker)); }
    virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept override = 0;
private:
    virtual NodeStatus ReapeatTick(const NodeStatus action_result)noexcept = 0;
};

class InverterNode
    : public DecoratorNode
{
public:
    InverterNode(BehaviorNode* const child_node)noexcept :DecoratorNode{ child_node } {}
public:
    virtual NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker)noexcept override {
        const NodeStatus status = m_childNode->Tick(owner_comp_sys, bt_root_timer, awaker);
        const uint8_t is_running = (NodeStatus::RUNNING == status);
        return static_cast<const NodeStatus>((!static_cast<const bool>(status)) + is_running + is_running);
    }
    virtual void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept override { m_childNode->Reset(owner_comp_sys, bt_root_timer); }
};

class CompositeNode
    : public BehaviorNode
{
public:
    virtual ~CompositeNode()noexcept {
        auto b = m_vecChildren.data();
        const auto e = b + m_vecChildren.size();
        while (e != b) { ServerCore::xdelete<BehaviorNode>(*b++); }
    }

    template <typename T, typename... Args>
    T* const AddChild(Args&&... args)noexcept {
        return static_cast<T* const>(m_vecChildren.emplace_back(ServerCore::xnew<T>(std::forward<Args>(args)...)));
    }

    void Reset(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer)noexcept override {
        m_runningChildIndex = 0;
        auto b = m_vecChildren.data();
        const auto e = b + m_vecChildren.size();
        while (e != b) { (*b++)->Reset(owner_comp_sys, bt_root_timer); }
    }
protected:
    ServerCore::Vector<BehaviorNode*> m_vecChildren;
    uint8_t m_runningChildIndex = 0;
};

class SequenceNode
    : public CompositeNode
{
public:
    NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker)noexcept override final
    {
        const auto children = m_vecChildren.data();
        const uint8_t numOfChildren = static_cast<const uint8_t>(m_vecChildren.size());
        for (uint8_t i = m_runningChildIndex; i < numOfChildren; ++i)
        {
            const NodeStatus status = children[i]->Tick(owner_comp_sys, bt_root_timer, awaker);
            if (NodeStatus::SUCCESS != status)
            {
                m_runningChildIndex = i & (-(NodeStatus::RUNNING == status));
                return status;
            }
        }
        m_runningChildIndex = 0;
        return NodeStatus::SUCCESS;
    }
};

class SelectorNode
    : public CompositeNode
{
public:
    NodeStatus Tick(const ComponentSystemNPC* const owner_comp_sys, TickTimerBT* const bt_root_timer, const ServerCore::S_ptr<ServerCore::ContentsEntity>& awaker)noexcept override final
    {
        const auto children = m_vecChildren.data();
        const uint8_t numOfChildren = static_cast<const uint8_t>(m_vecChildren.size());
        for (uint8_t i = m_runningChildIndex; i < numOfChildren; ++i)
        {
            const NodeStatus status = children[i]->Tick(owner_comp_sys, bt_root_timer, awaker);
                if (NodeStatus::FAILURE != status)
            {
                m_runningChildIndex = i & (-(NodeStatus::RUNNING == status));
                return status;
            }
        }
        m_runningChildIndex = 0;
        return NodeStatus::FAILURE;
    }
};