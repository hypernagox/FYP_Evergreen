#pragma once
#include "ServerCorePch.h"
#include "NagoxAtomic.h"

class ContentsComponent;
class ContentsUpdateComponent;

class ComponentSystem
{
	friend class ServerCore::ContentsEntity;
public:
	ComponentSystem(const NagoxAtomic::Atomic<bool>& bValidFlag_)noexcept;
	~ComponentSystem()noexcept;
public:
	template <typename T>
	constexpr inline T* const GetComp()const noexcept {
		return static_cast<T* const>(GetCompInternal<T::GetCompTypeNameGlobal()>());
	}
	inline const bool IsOwnerValid()const noexcept { return m_bOwnerValidFlag.load(); }
protected:
	void Update(const float dt_)const noexcept;
private:
	template <typename T, typename... Args>
	T* const AddComp(Args&&... args)noexcept {
		const auto comp_ptr = ServerCore::xnew<T>(std::forward<Args>(args)...);
		NAGOX_ASSERT(m_contentsComponents.end() == std::ranges::find_if(m_contentsComponents, [](const auto& ele)noexcept {
			return ele.first == T::GetCompTypeNameGlobal(); }));
		m_contentsComponents.emplace_back(T::GetCompTypeNameGlobal(), comp_ptr);
		if constexpr (std::derived_from<T, ContentsUpdateComponent>)
			m_vecUpdateComponents.emplace_back(comp_ptr);
		// TODO: shrink to fit
		return comp_ptr;
	}
	template <const uint64_t COMP_ID>
	constexpr inline ContentsComponent* const GetCompInternal()const noexcept {
		auto b = m_contentsComponents.data();
		const auto e = b + m_contentsComponents.size();
		do {
			if (COMP_ID == b->first)return b->second;
		} while (e != ++b);
		return nullptr;
	}
private:
	ServerCore::XVector<std::pair<const uint64_t, ContentsComponent* const>> m_contentsComponents;
	const NagoxAtomic::Atomic<bool>& m_bOwnerValidFlag;
	ServerCore::XVector<ContentsUpdateComponent*> m_vecUpdateComponents;
};

class ComponentSystemNPC
	:public ComponentSystem
{
	friend class TickTimerBT;
public:
	ComponentSystemNPC(const NagoxAtomic::Atomic<bool>& bValidFlag_, ServerCore::ContentsEntity* const pOwner_)noexcept
		: ComponentSystem{ bValidFlag_ }
		, m_pOwnerEntity{ pOwner_ }
	{}
	~ComponentSystemNPC()noexcept = default;
public:
	inline ServerCore::ContentsEntity* const GetOwnerEntity()const noexcept { return m_pOwnerEntity; }
private:
	ServerCore::ContentsEntity* const m_pOwnerEntity;
};