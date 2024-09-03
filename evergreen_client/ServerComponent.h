#pragma once


class ServerObject;
using udsdx::SceneObject;

class ServerComponent
{
	static inline std::set<uint64_t> g_setForCheckHashCollision;
	static inline std::mutex g_checkHashCollisionMutex;
protected:
	template<typename T>
	static const bool CheckHashCollision()noexcept {
		g_checkHashCollisionMutex.lock();
		if (!g_setForCheckHashCollision.emplace(T::GetCompTypeNameGlobal()).second) {
			NetHelper::LogStackTrace();
			std::cout << "Dectected Hash Collision !!: " << __FILE__ << '\n' << "LINE: " << __LINE__ << '\n';
			g_checkHashCollisionMutex.unlock();
			NET_NAGOX_ASSERT(false);
			return false;
		}
		g_checkHashCollisionMutex.unlock();
		return true;
	}
public:
	ServerComponent(ServerObject* const owner);
	virtual ~ServerComponent();
public:
	virtual void Update()noexcept = 0;
	constexpr virtual const uint64_t GetCompType()const noexcept = 0;
	SceneObject* const GetRootObject()const noexcept;
protected:
	ServerObject* const m_owner;
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

#define DECLARE_COMPONENT_TYPE(ComponentType)							\
	private:															\
	COMPONENT_ID(ComponentType)											\
	CHECK_HASH_COLLISION(ComponentType)									\
	public:																\
	GET_COMP_TYPE_FUNC_GLOBAL(ComponentType)							\
	GET_COMP_TYPE_FUNC_VIRTUAL(ComponentType)

#define CONSTRUCTOR_SERVER_COMPONENT(ComponentType)						\
	public:																\
    ComponentType(ServerObject* const pOwner_) noexcept					\
        : ServerComponent(pOwner_) {}									\
	DECLARE_COMPONENT_TYPE(ComponentType)
