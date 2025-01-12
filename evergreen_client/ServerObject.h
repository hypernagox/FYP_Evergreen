#pragma once
#include "pch.h"
#include "ServerComponent.h"
#include "Component.h"
#include "NaviAgent.h"

using udsdx::Component;
class ServerComponent;

class ServerObject : public Component
{
public:
	ServerObject(const std::shared_ptr<SceneObject>& object);
	~ServerObject();
public:
	virtual void Update(const udsdx::Time& time, udsdx::Scene& scene) override;

	void SetObjID(const uint32_t id) { m_objID = id; }
	const uint32_t GetObjID()const noexcept {
		// TODO: 유효성체크 반드시
		//NET_NAGOX_ASSERT_LOG(0 != m_objID, "Obj ID must be Bigger than Zero");
		return m_objID;
	}

	template <typename T> 
	T* const AddComp()noexcept {
		return static_cast<T* const>(AddComp(T::GetCompTypeNameGlobal(), new T{ this }));
	}
	template <typename T> 
	T* const GetComp()const noexcept {
		return static_cast<T* const>(GetComp(T::GetCompTypeNameGlobal()));
	}
	template <typename T= Component> requires std::derived_from<T,Component>
	void ServerCompUpdateALL()const noexcept {
		if constexpr (false == g_bUseNetWork)return;
		for (const auto& pComp : m_mapServerComp | std::views::values)pComp->Update();
	}
	template<typename T>
	void ServerCompUpdate()const noexcept {
		if constexpr (false == g_bUseNetWork)return;
		if (const auto comp = GetComp<T>())
			comp->Update();
	}
private:
	ServerComponent* const AddComp(const uint64_t comp_id, ServerComponent* const pComp)noexcept;
private:
	ServerComponent* const GetComp(const uint64_t comp_id)const noexcept {
		const auto iter = m_mapServerComp.find(comp_id);
		return m_mapServerComp.cend() != iter ? iter->second.get() : nullptr;
	}
	// TODO: 어따 놓을지 몰라서 임시
public:
	Common::NaviAgent* m_pNaviAgent = nullptr;
private:
	
	uint32_t m_objID = 0;
	std::unordered_map<uint64_t, std::unique_ptr<ServerComponent>> m_mapServerComp;
};

