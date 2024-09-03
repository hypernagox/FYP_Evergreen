#pragma once
#include "ServerCorePch.h"

class ContentsComponent;
class ContentsUpdateComponent;

class ComponentSystem
{
	friend class ServerCore::ContentsEntity;
public:
	ComponentSystem(const std::atomic_bool& bValidFlag_)noexcept :m_bOwnerValidFlag{ bValidFlag_ } {}
	~ComponentSystem()noexcept;
public:
	template <typename T>
	constexpr inline T* const GetComp()const noexcept {
		return static_cast<T* const>(GetCompInternal<T::GetCompTypeNameGlobal()>());
	}
	inline const bool IsOwnerValid()const noexcept { return m_bOwnerValidFlag.load(std::memory_order_acquire); }
protected:
	void Update(const float dt_)const noexcept;
private:
	template <typename T, typename... Args>
	T* const AddComp(Args&&... args)noexcept {
		const auto comp_ptr = xnew<T>(std::forward<Args>(args)...);
		NAGOX_ASSERT(m_mapContentsComponents.try_emplace(T::GetCompTypeNameGlobal(), comp_ptr).second);
		if constexpr (std::derived_from<T, ContentsUpdateComponent>)
			m_vecUpdateComponents.emplace_back(comp_ptr);
		return comp_ptr;
	}
	template <const uint64_t COMP_ID>
	constexpr inline ContentsComponent* const GetCompInternal()const noexcept {
		return const_cast<ComponentSystem* const>(this)->m_mapContentsComponents[COMP_ID];
	}
private:
	ServerCore::Map<uint64_t, ContentsComponent* const> m_mapContentsComponents;
	const std::atomic_bool& m_bOwnerValidFlag;
	ServerCore::Vector<ContentsUpdateComponent*> m_vecUpdateComponents;
};

class ComponentSystemEX
	:public ComponentSystem
{
	friend class TickTimerBT;
public:
	ComponentSystemEX(const std::atomic_bool& bValidFlag_, ServerCore::ContentsEntity* const pOwner_)noexcept
		: ComponentSystem{ bValidFlag_ }
		, m_pOwnerEntity{ pOwner_ }
	{}
	~ComponentSystemEX()noexcept = default;
public:
	inline ServerCore::ContentsEntity* const GetOwnerEntity()const noexcept { return m_pOwnerEntity; }
private:
	ServerCore::ContentsEntity* const m_pOwnerEntity;
};