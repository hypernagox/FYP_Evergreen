#pragma once
#include "ServerCorePch.h"
#include "RefCountable.h"
#include "ID_Ptr.hpp"
#include "ComponentSystem.h"
#include "NagoxDeleter.h"

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
	public:
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

	struct alignas(4) ClusterInfo
	{
		uint8_t fieldID;
		Point2D clusterID;
		ClusterInfo()noexcept = default;
		ClusterInfo(const uint8_t fieldID_,const uint8_t x_,const uint8_t y_)noexcept
			: fieldID{fieldID_}
			, clusterID{ x_,y_ }
		{}
		ClusterInfo(const uint8_t fieldID_, const Point2D clusterID_)noexcept
			: fieldID{ fieldID_ }
			, clusterID{ clusterID_ }
		{}
	};

	class alignas(8) EntityInfo
	{
	public:
		constexpr inline EntityInfo(const uint8_t primary_group_type, const uint64_t id_)noexcept
			:m_obj_id{ id_ }, m_obj_primary_group_type{ primary_group_type } {}

		constexpr inline EntityInfo(const uint8_t primary_group_type, const uint8_t detail_type, const uint64_t id_)noexcept
			:m_obj_id{ id_ }, m_obj_primary_group_type{ primary_group_type }, m_obj_detail_type{ detail_type } {}
	public:
		constexpr inline const uint64_t GetObjectID()const noexcept { return m_obj_id; }

		template<typename T = uint8_t> requires (std::is_enum_v<T> || std::same_as<T, uint8_t>) && (sizeof(T) == sizeof(uint8_t))
		constexpr inline const T GetPrimaryGroupType()const noexcept { return static_cast<const T>(m_obj_primary_group_type); }

		template<typename T = uint8_t> requires (std::is_enum_v<T> || std::same_as<T, uint8_t>) && (sizeof(T) == sizeof(uint8_t))
		constexpr inline const T GetObjectDetailType()const noexcept { return static_cast<const T>(m_obj_detail_type); }

		template<typename T> requires std::is_enum_v<T>
		constexpr inline void SetObjectDetailType(const T eDetailType_)const noexcept { m_obj_detail_type = static_cast<const uint8_t>(eDetailType_); }
	public:
		void SetObjectID4Reuse()const noexcept { m_obj_id = IDGenerator::GenerateID(); }
	private:
		mutable uint64_t m_obj_id : 48;
		const uint64_t m_obj_primary_group_type : 8;
		mutable uint64_t m_obj_detail_type : 8;
	};

	extern Cluster* const GetCluster(const ClusterInfo info)noexcept;

	class ContentsEntity final
		:public IocpObject
	{
	public:
		ContentsEntity(const uint8_t primary_group_type, const uint8_t detail_type) noexcept;
		ContentsEntity(Session* const session_) noexcept;
	private:
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

		constexpr inline const EntityInfo GetEntityInfo()const noexcept { return m_entity_info; }
		constexpr inline const uint32_t GetObjectID()const noexcept { return static_cast<const uint32_t>(m_entity_info.GetObjectID()); }

		constexpr inline const uint64_t GetObjectID64()const noexcept { return m_entity_info.GetObjectID(); }

		template<typename T = uint8_t> requires (std::is_enum_v<T> || std::same_as<T, uint8_t>) && (sizeof(T) == sizeof(uint8_t))
		constexpr inline const T GetPrimaryGroupType()const noexcept { return static_cast<const T>(m_entity_info.GetPrimaryGroupType<T>()); }

		inline bool IsNPC()noexcept { return 0 != m_entity_info.GetPrimaryGroupType(); }
	public:
		constexpr inline const PacketSession* const GetSession()const noexcept { return m_pSession; }
		inline const ClientSession* const GetClientSession()const noexcept { return reinterpret_cast<const ClientSession* const>(m_pSession); }
	public:
		inline const bool IsValid()const noexcept { return m_bIsValid.load(); }
		const bool TryOnDestroy()noexcept {
			const bool bRes = (true == m_bIsValid.load_relaxed()) && (true == m_bIsValid.exchange(false));
			if (true == bRes)OnDestroy();
			return bRes;
		}
		inline void DecRef()const noexcept { RefCountable::DecRef<ContentsEntity>(); }
		void SetActive()noexcept { m_bIsValid.store(true); }
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
		template <typename T> requires std::is_enum_v<T> && (sizeof(T) == sizeof(uint8_t))
		constexpr inline void SetDetailType(const T obj_detail_type)noexcept { m_entity_info.SetObjectDetailType<T>(obj_detail_type); }

		template<typename T = uint8_t> requires (std::is_enum_v<T> || std::same_as<T, uint8_t>) && (sizeof(T) == sizeof(uint8_t))
		constexpr inline const T GetDetailType()const noexcept { return m_entity_info.GetObjectDetailType<T>(); }

		void Update(const float dt_)noexcept;
		void UpdateNonCheck(const float dt_)const noexcept;
		template <typename T, typename... Args>
		T* const AddComp(Args&&... args)noexcept { return m_componentSystem->AddComp<T>(this, std::forward<Args>(args)...); }
		template <typename T>
		constexpr inline T* const GetComp()const noexcept { return m_componentSystem->GetComp<T>(); }
		inline const class ComponentSystem* const GetComponentSystem()const noexcept { return m_componentSystem; }
	public:
		void SetClusterInfo(const ClusterInfo info)noexcept { m_clusterInfo.store(info); }
		void SetClusterInfoUnsafe(const ClusterInfo info)noexcept { m_clusterInfo.store_relaxed(info); }

		ClusterInfo GetClusterInfo()const noexcept { return m_clusterInfo.load(); }
		Cluster* const GetCurCluster()const noexcept { return ServerCore::GetCluster(m_clusterInfo); }

		const bool IsPendingClusterEntry()const noexcept { return 0 != m_clusterEnterCount; }
		const bool RegisterEnterCount()noexcept { return 0 == InterlockedDecrement16(&m_clusterEnterCount); }
		void ResetClusterCount()noexcept { InterlockedExchange16(&m_clusterEnterCount, static_cast<SHORT>(ThreadMgr::NUM_OF_THREADS)); }
		const bool IsReadyAndValid()const noexcept { return IsValid() && !IsPendingClusterEntry(); }
	private:
		void PostEntityTask(Task&& task_)const noexcept;
		void OnDestroy()noexcept;
	public:
		template<typename T, typename... Args> requires std::derived_from<T,NagoxDeleter>
		void SetDeleter(Args&&... args)noexcept {
			NAGOX_ASSERT(nullptr == m_deleter);
			m_deleter = xnew<T>(std::forward<Args>(args)...);
		}
		const auto GetDeleter()const noexcept { return m_deleter; }
		void ResetDeleter()noexcept {
			if (m_deleter) {
				xdelete<NagoxDeleter>(m_deleter);
				m_deleter = nullptr;
			}
		}
		void ProcessCleanUp()noexcept;
	private:
		const PadByte<6> pad;
		NagoxDeleter* m_deleter = nullptr;
		PacketSession* const m_pSession = nullptr;
		const EntityInfo m_entity_info;
		class ComponentSystem* const m_componentSystem;
		IocpComponent* m_arrIocpComponents[etoi(IOCP_COMPONENT::END)] = {};
		NagoxAtomic::Atomic<bool> m_bIsValid{ true };
		volatile SHORT m_clusterEnterCount = static_cast<SHORT>(ThreadMgr::NUM_OF_THREADS);
		NagoxAtomic::Atomic<ClusterInfo> m_clusterInfo;
		//std::atomic_bool m_bNowUpdateFlag = false;
	};

	template <typename T, typename U> requires std::is_enum_v<T> && std::is_enum_v<U> && (sizeof(uint8_t) == sizeof(T)) && (sizeof(uint8_t) == sizeof(U))
	static constexpr inline S_ptr<ContentsEntity> CreateContentsEntity(const T primary_group_type, const U detail_type)noexcept { return MakeShared<ContentsEntity>(static_cast<const uint8_t>(primary_group_type), static_cast<const uint8_t>(detail_type)); }

	class IocpComponent
	{
		friend class ContentsEntity;
	public:
		IocpComponent(ContentsEntity* const pOwner_, const IOCP_COMPONENT compType)noexcept;
		virtual ~IocpComponent()noexcept;
	public:
		constexpr inline const ContentsEntity* const GetOwnerEntity()const noexcept { return m_pOwnerEntity; }
		constexpr inline ContentsEntity* const GetOwnerEntity()noexcept { return m_pOwnerEntity; }

		inline const bool IsValid()const noexcept { return m_pOwnerEntity->IsValid(); }
		virtual void ProcessCleanUp()noexcept {}
	public:
		template<typename T = uint8_t> requires (std::is_enum_v<T> || std::same_as<T, uint8_t>) && (sizeof(T) == sizeof(uint8_t))
		constexpr inline const T GetOwnerPrimaryGroup()const noexcept { return m_pOwnerEntity->GetPrimaryGroupType<T>(); }
		constexpr inline const uint32_t GetOwnerObjectID()const noexcept { return m_pOwnerEntity->GetObjectID(); }
		const ClusterInfo GetOwnerClusterInfo()const noexcept { return m_pOwnerEntity->GetClusterInfo(); }
	protected:
		virtual void Dispatch(S_ptr<ContentsEntity>* const owner_entity)noexcept = 0;
		void PostIocpEvent(S_ptr<ContentsEntity>* const owner_entity = nullptr)noexcept;
		void ReserveIocpEvent(const uint64_t tickAfterMs_, S_ptr<ContentsEntity>* const owner_entity = nullptr)noexcept;
		void SetIncRefEntity()noexcept { SetIncRefEntity(S_ptr<IocpObject>{ m_pOwnerEntity }); }
		void SetIncRefEntity(S_ptr<ContentsEntity>&& owner_entity)noexcept;
		class IocpCompEvent& GetIocpCompEvent()const noexcept { return reinterpret_cast<IocpCompEvent&>(const_cast<uint64_t&>(m_iocpCompEvent)); }
		[[nodiscard]] S_ptr<ContentsEntity> PassEntity()noexcept;
	private:
		ContentsEntity* const m_pOwnerEntity;
		const uint64_t m_iocpCompEvent;
	};
}