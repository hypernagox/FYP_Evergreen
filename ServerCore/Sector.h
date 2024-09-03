#pragma once
#include "ServerCorePch.h"
#include "VectorHash.hpp"

namespace ServerCore
{
	class Session;
	class SendBuffer;
	class World;

	class Sector
		:public TaskQueueable
	{
		enum { HEART_BEAT_TICK = 15000 };
	public:
		Sector(const uint8_t num_of_groups, const uint8_t x, const uint8_t y, World* const parentWorld)noexcept
			: m_vectorHashMapForEntity{ CreateTCMallocArray<VectorHashMap4ID<uint32_t, ContentsEntity*>>(num_of_groups) }
			, m_sectorID{ Point2D::CombineXY(x,y) }
			, m_pParentWorld{ parentWorld }
		{
			//Update(100);
		}
		virtual ~Sector()noexcept;
	public:
		template <typename T = Sector>
		constexpr inline S_ptr<T> SharedFromThis()const noexcept { return S_ptr<T>{this}; }
		constexpr inline auto& GetSectorSRWLock(const uint8_t group_type)noexcept { return m_vectorHashMapForEntity[group_type].GetSRWLock(); }
		inline void sector_lock(const uint8_t group_type)noexcept { m_vectorHashMapForEntity[group_type].lock(); }
		inline void sector_unlock(const uint8_t group_type)noexcept { m_vectorHashMapForEntity[group_type].unlock(); }
		inline void sector_lock_shared(const uint8_t group_type)noexcept { m_vectorHashMapForEntity[group_type].lock_shared(); }
		inline void sector_unlock_shared(const uint8_t group_type)noexcept { m_vectorHashMapForEntity[group_type].unlock_shared(); }
		
		constexpr inline auto& GetSessionSRWLock()noexcept { return m_vectorHashMapForEntity.data()->GetSRWLock(); }
		inline void session_lock()noexcept { m_vectorHashMapForEntity.data()->lock(); }
		inline void session_unlock()noexcept { m_vectorHashMapForEntity.data()->unlock(); }
		inline void session_lock_shared()noexcept { m_vectorHashMapForEntity.data()->lock_shared(); }
		inline void session_unlock_shared()noexcept { m_vectorHashMapForEntity.data()->unlock_shared(); }
		inline const uint16_t GetSectorID()const noexcept { return m_sectorID; }
		inline const Point2D GetSectorXY()const noexcept { return Point2D{ m_sectorID }; }
		inline const World* const GetParentWorld()const noexcept { return m_pParentWorld; }
	public:
		static void BroadCastParallel(const S_ptr<SendBuffer>& pSendBuffer, const std::span<Sector* const>& sectors)noexcept;
		static void BroadCastParallel(const S_ptr<SendBuffer>& pSendBuffer, const std::span<Sector* const>& sectors
			, const ContentsEntity* const broadcast_entity
			, const bool bExceptThisSession = true
			, const MoveBroadcaster::HuristicFunc huristic = nullptr
		)noexcept;
		void BroadCastParallel(const S_ptr<SendBuffer>& pSendBuffer)const noexcept;
	public:
		inline const auto& GetSessionVectorHash()const noexcept { return *m_vectorHashMapForEntity.data(); }
		inline const auto& GetVectorHash(const uint8_t group_type)const noexcept { return *(m_vectorHashMapForEntity.data() + group_type); }
		inline const auto GetEntities()const noexcept { return m_vectorHashMapForEntity; }
		inline const auto& GetSessionList()const noexcept { return m_vectorHashMapForEntity.data()->GetItemListRef(); }
		inline const auto& GetEntityList(const uint8_t group_type)const noexcept { return (m_vectorHashMapForEntity.data() + group_type)->GetItemListRef(); }
		inline void EnterEnqueue(const uint8_t group_type, const uint32_t obj_id, ContentsEntity* const pEntity_)noexcept{
			pEntity_->SetSectorInfo(GetSectorID(), this);
			EnqueueAsync(&Sector::Enter, uint8_t{ group_type }, c_uint32{ obj_id }, static_cast<ContentsEntity* const>(pEntity_));
		}
		inline void EnterEnqueue(ContentsEntity* const pEntity_)noexcept { EnterEnqueue(pEntity_->GetObjectType(), pEntity_->GetObjectID(), static_cast<ContentsEntity* const>(pEntity_)); }
		inline void LeaveAndDestroyEnqueue(const uint8_t group_type, const uint32_t obj_id)noexcept { EnqueueAsync(&Sector::LeaveAndDestroy, uint8_t{ group_type }, c_uint32{ obj_id }); }
		inline void LeaveEnqueue(const uint8_t group_type, const uint32_t obj_id)noexcept { EnqueueAsync(&Sector::Leave, uint8_t{ group_type }, c_uint32{ obj_id }); }
		inline void BroadCastEnqueue(S_ptr<SendBuffer> pSendBuffer)noexcept{ EnqueueAsync(&Sector::BroadCast, std::move(pSendBuffer)); }
		inline void BroadCastEnqueue(S_ptr<SendBuffer> pSendBuffer, c_uint32 exceptSessionNumber)noexcept { EnqueueAsync(&Sector::BroadCastExceptOne, std::move(pSendBuffer), c_uint32{ exceptSessionNumber }); }
		inline void MigrationEnqueue(S_ptr<Sector> pOtherSector, const uint8_t group_type, c_uint32 obj_id)noexcept { EnqueueAsync(&Sector::Migration, std::move(pOtherSector), uint8_t{ group_type }, c_uint32{ obj_id }); }
		inline void MigrationEnqueue(S_ptr<Sector> pOtherSector, const ContentsEntity* const pEntity_)noexcept { MigrationEnqueue(std::move(pOtherSector), pEntity_->GetObjectType(), pEntity_->GetObjectID()); }
		inline void MigrationAllEnqueue(S_ptr<Sector> pOtherSector)noexcept { EnqueueAsync(&Sector::MigrationAll, std::move(pOtherSector)); }
		inline void MigrationWorldEnqueue(S_ptr<World> curWorld, S_ptr<World> destWorld, ContentsEntity* const pEntity)noexcept {
			EnqueueAsync(&Sector::MigrationWorld, std::move(curWorld), std::move(destWorld), pEntity->GetObjectType(), pEntity->GetObjectID());
		}
		inline void MigrationWorldWithXYEnqueue(S_ptr<World> curWorld, S_ptr<World> destWorld, ContentsEntity* const pEntity, const uint8_t start_x, const uint8_t start_y)noexcept {
			EnqueueAsync(&Sector::MigrationWorldWithXY, std::move(curWorld), std::move(destWorld), pEntity->GetObjectType(), pEntity->GetObjectID(), uint8_t{ start_x }, uint8_t{ start_y });
		}
	protected:
		static const Vector<ContentsEntity*>& GetSessionCopyListIncRef(const std::span<Sector* const> sectors)noexcept;
		static const Vector<ContentsEntity*>& GetEntityCopyListIncRef(const std::span<Sector* const> sectors)noexcept;
		static const Vector<ContentsEntity*>& GetVectorHashCopyListIncRef(const VectorHashMap4ID<uint32_t, ContentsEntity*>& vecHash)noexcept;
		void Update(const uint32_t tick_ms = 100)noexcept;
		virtual void MigrationAfterBehavior(ServerCore::Sector* const beforeSector, ContentsEntity* const pEntity_)noexcept {}
	public:
		inline const Vector<ContentsEntity*>& GetSessionCopyListIncRef()const noexcept { return Sector::GetVectorHashCopyListIncRef(*m_vectorHashMapForEntity.data()); }
		inline const Vector<ContentsEntity*>& GetEntityCopyListIncRef(const uint8_t group_type)const noexcept { return Sector::GetVectorHashCopyListIncRef(m_vectorHashMapForEntity[group_type]); }
	private:
		void Enter(const uint8_t group_type, const uint32_t obj_id, ContentsEntity* const pEntity_)noexcept;
		void LeaveAndDestroy(const uint8_t group_type, const uint32_t obj_id)noexcept;
		void Leave(const uint8_t group_type, const uint32_t obj_id)noexcept;
		void Migration(const S_ptr<Sector> pOtherSector, const uint8_t group_type, const uint32_t obj_id) noexcept;
		void MigrationAll(const S_ptr<Sector> pOtherSector)noexcept;
		void ProcessMigration(const S_ptr<Sector> pBeforSector, const uint8_t group_type, const uint32_t obj_id, ContentsEntity* const pEntity_)noexcept;
		void BroadCast(const S_ptr<SendBuffer> pSendBuffer)noexcept;
		void BroadCastExceptOne(const S_ptr<SendBuffer> pSendBuffer, const uint32_t exceptSessionNumber)noexcept;
		void RegisterHeartBeat()noexcept;
		void ListenHeartBeat()noexcept;
		void MigrationWorld(S_ptr<World> curWorld, S_ptr<World> destWorld, const uint8_t group_type, const uint32_t obj_id)noexcept;
		void MigrationWorldWithXY(S_ptr<World> curWorld, S_ptr<World> destWorld, const uint8_t group_type, const uint32_t obj_id, const uint8_t start_x, const uint8_t start_y)noexcept;
	private:
		static const Vector<ContentsEntity*>& GetSessionCopyListIncRefInternal(const std::span<Sector* const> sectors)noexcept;
		static const Vector<ContentsEntity*>& GetEntityCopyListIncRefInternal(const std::span<Sector* const> sectors)noexcept;
	protected:
		const std::span<VectorHashMap4ID<uint32_t, ContentsEntity*>> m_vectorHashMapForEntity;
		uint64_t m_lastUpdateTime = ::GetTickCount64();
		const uint16_t m_sectorID;
		World* const m_pParentWorld;
	};
}
