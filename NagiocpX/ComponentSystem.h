#pragma once
#include "NagiocpXPch.h"
#include "NagoxAtomic.h"

class ContentsComponent;
class ContentsUpdateComponent;

class ComponentSystem
{
	friend class NagiocpX::ContentsEntity;
public:
	ComponentSystem(const NagoxAtomic::Atomic<bool>& bValidFlag_)noexcept;
	~ComponentSystem()noexcept;
public:
	template <typename T>
	constexpr inline T* const GetComp()const noexcept {
		return static_cast<T* const>(GetCompInternal<T::GetCompTypeNameGlobal()>());
	}
	inline const bool IsOwnerValid()const noexcept { return m_bOwnerValidFlag.load(); }
	void ProcessCleanUp()const noexcept;
	void ShrinkToFit()noexcept {
		m_contentsComponents.shrink_to_fit();
		m_vecUpdateComponents.shrink_to_fit();
	}
protected:
	void Update(const float dt_)const noexcept;
private:
	template <typename T, typename... Args>
	T* const AddComp(Args&&... args)noexcept {
		const auto comp_ptr = NagiocpX::xnew<T>(std::forward<Args>(args)...);
		NAGOX_ASSERT(m_contentsComponents.end() == std::ranges::find_if(m_contentsComponents, [](const auto& ele)noexcept {
			return ele.first == T::GetCompTypeNameGlobal(); }));
		m_contentsComponents.emplace_back(T::GetCompTypeNameGlobal(), comp_ptr);
		if constexpr (std::derived_from<T, ContentsUpdateComponent>)
			m_vecUpdateComponents.emplace_back(comp_ptr);
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
	NagiocpX::XVector<std::pair<const uint64_t, ContentsComponent* const>> m_contentsComponents;
	const NagoxAtomic::Atomic<bool>& m_bOwnerValidFlag;
	NagiocpX::XVector<ContentsUpdateComponent*> m_vecUpdateComponents;
};

class ComponentSystemNPC
	:public ComponentSystem
{
	friend class TickTimerBT;
	friend class TickTimerFSM;
public:
	ComponentSystemNPC(const NagoxAtomic::Atomic<bool>& bValidFlag_, NagiocpX::ContentsEntity* const pOwner_)noexcept
		: ComponentSystem{ bValidFlag_ }
		, m_pOwnerEntity{ pOwner_ }
	{}
	~ComponentSystemNPC()noexcept = default;
public:
	inline NagiocpX::ContentsEntity* const GetOwnerEntity()const noexcept { return m_pOwnerEntity; }
private:
	NagiocpX::ContentsEntity* const m_pOwnerEntity;
};