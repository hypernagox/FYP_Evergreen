#include "ServerCorePch.h"
#include "MoveBroadcaster.h"
#include "PacketSession.h"
#include "Sector.h"
#include "Queueabler.h"
#include "Cluster.h"
#include "ClusterInfoHelper.h"
#include "Service.h"

namespace ServerCore
{
	thread_local HashMap<uint32_t, const ContentsEntity* const> new_view_list_session(512);
	thread_local HashMap<uint32_t, const ContentsEntity* const> new_view_list_npc(512);

	void MoveBroadcaster::BroadcastMove() noexcept
	{
		extern thread_local HashMap<uint32_t, const ContentsEntity* const> new_view_list_session;
		extern thread_local HashMap<uint32_t, const ContentsEntity* const> new_view_list_npc;

		const auto pOwnerEntity = GetOwnerEntityRaw();

		const auto clusters = ClusterInfoHelper::GetAdjClusters(pOwnerEntity);

		const auto thisSession = pOwnerEntity->GetSession();
		const auto obj_type = pOwnerEntity->GetObjectType();
		const auto obj_id = pOwnerEntity->GetObjectID();

		const auto add_pkt_func = g_create_add_pkt;
		const auto remove_pkt_func = g_create_remove_pkt;

		const auto move_pkt = g_create_move_pkt(pOwnerEntity);
		const auto add_pkt = add_pkt_func(pOwnerEntity);
		const auto remove_pkt = remove_pkt_func(obj_id);

		const auto service = static_cast<const ServerService* const>(Service::GetMainService());
		
		const auto huristic_func = g_huristic;
		
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
						new_view_list_session.emplace(entity_ptr->GetObjectID(), entity_ptr);
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
						new_view_list_npc.emplace(entity_ptr->GetObjectID(), entity_ptr);
					}
				}
			}
		}

		const auto it = new_view_list_session.find(obj_id);
		if (new_view_list_session.cend() != it)new_view_list_session.erase(it);


		const auto e_iter = new_view_list_session.cend();
		{
			for (auto iter = new_view_list_session.cbegin(); iter != e_iter; ++iter)
			{
				const auto [id, entity_ptr] = *iter;
				const auto pSession = entity_ptr->GetSession();

				if (m_view_list_session.emplace(id).second)
				{
					thisSession->SendAsync(add_pkt_func(entity_ptr));

					pSession->SendAsync(add_pkt);
				}
				else
				{
					pSession->SendAsync(move_pkt);
				}
			}
		}
		{
			for (auto iter = m_view_list_session.cbegin(); iter != m_view_list_session.cend();)
			{
				const auto entity_session_id = *iter;
				if (!new_view_list_session.erase(entity_session_id))
				{
					iter = m_view_list_session.erase(iter);

					thisSession->SendAsync(remove_pkt_func(entity_session_id));

					if (const auto entity_ptr = service->GetSession(entity_session_id))
						entity_ptr->GetSession()->SendAsync(remove_pkt);
				}
				else
				{
					++iter;
				}
			}
		}

		const auto e_iter2 = new_view_list_npc.cend();
		{
			for (auto iter = new_view_list_npc.cbegin(); iter != e_iter2; ++iter)
			{
				const auto [id, entity_ptr_npc] = *iter;

				if (m_view_list_npc.emplace(id).second)
				{
					thisSession->SendAsync(add_pkt_func(entity_ptr_npc));
				}
			}
		}
	
		{
			for (auto iter = m_view_list_npc.cbegin(); iter != m_view_list_npc.cend();)
			{
				const auto entity_npc_id = *iter;
				if (!new_view_list_npc.erase(entity_npc_id))
				{
					iter = m_view_list_npc.erase(iter);

					thisSession->SendAsync(remove_pkt_func(entity_npc_id));
				}
				else
				{
					++iter;
				}
			}
		}
	}
}