#pragma once
#include "ServerCorePch.h"
#include "RefCountable.h"
#include "ID_Ptr.hpp"
#include "ComponentSystem.h"

class ClientSession;

namespace ServerCore
{
	class IocpEvent;
	class Sector;
	class PacketSession;

	/*--------------
		IocpCore
	---------------*/

	// IOCP에 등록 가능한 모든 오브젝트
	class IocpObject
		:public RefCountable
	{
	protected:
		IocpObject()noexcept = default;
		virtual ~IocpObject()noexcept = default;
	public:
		virtual void Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept = 0;
		template<typename T = IocpObject>
		constexpr inline S_ptr<T> SharedFromThis()const noexcept { return S_ptr<T>{this}; }
	};

	enum class IOCP_COMPONENT :uint8_t
	{
		Queueabler,
		TickTimer,
		

		END,
	};

	class IocpComponent;
	class Queueabler;
	class TickTimer;
	class Cluster;

	struct ClusterInfo
	{
		uint8_t fieldID;
		Point2D clusterID;
		ClusterInfo()noexcept = default;
		ClusterInfo(const uint8_t fieldID_,const uint8_t x_,const uint8_t y_)noexcept
			: fieldID{fieldID_}
			, clusterID{ x_,y_ }
		{}
		ClusterInfo(const uint8_t fieldID_,const Point2D clusterID_)noexcept
			: fieldID{ fieldID_ }
			, clusterID{ clusterID_ }
		{}
	private:
		char pad;
	};

	extern Cluster* const GetCluster(const ClusterInfo info)noexcept;

	class alignas(64) ContentsEntity final
		:public IocpObject
	{
	public:
		ContentsEntity(const uint16_t type_id, const uint8_t obj_type_info) noexcept;
		ContentsEntity(Session* const session_) noexcept;
		~ContentsEntity()noexcept;
	public:
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void PostEntityTask(Ret(T::* const memFunc)(Args...)noexcept, S_ptr<U>&& ptr, Args&&... args)noexcept {
			PostEntityTask(Task{ memFunc, std::move(ptr), std::forward<Args>(args)... });
		}
		template<typename Ret, typename... Args>
		void PostEntityTask(Ret(ContentsEntity::* const memFunc)(Args...)noexcept, Args&&... args)noexcept {
			PostEntityTask(Task{ memFunc, this, std::forward<Args>(args)... });
		}
		virtual void Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept override final;
	public:
		inline S_ptr<ContentsEntity> SharedFromThis()const noexcept { return S_ptr<ContentsEntity>{this}; }
		inline const uint64_t GetObjectCombineID()const noexcept { return m_objectCombineID; }
		inline const uint32_t GetObjectID()const noexcept { return static_cast<const uint32_t>(ServerCore::GetObjectID(m_objectCombineID)); }
		inline const uint64_t GetObjectID64()const noexcept { return ServerCore::GetObjectID(m_objectCombineID); }
		inline const uint8_t GetObjectType()const noexcept { return static_cast<const uint8_t>(ServerCore::GetObjectType(m_objectCombineID)); }

		static inline bool IsNPC(const uint64_t combinedID)noexcept { return 0 != ServerCore::GetObjectType(combinedID); }
	public:
		constexpr inline const PacketSession* const GetSession()const noexcept { return m_pSession; }
		inline const ClientSession* const GetClientSession()const noexcept { return reinterpret_cast<const ClientSession* const>(m_pSession); }
	public:
		inline const bool IsValid()const noexcept { return m_bIsValid.load(); }
		const bool TryOnDestroy()noexcept {
			const bool bRes = (true == m_bIsValid.load(std::memory_order_relaxed)) && (true == m_bIsValid.exchange(false));
			if (true == bRes)OnDestroy();
			return bRes;
		}
		inline void DecRef()const noexcept { RefCountable::DecRef<ContentsEntity>(); }
	public:
		template <typename T>
		inline T* const GetIocpComponent()const noexcept {
			if constexpr (std::derived_from<T, Queueabler>)
				return static_cast<T* const>(m_arrIocpComponents[etoi(IOCP_COMPONENT::Queueabler)]);
			else if constexpr (std::derived_from<T, TickTimer>)
				return static_cast<T* const>(m_arrIocpComponents[etoi(IOCP_COMPONENT::TickTimer)]);
			else
				static_assert(std::_Always_false<T>);

			return nullptr;
		}
		template <typename T, typename... Args>
		T* const AddIocpComponent(Args&&... args)noexcept {
			const auto temp = xnew<T>(this, std::forward<Args>(args)...);

			if constexpr (std::derived_from<T, Queueabler>)
				m_arrIocpComponents[etoi(IOCP_COMPONENT::Queueabler)] = temp;
			else if constexpr (std::derived_from<T, TickTimer>)
				m_arrIocpComponents[etoi(IOCP_COMPONENT::TickTimer)] = temp;
			else
				static_assert(std::_Always_false<T>);

			return temp;
		}
		template <typename T = Queueabler>
		constexpr inline Queueabler* const GetQueueabler()const noexcept { return GetIocpComponent<T>(); }
	public:
		template <typename T> requires std::is_enum_v<T>
		void SetObjectTypeInfo(const T obj_type_info)noexcept { m_objTypeInfo = static_cast<const uint8_t>(obj_type_info); }
		const uint8_t GetObjectTypeInfo()const noexcept { return m_objTypeInfo; }
		void Update(const float dt_)noexcept;
		void UpdateNonCheck(const float dt_)const noexcept;
		template <typename T, typename... Args>
		T* const AddComp(Args&&... args)noexcept { return m_componentSystem->AddComp<T>(this, std::forward<Args>(args)...); }
		template <typename T>
		inline T* const GetComp()const noexcept { return m_componentSystem->GetComp<T>(); }
		inline const class ComponentSystem* const GetComponentSystem()const noexcept { return m_componentSystem; }
	public:
		void SetClusterInfo(const ClusterInfo info)noexcept { m_clusterInfo.store(info); }
		ClusterInfo GetClusterInfo()const noexcept { return m_clusterInfo; }
		Cluster* const GetCurCluster()const noexcept { return ServerCore::GetCluster(m_clusterInfo); }
	private:
		void PostEntityTask(Task&& task_)const noexcept;
		void OnDestroy()noexcept;
		void Destroy()noexcept;
	private:
		alignas(64) PacketSession* const m_pSession = nullptr;
		const uint64_t m_objectCombineID;
		uint8_t m_objTypeInfo;
		class ComponentSystem* const m_componentSystem;
		IocpComponent* m_arrIocpComponents[etoi(IOCP_COMPONENT::END)] = {};
		alignas(64) std::atomic_bool m_bIsValid = true;
		std::atomic<ClusterInfo> m_clusterInfo;
		std::atomic_bool m_bNowUpdateFlag = false;
	};

	template <typename T, typename U> requires std::is_enum_v<T> && std::is_enum_v<U>
	static constexpr inline S_ptr<ContentsEntity> CreateContentsEntity(const T type_id, const U obj_type_info)noexcept { return MakeSharedAligned<ContentsEntity>(static_cast<const uint16_t>(type_id), static_cast<const uint8_t>(obj_type_info)); }

	class IocpComponent
		:public IocpObject
	{
	public:
		IocpComponent(ContentsEntity* const pOwner_)noexcept :m_pOwnerEntity{ pOwner_ } {}
		virtual ~IocpComponent()noexcept = default;
	public:
		template<typename T = IocpComponent>
		constexpr inline S_ptr<T> SharedFromThis()const noexcept { return S_ptr<T>{this}; }
		constexpr inline const ContentsEntity* const GetOwnerEntity()const noexcept { return m_pOwnerEntity; }
		constexpr inline ContentsEntity* const GetOwnerEntity()noexcept { return m_pOwnerEntity; }
		inline const bool IsValid()const noexcept { return m_pOwnerEntity->IsValid(); }
	public:
		inline const uint16_t GetOwnerObjectType()const noexcept { return m_pOwnerEntity->GetObjectType(); }
		inline const uint32_t GetOwnerObjectID()const noexcept { return m_pOwnerEntity->GetObjectID(); }
		// inline const ID_Ptr<Sector> GetOwnerSectorInfo()const noexcept { return m_pOwnerEntity->GetCombinedSectorInfo(); }
		const ClusterInfo GetOwnerClusterInfo()const noexcept { return m_pOwnerEntity->GetClusterInfo(); }
		virtual void OnDestroy()noexcept = 0;
	protected:
		virtual void Dispatch(ServerCore::IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept = 0;
		inline void IncOwnerRef()const noexcept { m_pOwnerEntity->IncRef(); }
		inline void DecOwnerRef()const noexcept { m_pOwnerEntity->DecRef(); }
	private:
		ContentsEntity* const m_pOwnerEntity;
	};
}