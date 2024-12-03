#include "ServerCorePch.h"
#include "MoveBroadcaster.h"
#include "PacketSession.h"
#include "Sector.h"
#include "Queueabler.h"
#include "Cluster.h"
#include "ClusterInfoHelper.h"

namespace ServerCore
{
	thread_local HashSet<const ContentsEntity*> new_view_list(1024);
	thread_local Vector<uint64_t> LViewListForCopy;

	void MoveBroadcaster::BroadcastMove() noexcept
	{
		extern thread_local HashSet<const ContentsEntity*> new_view_list;

		const auto pOwnerEntity = GetOwnerEntityRaw();

		const auto clusters = ClusterInfoHelper::GetAdjClusters(pOwnerEntity);

		const auto thisSession = pOwnerEntity->GetSession();
		const auto obj_type = pOwnerEntity->GetObjectType();
		const auto obj_id = pOwnerEntity->GetObjectID();

		const auto add_pkt_func = g_create_add_pkt;
		const auto remove_pkt_func = g_create_remove_pkt;

		const auto move_pkt = g_create_move_pkt(pOwnerEntity);
		const auto add_pkt = add_pkt_func(pOwnerEntity);
		const auto remove_pkt = remove_pkt_func(pOwnerEntity);

		new_view_list.clear();
		LViewListForCopy.clear();

		const auto huristic_func = g_huristic;

		if (false == m_spinLock.try_lock())return;
		const S_ptr<ViewListWrapper> viewListPtr{ m_viewListPtr };
		m_spinLock.unlock();

		if (!viewListPtr)
			return;

		auto& m_viewList = viewListPtr->view_list;

		for (const auto& cluster : clusters)
		{
			const auto& entities = cluster->GetAllEntites();
			auto b = entities.data();
			const auto e = b + entities.size();
			{
				const auto& sessions = (*b++).GetItemListRef();
				auto b = sessions.data();
				const auto e = b + sessions.size();
				while (e != b)
				{
					const auto entity_ptr = (*b++);
					if (huristic_func[0](pOwnerEntity, entity_ptr))
					{
						new_view_list.emplace(entity_ptr);
					}
				}
			}
			while (e != b) 
			{
				const auto& entities = (*b++).GetItemListRef();
				auto b = entities.data();
				const auto e = b + entities.size();
				while (e != b)
				{
					const auto entity_ptr = (*b++);
					if (huristic_func[1](pOwnerEntity, entity_ptr))
					{
						new_view_list.emplace(entity_ptr);
					}
				}
			}
		}

		const auto it = new_view_list.find(pOwnerEntity);
		if (new_view_list.cend() != it)new_view_list.erase(it);

		const auto e_iter = new_view_list.cend();
		for (auto iter = new_view_list.cbegin(); iter != e_iter;++iter)
		{
			const auto entity_ptr = *iter;
			const auto pSession = entity_ptr->GetSession();

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
		
				entity_ptr->DecRef();
			}
			else
			{
				LViewListForCopy.emplace_back(entity_ptr->GetObjectCombineID());
				++iter;
			}
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