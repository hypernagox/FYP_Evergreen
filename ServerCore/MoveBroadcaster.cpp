#include "ServerCorePch.h"
#include "MoveBroadcaster.h"
#include "PacketSession.h"
#include "Sector.h"
#include "World.h"
#include "Queueabler.h"
#include "WorldMgr.h"

namespace ServerCore
{
	thread_local Vector<uint32_t> LViewListForCopy(1024);

	void MoveBroadcaster::BroadcastMove()noexcept
	{
		extern thread_local Vector<uint32_t> LViewListForCopy;
		thread_local HashSet<const ContentsEntity*> new_view_list(1024);
		
		const auto pOwnerEntity = GetOwnerEntityRaw();

		const auto sectors = SectorInfoHelper::GetAdjSectors(pOwnerEntity);

		const auto thisSession = pOwnerEntity->GetSession();
		const auto obj_type = pOwnerEntity->GetObjectType();
		const auto obj_id = pOwnerEntity->GetObjectID();

		const auto add_pkt_func = g_create_add_pkt;
		const auto remove_pkt_func = g_create_remove_pkt;

		const auto move_pkt = g_create_move_pkt(pOwnerEntity);
		const auto add_pkt = add_pkt_func(pOwnerEntity);
		const auto remove_pkt = remove_pkt_func(pOwnerEntity);

		new_view_list.clear();

		const auto huristic_func = g_huristic;

		m_spinLock.lock();
		const S_ptr<ViewListWrapper> viewListPtr{ m_viewListPtr };
		m_spinLock.unlock();

		if (!viewListPtr)
			return;

		auto& m_viewList = viewListPtr->view_list;

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

					if (pSession)
					{
						pSession->SendAsync(add_pkt);
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
				const auto pSession = entity_ptr->GetSession();

				thisSession->SendAsync(remove_pkt_func(entity_ptr));

				if (pSession)
				{
					pSession->SendAsync(remove_pkt);
				}
			}
			else
			{
				++iter;
			}
			entity_ptr->DecRef();
		}
		
		m_srwLock.lock();
		m_vecViewListForCopy.swap(LViewListForCopy);
		m_srwLock.unlock();
	}

	MoveBroadcaster::ViewListWrapper::~ViewListWrapper() noexcept
	{
		for (const auto pEntity : view_list)pEntity->DecRef();
	}
}