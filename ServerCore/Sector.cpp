#include "ServerCorePch.h"
#include "Sector.h"
#include "PacketSession.h"
#include "MoveBroadcaster.h"
#include "Service.h"
#include "World.h"

namespace ServerCore
{
	//Sector::Sector(const uint8_t x, const uint8_t y, World* const parentWorld)noexcept
	//	: m_sectorID{ Point2D::CombineXY(x,y) }
	//	, m_pParentWorld{ parentWorld }
	//{
	//	//Mgr(TaskTimerMgr)->ReserveAsyncTask(HEART_BEAT_TICK, [this]()noexcept
	//	//	{
	//	//		if (const auto ptr = this->shared_from_this())
	//	//		{
	//	//			RegisterHeartBeat();
	//	//		}
	//	//	});
	//}

	Sector::~Sector()noexcept
	{
		std::cout << "섹터 소멸" << std::endl;
		DeleteTCMallocArray(m_vectorHashMapForEntity);
	}

	void Sector::BroadCastParallel(const S_ptr<SendBuffer>& pSendBuffer, const std::span<Sector* const>& sectors) noexcept
	{
		HashSet<const ContentsEntity*>& copy_set = GetTLSetForUnique<const ContentsEntity*>();
		const Vector<ContentsEntity*>& copy_sessions = GetSessionCopyListIncRefInternal(sectors);
		auto b = copy_sessions.data();
		const auto e = b + copy_sessions.size();
		while (e != b) {
			const ContentsEntity* const __restrict pSessionEntity = *b++;
			if (true == copy_set.emplace(pSessionEntity).second)
				pSessionEntity->GetSession()->SendAsync(pSendBuffer);
			pSessionEntity->DecRef();
		}
		copy_set.clear();
	}

	void Sector::BroadCastParallel(const S_ptr<SendBuffer>& pSendBuffer, const std::span<Sector* const>& sectors
		, const ContentsEntity* const broadcast_entity
		, const bool bExceptThisSession
		, const MoveBroadcaster::HuristicFunc huristic) noexcept
	{
		HashSet<const ContentsEntity*>& copy_set = GetTLSetForUnique<const ContentsEntity*>();
		const Vector<ContentsEntity*>& copy_sessions = GetSessionCopyListIncRefInternal(sectors);
		auto b = copy_sessions.data();
		const auto e = b + copy_sessions.size();
		if (huristic) {
			while (e != b) {
				const ContentsEntity* const pSessionEntity = *b++;
				if (true == copy_set.emplace(pSessionEntity).second) {
					const bool bFlag = !(bExceptThisSession && (broadcast_entity == pSessionEntity));
					if (bFlag && huristic(broadcast_entity, pSessionEntity))
						pSessionEntity->GetSession()->SendAsync(pSendBuffer);
				}
				pSessionEntity->DecRef();
			}
		}
		else {
			while (e != b) {
				const ContentsEntity* const pSessionEntity = *b++;
				if (true == copy_set.emplace(pSessionEntity).second) {
					if (!(bExceptThisSession && (broadcast_entity == pSessionEntity)))
						pSessionEntity->GetSession()->SendAsync(pSendBuffer);
				}
				pSessionEntity->DecRef();
			}
		}
		copy_set.clear();
	}

	void Sector::BroadCastParallel(const S_ptr<SendBuffer>& pSendBuffer) const noexcept
	{
		const Vector<ContentsEntity*>& copy_sessions = GetSessionCopyListIncRef();
		auto b = copy_sessions.data();
		const auto e = b + copy_sessions.size();
		while (e != b) {
			const ContentsEntity* const __restrict pSessionEntity = *b++;
			pSessionEntity->GetSession()->SendAsync(pSendBuffer);
			pSessionEntity->DecRef();
		}
	}

	const Vector<ContentsEntity*>& Sector::GetSessionCopyListIncRef(const std::span<Sector* const> sectors)noexcept
	{
		Vector<ContentsEntity*>& copy_sessions = GetTLVectorForCopy<ContentsEntity*>();
		HashSet<const ContentsEntity*>& copy_set = GetTLSetForUnique<const ContentsEntity*>();
		for (const auto sector : sectors) {
			const auto& session_vecHash = sector->GetSessionVectorHash();
			const auto& session_lock = session_vecHash.GetSRWLock();
			const auto& session_list = session_vecHash.GetItemListRef();
			session_lock.lock_shared();
			auto b = session_list.data();
			const auto e = b + session_list.size();
			while (e != b) {
				ContentsEntity* const __restrict pSessionEntity = *b++;
				if (false == copy_set.emplace(pSessionEntity).second)continue;
				copy_sessions.emplace_back(pSessionEntity);
				pSessionEntity->IncRef();
			}
			session_lock.unlock_shared();
		}
		copy_set.clear();
		return copy_sessions;
	}

	const Vector<ContentsEntity*>& Sector::GetEntityCopyListIncRef(const std::span<Sector* const> sectors) noexcept
	{
		Vector<ContentsEntity*>& copy_entities = GetTLVectorForCopy<ContentsEntity*>();
		HashSet<const ContentsEntity*>& copy_set = GetTLSetForUnique<const ContentsEntity*>();
		for (const auto sector : sectors) {
			for (const auto& entities : sector->GetEntities()) {
				const auto& entity_lock = entities.GetSRWLock();
				const auto& entity_list = entities.GetItemListRef();
				entity_lock.lock_shared();
				auto b = entity_list.data();
				const auto e = b + entity_list.size();
				while (e != b) {
					ContentsEntity* const __restrict pEntity = *b++;
					if (false == copy_set.emplace(pEntity).second)continue;
					copy_entities.emplace_back(pEntity);
					pEntity->IncRef();
				}
				entity_lock.unlock_shared();
			}
		}
		copy_set.clear();
		return copy_entities;
	}

	const Vector<ContentsEntity*>& Sector::GetVectorHashCopyListIncRef(const VectorHashMap4ID<uint32_t, ContentsEntity*>& vecHash) noexcept
	{
		Vector<ContentsEntity*>& copy_vecHash = GetTLVectorForCopy<ContentsEntity*>();
		const auto& vecHash_lock = vecHash.GetSRWLock();
		const auto& vecHash_list = vecHash.GetItemListRef();
		vecHash_lock.lock_shared();
		auto b = vecHash_list.data();
		const auto e = b + vecHash_list.size();
		while (e != b) {
			ContentsEntity* const __restrict pEntity = *b++;
			copy_vecHash.emplace_back(pEntity);
			pEntity->IncRef();
		}
		vecHash_lock.unlock_shared();
		return copy_vecHash;
	}

	void Sector::Update(const uint32_t tick_ms) noexcept
	{
		if (false == IsValid())
			return;

		const uint64_t cur_time = ::GetTickCount64();
		const float dt = static_cast<const float>(cur_time - m_lastUpdateTime) / 1000.f;

		auto b = m_vectorHashMapForEntity.data();
		const auto e = b + m_vectorHashMapForEntity.size();
		{
			const auto& vec = (b++)->GetItemListRef();
			auto b2 = vec.data();
			const auto e2 = b2 + vec.size();
			while (e2 != b2) { (*b2++)->Update(dt); }
		}
		++b;
		while (e != b)
		{
			const auto& vec = (b++)->GetItemListRef();
			auto b2 = vec.data();
			const auto e2 = b2 + vec.size();
			while (e2 != b2) { (*b2++)->Update(dt); }
		}

		m_lastUpdateTime = cur_time;
		EnqueueAsyncTimer(tick_ms, &Sector::Update, uint32_t{ tick_ms });
	}

	void Sector::Migration(const S_ptr<Sector> pOtherSector, const uint8_t group_type, const uint32_t obj_id) noexcept
	{
		if (const auto entity = m_vectorHashMapForEntity[group_type].FindItemUnsafe(obj_id))
		{
			pOtherSector->EnqueueAsync(&Sector::ProcessMigration, SharedFromThis(), uint8_t{ group_type }, uint32_t{ obj_id }, static_cast<ContentsEntity* const>(entity));
		}
		else
		{
			//std::cout << "Cannot Find Session" << std::endl;
		}
	}

	void Sector::MigrationAll(const S_ptr<Sector> pOtherSector)noexcept
	{
		for (const auto& entities : m_vectorHashMapForEntity)
		{
			for (const auto entity : entities.GetItemListRef())
			{
				pOtherSector->EnqueueAsync(&Sector::ProcessMigration, SharedFromThis(), entity->GetObjectType(), entity->GetObjectID(), static_cast<ContentsEntity* const>(entity));
			}
		}
	}

	void Sector::ProcessMigration(const S_ptr<Sector> pBeforSector, const uint8_t group_type, const uint32_t obj_id, ContentsEntity* const pEntity_) noexcept
	{
		Sector* const before_sector = pBeforSector.get();
		pEntity_->SetSectorInfo(before_sector->GetSectorID(), this);
		std::atomic_thread_fence(std::memory_order_seq_cst);
		Enter(group_type, obj_id, pEntity_);
		before_sector->LeaveEnqueue(uint8_t{ group_type }, uint32_t{ obj_id });
		MigrationAfterBehavior(before_sector, pEntity_);
	}

	void Sector::RegisterHeartBeat() noexcept
	{
		EnqueueAsync(&Sector::ListenHeartBeat);
	}

	void Sector::ListenHeartBeat() noexcept
	{
		//CREATE_FUNC_LOG(L"HeartBeat");
		//const S_ptr<SendBuffer> sendBuffer = CreateHeartBeatSendBuffer(HEART_BEAT::s2c_HEART_BEAT);
		//for (const auto session : m_linkedHashMapForSession)
		//{
		//	if (session->IsHeartBeatAlive())
		//	{
		//		session->SetHeartBeat(false);
		//		//session->SendOnlyEnqueue(sendBuffer);
		//		//session << sendBuffer;
		//	}
		//	else
		//	{
		//		// TODO: 하트비트 반응없으면 쳐내야함 
		//		//LeaveAndDisconnectEnqueue(session);
		//	}
		//}
		//EnqueueAsyncTimer(HEART_BEAT_TICK, &SessionManageable::ListenHeartBeat);
	}

	void Sector::MigrationWorld(S_ptr<World> curWorld, S_ptr<World> destWorld, const uint8_t group_type, const uint32_t obj_id) noexcept
	{
		if (const auto entity = m_vectorHashMapForEntity[group_type].FindItemUnsafe(obj_id))
		{
			const auto dest_sector = destWorld->GetStartSector();
			destWorld->EnterWorld(entity);
			dest_sector->EnqueueAsync(&World::MigrationWolrdAfterBehavior, std::move(destWorld), std::move(curWorld), static_cast<ContentsEntity* const>(entity));
		}
		else
		{
			//std::cout << "Cannot Find Session" << std::endl;
		}
	}

	void Sector::MigrationWorldWithXY(S_ptr<World> curWorld, S_ptr<World> destWorld, const uint8_t group_type, const uint32_t obj_id, const uint8_t start_x, const uint8_t start_y) noexcept
	{
		if (const auto entity = m_vectorHashMapForEntity[group_type].FindItemUnsafe(obj_id))
		{
			const auto dest_sector = destWorld->GetSector(start_x, start_y);
			destWorld->EnterWorldWithXY(start_x, start_y, entity);
			dest_sector->EnqueueAsync(&World::MigrationWolrdAfterBehavior, std::move(destWorld), std::move(curWorld), static_cast<ContentsEntity* const>(entity));
		}
		else
		{
			//std::cout << "Cannot Find Session" << std::endl;
		}
	}

	const Vector<ContentsEntity*>& Sector::GetSessionCopyListIncRefInternal(const std::span<Sector* const> sectors) noexcept
	{
		Vector<ContentsEntity*>& copy_sessions = GetTLVectorForCopy<ContentsEntity*>();
		for (const auto sector : sectors) {
			const auto& session_vecHash = sector->GetSessionVectorHash();
			const auto& session_lock = session_vecHash.GetSRWLock();
			const auto& session_list = session_vecHash.GetItemListRef();
			session_lock.lock_shared();
			auto b = session_list.data();
			const auto e = b + session_list.size();
			while (e != b) {
				ContentsEntity* const __restrict pSessionEntity = *b++;
				copy_sessions.emplace_back(pSessionEntity);
				pSessionEntity->IncRef();
			}
			session_lock.unlock_shared();
		}
		return copy_sessions;
	}

	const Vector<ContentsEntity*>& Sector::GetEntityCopyListIncRefInternal(const std::span<Sector* const> sectors) noexcept
	{
		Vector<ContentsEntity*>& copy_entities = GetTLVectorForCopy<ContentsEntity*>();
		for (const auto sector : sectors) {
			for (const auto& entities : sector->GetEntities()) {
				const auto& entity_lock = entities.GetSRWLock();
				const auto& entity_list = entities.GetItemListRef();
				entity_lock.lock_shared();
				auto b = entity_list.data();
				const auto e = b + entity_list.size();
				while (e != b) {
					ContentsEntity* const __restrict pEntity = *b++;
					copy_entities.emplace_back(pEntity);
					pEntity->IncRef();
				}
				entity_lock.unlock_shared();
			}
		}
		return copy_entities;
	}

	void Sector::Enter(const uint8_t group_type, const uint32_t obj_id, ContentsEntity* const pEntity_)noexcept
	{
		auto& target_vecHash = *(m_vectorHashMapForEntity.data() + group_type);

		if (false == pEntity_->IsValid())
		{
			if (pEntity_->TryOnDestroy())
			{
				m_pParentWorld->DecRef();
				pEntity_->DecRef();
			}
			return;
		}

		if (!target_vecHash.AddItem(obj_id,pEntity_))
		{
			// TODO: 이미 방에 있는데 또 들어오려한거임
			// std::cout << "Alread Exist in Room" << std::endl;
			return;
		}
	}

	void Sector::BroadCast(const S_ptr<SendBuffer> pSendBuffer)noexcept
	{
		const auto& sessions = GetSessionList();
		auto b = sessions.data();
		const auto e = b + sessions.size();
		while (e != b) { (*b++)->GetSession()->SendAsync(pSendBuffer); }
	}

	void Sector::BroadCastExceptOne(const S_ptr<SendBuffer> pSendBuffer, const uint32_t exceptSessionNumber) noexcept
	{
		auto& session_vecHash = *m_vectorHashMapForEntity.data();
		const auto& sessions = session_vecHash.GetItemListRef();
		auto b = sessions.data();
		const auto e = b + sessions.size();
		if (session_vecHash.HasItem(exceptSessionNumber))
		{
			session_vecHash.SwapElement((*b++)->GetObjectID(), exceptSessionNumber);
		}
		while (e != b) { (*b++)->GetSession()->SendAsync(pSendBuffer); }
	}

	void Sector::LeaveAndDestroy(const uint8_t group_type, const uint32_t obj_id)noexcept
	{
		if (const auto entity = m_vectorHashMapForEntity[group_type].ExtractItemSafe(obj_id))
		{
			entity->Destroy();
			m_pParentWorld->DecRef();
			entity->DecRef();
		}
	}

	void Sector::Leave(const uint8_t group_type, const uint32_t obj_id) noexcept
	{
		if (const auto entity = m_vectorHashMapForEntity[group_type].ExtractItemSafe(obj_id))
		{
			// TODO: 단순히 나가기만 했을 때 할 것
		}
	}
}