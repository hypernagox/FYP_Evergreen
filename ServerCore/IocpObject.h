#pragma once
#include "ServerCorePch.h"
#include "MoveBroadcaster.h"
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

	class alignas(64) ContentsEntity final
		:public IocpObject
	{
		friend class Sector;
	public:
		ContentsEntity(const uint16_t type_id, const uint8_t obj_type_info) noexcept
			: m_objectCombineID{ CombineObjectID(type_id,IDGenerator::GenerateID()) }
			, m_objTypeInfo{ obj_type_info }
			, m_componentSystem{ xnew<ComponentSystemEX>(m_bIsValid,this) }
		{}
		ContentsEntity(Session* const session_) noexcept
			: m_objectCombineID{ CombineObjectID(0,IDGenerator::GenerateID()) }
			, m_pSession{ reinterpret_cast<PacketSession* const>(session_) }
			, m_componentSystem{ xnew<ComponentSystem>(m_bIsValid) }
		{}
		~ContentsEntity()noexcept;
	public:
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void PostEntityTask(Ret(T::* const memFunc)(Args...)noexcept, S_ptr<U>&& ptr, Args&&... args)noexcept {
			PostEntityTask(Task{ memFunc, std::move(ptr), std::forward<Args>(args)... });
		}
		virtual void Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept override final;
	public:
		inline S_ptr<ContentsEntity> SharedFromThis()const noexcept { return S_ptr<ContentsEntity>{this}; }
		inline const uint64_t GetObjectCombineID()const noexcept { return m_objectCombineID; }
		inline const uint32_t GetObjectID()const noexcept { return static_cast<const uint32_t>(ServerCore::GetObjectID(m_objectCombineID)); }
		inline const uint64_t GetObjectID64()const noexcept { return ServerCore::GetObjectID(m_objectCombineID); }
		inline const uint8_t GetObjectType()const noexcept { return static_cast<const uint8_t>(ServerCore::GetObjectType(m_objectCombineID)); }
	public:
		constexpr inline const PacketSession* const GetSession()const noexcept { return m_pSession; }
		inline const ClientSession* const GetClientSession()const noexcept { return reinterpret_cast<const ClientSession* const>(m_pSession); }
		constexpr inline MoveBroadcaster* const GetMoveBroadcaster()const noexcept { return m_moveBroadcaster; }
		inline const int BroadcastMove(const float x, const float y, Vector<Sector*> sectors)const noexcept { return m_moveBroadcaster->BroadcastMove(x, y, std::move(sectors)); }
		template <typename T = class World>
		constexpr inline const T* const GetCurWorld()const noexcept { return m_moveBroadcaster.GetCurWorld<T>(); }
	public:
		inline const ID_Ptr<ServerCore::Sector> GetCombinedSectorInfo()const noexcept { return m_CurrentSectorInfo.load(std::memory_order_acquire); }
		inline void SetSectorInfo(const uint16_t prev_sector_id, const ServerCore::Sector* const cur_sector)noexcept { m_CurrentSectorInfo.store(ID_Ptr<ServerCore::Sector>{ prev_sector_id, cur_sector }, std::memory_order_release); }
		inline const uint16_t GetPrevSectorID()const noexcept { return GetCombinedSectorInfo().GetID(); }
		template <typename T = ServerCore::Sector>
		inline T* const GetCurSector()const noexcept { return static_cast<T* const>(GetCombinedSectorInfo().GetPtr()); }
		inline const ID_Ptr<ServerCore::Sector> ExchangeSector(const ID_Ptr<ServerCore::Sector> sector)noexcept { return m_CurrentSectorInfo.exchange(sector, std::memory_order_acq_rel); }
		inline const ID_Ptr<ServerCore::Sector> ResetSector()noexcept { return ExchangeSector(ID_Ptr<ServerCore::Sector>{0, nullptr}); }
	public:
		inline const bool IsValid()const noexcept { return m_bIsValid.load(std::memory_order_acquire); }
		const bool TryOnDestroy()noexcept {
			const bool bRes = m_bIsValid.exchange(false, std::memory_order_acq_rel);
			if (true == bRes)OnDestroy();
			return bRes;
		}
		inline void DecRef()const noexcept { RefCountable::DecRef<ContentsEntity>(); }
		inline void DecRef(const int32_t dec_cnt)const noexcept { RefCountable::DecRef<ContentsEntity>(dec_cnt); }
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
		template <typename T = Queueabler>
		constexpr inline void MoveBroadcastEnqueue(const float x, const float y, Vector<Sector*>&& sectors) const noexcept {
			GetQueueabler<T>()->EnqueueBroadcastEventTryExecute(&MoveBroadcaster::BroadcastMove, const_cast<MoveBroadcaster* const>(m_moveBroadcaster), float{ x }, float{ y }, std::move(sectors));
		}
	public:
		template <typename T> requires std::is_enum_v<T>
		void SetObjectTypeInfo(const T obj_type_info)noexcept { m_objTypeInfo = static_cast<const uint8_t>(obj_type_info); }
		const uint8_t GetObjectTypeInfo()const noexcept { return m_objTypeInfo; }
		inline void Update(const float dt_)noexcept {
			if (true == m_bNowUpdateFlag.exchange(true, std::memory_order_relaxed))
				return;
			m_componentSystem->Update(dt_);
			m_bNowUpdateFlag.store(false, std::memory_order_release);
		}
		inline void UpdateNonCheck(const float dt_)const noexcept { m_componentSystem->Update(dt_); }
		template <typename T, typename... Args>
		T* const AddComp(Args&&... args)noexcept { return m_componentSystem->AddComp<T>(this, std::forward<Args>(args)...); }
		template <typename T>
		constexpr inline T* const GetComp()const noexcept { return m_componentSystem->GetComp<T>(); }
		inline const ComponentSystem* const GetComponentSystem()const noexcept { return m_componentSystem; }
	private:
		void PostEntityTask(Task&& task_)const noexcept;
		void OnDestroy()noexcept;
		void Destroy()noexcept;
	private:
		alignas(64) PacketSession* const m_pSession = nullptr;
		const uint64_t m_objectCombineID;
		uint8_t m_objTypeInfo;
		ComponentSystem* const m_componentSystem;
		MoveBroadcaster* const m_moveBroadcaster = xnew<MoveBroadcaster>(this);
		IocpComponent* m_arrIocpComponents[etoi(IOCP_COMPONENT::END)] = {};
		alignas(64) std::atomic_bool m_bIsValid = true;
		std::atomic_bool m_bNowUpdateFlag = false;
		std::atomic<ID_Ptr<ServerCore::Sector>> m_CurrentSectorInfo;
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
		inline const ID_Ptr<Sector> GetOwnerSectorInfo()const noexcept { return m_pOwnerEntity->GetCombinedSectorInfo(); }
		virtual void OnDestroy()noexcept = 0;
	protected:
		virtual void Dispatch(ServerCore::IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept = 0;
		inline void IncOwnerRef()const noexcept { m_pOwnerEntity->IncRef(); }
		inline void DecOwnerRef()const noexcept { m_pOwnerEntity->DecRef(); }
	private:
		ContentsEntity* const m_pOwnerEntity;
	};
}