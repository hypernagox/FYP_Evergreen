#pragma once
#include "RefCountable.h"

class ServerCore::ContentsEntity;
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
			ServerCore::LogStackTrace();
			std::cout << "Dectected Hash Collision !!: " << __FILE__ << '\n' << "LINE: " << __LINE__ << '\n';
			g_checkHashCollisionMutex.unlock();
			NAGOX_ASSERT(false);
			return false;
		}
		g_checkHashCollisionMutex.unlock();
		return true;
	}
public:
	ContentsComponent(ServerCore::ContentsEntity* const pOwner_)noexcept :m_pOwnerEntity{ pOwner_ } {}
	virtual ~ContentsComponent()noexcept = default;
public:
	ServerCore::ContentsEntity* const GetOwnerEntityRaw()const noexcept { return m_pOwnerEntity; }
	inline ServerCore::S_ptr<ServerCore::ContentsEntity> GetOwnerEntity()const noexcept { return ServerCore::S_ptr<ServerCore::ContentsEntity>{m_pOwnerEntity}; }
	constexpr virtual const uint64_t GetCompType()const noexcept = 0;
	inline const bool IsValid()const noexcept { return m_pOwnerEntity->IsValid(); }
public:
	const uint16_t GetOwnerObjectType()const noexcept { return m_pOwnerEntity->GetObjectType(); }
	const uint32_t GetOwnerObjectID()const noexcept { return m_pOwnerEntity->GetObjectID(); }
	ServerCore::ClusterInfo GetOwnerClusterInfo()const noexcept { return m_pOwnerEntity->GetClusterInfo(); }
private:
	ServerCore::ContentsEntity* const m_pOwnerEntity;
};

class ContentsUpdateComponent
	:public ContentsComponent
{
public:
	ContentsUpdateComponent(ServerCore::ContentsEntity* const pOwner_)noexcept :ContentsComponent{ pOwner_ } {}
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
    ComponentType(ServerCore::ContentsEntity* const pOwner_) noexcept				\
        : ContentsComponent{ pOwner_ } {}											\
	DECLARE_COMPONENT_TYPE(ComponentType)

#define CONSTRUCTOR_CONTENTS_UPDATE_COMPONENT(ComponentType)						\
	public:																			\
    ComponentType(ServerCore::ContentsEntity* const pOwner_) noexcept				\
        : ContentsUpdateComponent{ pOwner_ } {}										\
	DECLARE_COMPONENT_TYPE(ComponentType)
