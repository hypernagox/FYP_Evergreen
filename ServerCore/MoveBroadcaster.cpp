#include "ServerCorePch.h"
#include "MoveBroadcaster.h"
#include "PacketSession.h"
#include "Sector.h"
#include "World.h"
#include "Queueabler.h"

namespace ServerCore
{
	thread_local Vector<uint64_t> LViewListForCopy(1024);

	MoveBroadcaster::~MoveBroadcaster() noexcept
	{
		for (const auto pEntity : m_viewList)pEntity->DecRef();
	}
	void MoveBroadcaster::ProcessAddObject(const Session* const owner_session, S_ptr<ContentsEntity> other_entity) noexcept
	{
		const auto other_entity_ptr = other_entity.get();
		if (owner_session->IsConnected() && m_viewList.emplace(other_entity_ptr).second)
		{
			owner_session->SendAsync(g_create_add_pkt(other_entity_ptr));
			other_entity.release();
		}
		
	}
	void MoveBroadcaster::ProcessRemoveObject(const Session* const owner_session, S_ptr<ContentsEntity> other_entity) noexcept
	{
		const auto other_entity_ptr = other_entity.get();
		if (m_viewList.erase(other_entity_ptr))
		{
			other_entity.release();
			owner_session->SendAsync(g_create_remove_pkt(other_entity_ptr));
			other_entity_ptr->DecRef(2);
		}
	}
	const int MoveBroadcaster::BroadcastMove(const float x, const float y, const Vector<Sector*> sectors)noexcept
	{
		extern thread_local Vector<uint64_t> LViewListForCopy;
		thread_local HashSet<const ContentsEntity*> new_view_list(1024);
		
		int sector_state = 0;

		const auto pOwnerEntity = m_pOwnerEntity;
		const auto thisSession = pOwnerEntity->GetSession();
		const auto obj_type = pOwnerEntity->GetObjectType();
		const auto obj_id = pOwnerEntity->GetObjectID();
		const auto cur_world = m_curWorld.load(std::memory_order_relaxed);
		const auto cur_XY = m_cur_sectorXY;
		
		const auto add_pkt_func = g_create_add_pkt;
		const auto remove_pkt_func = g_create_remove_pkt;

		const auto move_pkt = g_create_move_pkt(pOwnerEntity);
		const auto add_pkt = add_pkt_func(pOwnerEntity);
		const auto remove_pkt = remove_pkt_func(pOwnerEntity);

		new_view_list.clear();

		// TODO: 나중에 몬스터 세션이 추가된다면 몬스터 + 세션인 경우를 한번 더 나누어야함
		// 밑에 섹터검색만 케이스 나누면 될 듯
		if (thisSession)
		{
			const auto huristic_func = g_huristic;

			for (const auto sector : sectors)
			{
				for (const auto& entities : sector->GetEntities())
				{
					const auto& entity_lock = entities.GetSRWLock();
					const auto& entity_list = entities.GetItemListRef();
					entity_lock.lock_shared();
					auto b = entity_list.data();
					const auto e = b + entity_list.size();
					while (e != b) {
						const ContentsEntity* const __restrict pEntity = *b++;
						if (new_view_list.emplace(pEntity).second)
							pEntity->IncRef();
					}
					entity_lock.unlock_shared();
				}
			}

			if (new_view_list.erase(pOwnerEntity))pOwnerEntity->DecRef();

			for (auto iter = new_view_list.cbegin(); iter != new_view_list.cend();)
			{
				const auto entity_ptr = *iter;
				const auto pSession = entity_ptr->GetSession();
				if (huristic_func[!pSession](pOwnerEntity, entity_ptr))
				{
					if (m_viewList.emplace(entity_ptr).second)
					{
						thisSession->SendAsync(add_pkt_func(entity_ptr));

						if (pSession) {
							entity_ptr->GetQueueabler()->EnqueueBroadcastEvent(&MoveBroadcaster::ProcessAddObject, entity_ptr->GetMoveBroadcaster(),
								static_cast<const Session* const>(pSession), pOwnerEntity->SharedFromThis());
						}
						
						entity_ptr->IncRef();
					}
					else
					{
						if (pSession)
							pSession->SendAsync(move_pkt);
					}
					++iter;
				}
				else
				{
					iter = new_view_list.erase(iter);
					entity_ptr->DecRef();
				}
			}

			for (auto iter = m_viewList.cbegin(); iter != m_viewList.cend();)
			{
				const auto entity_ptr = *iter;
				if (!new_view_list.erase(entity_ptr))
				{
					iter = m_viewList.erase(iter);

					thisSession->SendAsync(remove_pkt_func(entity_ptr));

					if (const auto pSession = entity_ptr->GetSession()) {
						entity_ptr->GetQueueabler()->EnqueueBroadcastEvent(&MoveBroadcaster::ProcessRemoveObject, entity_ptr->GetMoveBroadcaster(),
							static_cast<const Session* const>(pSession), pOwnerEntity->SharedFromThis());
					}
				}
				else
				{
					++iter;
				}
				entity_ptr->DecRef();
			}
		}
		else
		{
			const auto huristic_func = g_huristic[0];

			for (const auto sector : sectors)
			{
				const auto& session_vecHash = sector->GetSessionVectorHash();
				const auto& session_lock = session_vecHash.GetSRWLock();
				const auto& session_list = session_vecHash.GetItemListRef();
				session_lock.lock_shared();
				auto b = session_list.data();
				const auto e = b + session_list.size();
				while (e != b) { 
					const ContentsEntity* const __restrict pSessionEntity = *b++;
					if (new_view_list.emplace(pSessionEntity).second)
						pSessionEntity->IncRef();
				}
				session_lock.unlock_shared();
			}

			for (auto iter = new_view_list.cbegin(); iter != new_view_list.cend();)
			{
				const auto entity_ptr = *iter;
				if (huristic_func(pOwnerEntity, entity_ptr))
				{
					const auto pSession = entity_ptr->GetSession();

					if (m_viewList.emplace(entity_ptr).second)
					{
						entity_ptr->GetQueueabler()->EnqueueBroadcastEvent(&MoveBroadcaster::ProcessAddObject, entity_ptr->GetMoveBroadcaster(),
							static_cast<const Session* const>(pSession), pOwnerEntity->SharedFromThis());

						entity_ptr->IncRef();
					}
					else
					{
						pSession->SendAsync(move_pkt);
					}
					++iter;
				}
				else
				{
					iter = new_view_list.erase(iter);
					entity_ptr->DecRef();
				}
			}

			sector_state |= !new_view_list.empty();

			LViewListForCopy.clear();

			for (auto iter = m_viewList.cbegin(); iter != m_viewList.cend();)
			{
				const auto entity_ptr = *iter;
				if (!new_view_list.erase(entity_ptr))
				{
					iter = m_viewList.erase(iter);
					LViewListForCopy.emplace_back(entity_ptr->GetObjectCombineID());

					entity_ptr->GetQueueabler()->EnqueueBroadcastEvent(&MoveBroadcaster::ProcessRemoveObject, entity_ptr->GetMoveBroadcaster(),
						static_cast<const Session* const>(entity_ptr->GetSession()), pOwnerEntity->SharedFromThis());
				}
				else
				{
					++iter;
				}
				entity_ptr->DecRef();
			}
		}
		
		const Point2D next_sector = cur_world->CalculateSectorXY(x,y);

		if (next_sector != cur_XY)
		{
			cur_world->GetSector(cur_XY)->MigrationEnqueue(cur_world->GetSector(next_sector)->SharedFromThis(), obj_type, obj_id);
			m_cur_sectorXY = next_sector;
		}

		m_srwLock.lock();
		m_vecViewListForCopy.swap(LViewListForCopy);
		m_srwLock.unlock();

		return 3 ^ sector_state;
	}

	void MoveBroadcaster::ReleaseViewList() noexcept
	{
		for (const auto pEntity : m_viewList)pEntity->DecRef();
		m_viewList.clear();
	}
}