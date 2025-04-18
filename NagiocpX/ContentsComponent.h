#pragma once
#include "RefCountable.h"

class NagiocpX::ContentsEntity;
class ComponentSystem;

class ContentsComponent
{
protected:
	template<typename T>
	static const bool CheckHashCollision()noexcept {
		static std::set<uint64_t> g_setForCheckHashCollision;
		static std::mutex g_checkHashCollisionMutex;
		g_checkHashCollisionMutex.lock();
		if (!g_setForCheckHashCollision.emplace(T::GetCompTypeNameGlobal()).second) {
			NagiocpX::LogStackTrace();
			std::cout << "Dectected Hash Collision !!: " << __FILE__ << '\n' << "LINE: " << __LINE__ << '\n';
			g_checkHashCollisionMutex.unlock();
			NAGOX_ASSERT(false);
			return false;
		}
		g_checkHashCollisionMutex.unlock();
		return true;
	}
public:
	ContentsComponent(NagiocpX::ContentsEntity* const pOwner_)noexcept :m_pOwnerEntity{ pOwner_ } {}
	virtual ~ContentsComponent()noexcept = default;
public:
	virtual void ProcessCleanUp()noexcept {}
public:
	template<typename ComponentFunc, typename... Args> requires std::is_member_function_pointer_v<ComponentFunc>
	constexpr inline void EnqueueCompTaskPushOnly(ComponentFunc&& comp_func,Args&&... args)noexcept{
		m_pOwnerEntity->GetQueueabler<class Queueabler>()->EnqueueAsyncPushOnly(std::forward<ComponentFunc>(comp_func), this, std::forward<Args>(args)...);
	}
	template<typename ComponentFunc, typename... Args> requires std::is_member_function_pointer_v<ComponentFunc>
	constexpr inline void EnqueueCompTask(ComponentFunc&& comp_func, Args&&... args)noexcept {
		m_pOwnerEntity->GetQueueabler<class Queueabler>()->EnqueueAsync(std::forward<ComponentFunc>(comp_func), this, std::forward<Args>(args)...);
	}
	template<typename ComponentFunc, typename... Args> requires std::is_member_function_pointer_v<ComponentFunc>
	constexpr inline void EnqueueCompTaskTimer(const uint64_t tickAfter, ComponentFunc&& comp_func, Args&&... args)noexcept {
		m_pOwnerEntity->GetQueueabler<class Queueabler>()->EnqueueAsyncTimer(tickAfter, std::forward<ComponentFunc>(comp_func), this, std::forward<Args>(args)...);
	}
public:
	NagiocpX::ContentsEntity* const GetOwnerEntityRaw()const noexcept { return m_pOwnerEntity; }
	inline NagiocpX::S_ptr<NagiocpX::ContentsEntity> GetOwnerEntity()const noexcept { return NagiocpX::S_ptr<NagiocpX::ContentsEntity>{m_pOwnerEntity}; }
	constexpr virtual const uint64_t GetCompType()const noexcept = 0;
	inline const bool IsValid()const noexcept { return m_pOwnerEntity->IsValid(); }
public:
	template<typename T = uint8_t> requires (std::is_enum_v<T> || std::same_as<T, uint8_t>) && (sizeof(T) == sizeof(uint8_t))
	constexpr inline const T GetOwnerPrimaryGroup()const noexcept { return m_pOwnerEntity->GetPrimaryGroupType<T>(); }
	constexpr inline const uint32_t GetOwnerObjectID()const noexcept { return m_pOwnerEntity->GetObjectID(); }
	NagiocpX::ClusterFieldInfo GetOwnerClusterInfo()const noexcept { return m_pOwnerEntity->GetClusterFieldInfo(); }
private:
	NagiocpX::ContentsEntity* const m_pOwnerEntity;
};

class ContentsUpdateComponent
	:public ContentsComponent
{
public:
	ContentsUpdateComponent(NagiocpX::ContentsEntity* const pOwner_)noexcept :ContentsComponent{ pOwner_ } {}
	virtual ~ContentsUpdateComponent()noexcept = default;
public:
	virtual void Update(const ComponentSystem* const owner_comp_sys, const float dt_)noexcept = 0;
};

consteval inline const uint64_t fnv1a_hash64(const char* const str, const uint64_t hash = 0xcbf29ce484222325u) noexcept {
	return (*str == 0) ? hash : fnv1a_hash64(str + 1, (hash ^ static_cast<uint64_t>(*str)) * 0x100000001b3u);
}

#define USE_CHECK_HASH_COLLISION

#ifdef USE_CHECK_HASH_COLLISION
#define CHECK_HASH_COLLISION(ComponentType) static inline const bool g_flagForCheckHashCollision = CheckHashCollision<ComponentType>();
#else
#define CHECK_HASH_COLLISION(ComponentType)
#endif

#define COMPONENT_ID(ComponentType) static constexpr inline const uint64_t COMP_ID = fnv1a_hash64(#ComponentType);
#define GET_COMP_TYPE_FUNC_GLOBAL(ComponentType) static inline consteval const uint64_t GetCompTypeNameGlobal() noexcept { return ComponentType::COMP_ID; }
#define GET_COMP_TYPE_FUNC_VIRTUAL(ComponentType) constexpr virtual const uint64_t GetCompType()const noexcept override { return ComponentType::COMP_ID; }

#define DECLARE_COMPONENT_TYPE(ComponentType)										\
	private:																		\
	COMPONENT_ID(ComponentType)														\
	CHECK_HASH_COLLISION(ComponentType)												\
	public:																			\
	GET_COMP_TYPE_FUNC_GLOBAL(ComponentType)										\
	GET_COMP_TYPE_FUNC_VIRTUAL(ComponentType)

#define CONSTRUCTOR_CONTENTS_COMPONENT(ComponentType)								\
	public:																			\
    ComponentType(NagiocpX::ContentsEntity* const pOwner_) noexcept				\
        : ContentsComponent{ pOwner_ } {}											\
	DECLARE_COMPONENT_TYPE(ComponentType)

#define CONSTRUCTOR_CONTENTS_UPDATE_COMPONENT(ComponentType)						\
	public:																			\
    ComponentType(NagiocpX::ContentsEntity* const pOwner_) noexcept				\
        : ContentsUpdateComponent{ pOwner_ } {}										\
	DECLARE_COMPONENT_TYPE(ComponentType)
