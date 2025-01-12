#include "ServerCorePch.h"
#include "MoveBroadcaster.h"
#include "PacketSession.h"
//#include "Sector.h"
#include "Queueabler.h"
#include "Cluster.h"
#include "ClusterInfoHelper.h"
#include "Service.h"

namespace ServerCore
{
	thread_local VectorSetUnsafe<const ContentsEntity*> new_view_list_session(512);
	thread_local VectorSetUnsafe<const ContentsEntity*> new_view_list_npc(512);

	void MoveBroadcaster::BroadcastMove() noexcept
	{
		extern thread_local VectorSetUnsafe<const ContentsEntity*> new_view_list_session;
		extern thread_local VectorSetUnsafe<const ContentsEntity*> new_view_list_npc;
		
		const auto pOwnerEntity = GetOwnerEntityRaw();

		const auto clusters = ClusterInfoHelper::GetAdjClusters(pOwnerEntity);

		const auto thisSession = pOwnerEntity->GetSession();
		const auto obj_id = pOwnerEntity->GetObjectID();

		const auto add_pkt_func = g_create_add_pkt;
		const auto remove_pkt_func = g_create_remove_pkt;

		const auto move_pkt = g_create_move_pkt(pOwnerEntity);
		const auto add_pkt = add_pkt_func(pOwnerEntity);
		const auto remove_pkt = remove_pkt_func(obj_id);

		const auto service = static_cast<const ServerService* const>(Service::GetMainService());
		
		const auto huristic_func = g_huristic;
		
		for (const Cluster* const cluster : clusters)
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
					const ContentsEntity* const entity_ptr = (*b++);
					if (huristic_func[0](pOwnerEntity, entity_ptr))
					{
						new_view_list_session.AddItem(entity_ptr);
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
					const ContentsEntity* const entity_ptr = (*b++);
					if (huristic_func[1](pOwnerEntity, entity_ptr))
					{
						new_view_list_npc.AddItem(entity_ptr);
					}
				}
			}
		}

		new_view_list_session.EraseItem(pOwnerEntity);

		{
			const auto& new_session_list = new_view_list_session.GetItemListRef();
			auto b = new_session_list.data();
			const auto e = b + new_session_list.size();
			while (e != b)
			{
				const ContentsEntity* const entity_ptr = (*b++);
				const auto id = entity_ptr->GetObjectID();
				const auto pSession = entity_ptr->GetSession();
				if (m_view_list_session.AddItem(std::make_pair(id, entity_ptr)))
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
			auto& view_list_session = m_view_list_session.GetItemListRef();
			for (auto iter = view_list_session.begin(); iter != view_list_session.end();)
			{
				const auto id_ptr = *iter;

				if (!new_view_list_session.TryEraseItem(id_ptr.second))
				{
					iter = m_view_list_session.EraseItemAndGetIter(id_ptr);

					thisSession->SendAsync(remove_pkt_func(id_ptr.first));

					if (const auto entity_ptr = service->GetSession(id_ptr.first))
						entity_ptr->GetSession()->SendAsync(remove_pkt);
				}
				else
				{
					++iter;
				}
			}
		}
	
		{
			const auto& new_npc_list = new_view_list_npc.GetItemListRef();
			auto b = new_npc_list.data();
			const auto e = b + new_npc_list.size();
			while (e != b)
			{
				const ContentsEntity* const entity_ptr = (*b++);
				if (m_view_list_npc.AddItem(entity_ptr))
				{
					entity_ptr->IncRef();
					thisSession->SendAsync(add_pkt_func(entity_ptr));
				}
			}
		}

		{
			auto& view_list_npc = m_view_list_npc.GetItemListRef();
			for (auto iter = view_list_npc.begin(); iter != view_list_npc.end();)
			{
				const ContentsEntity* const entity_ptr = *iter;

				if (!new_view_list_npc.TryEraseItem(entity_ptr))
				{
					const auto id = entity_ptr->GetObjectID();
					iter = m_view_list_npc.EraseItemAndGetIter(entity_ptr);

					entity_ptr->DecRef();
					thisSession->SendAsync(remove_pkt_func(id));
				}
				else
				{
					++iter;
				}
			}
		}
	}
	void MoveBroadcaster::BroadcastPacket(const S_ptr<SendBuffer>& pSendBuff_) const noexcept
	{
		const auto& session_list = m_view_list_session.GetItemListRef();
		auto b = session_list.data();
		const auto e = b + session_list.size();
		while (e != b) { (*b++).second->GetSession()->SendAsync(pSendBuff_); }
	}
	Vector<S_ptr<ContentsEntity>> MoveBroadcaster::GetSptrSession() const noexcept
	{
		const auto& session_list = m_view_list_session.GetItemListRef();
		auto b = session_list.data();
		const auto e = b + session_list.size();
		Vector<S_ptr<ContentsEntity>> temp;
		temp.reserve(e - b);
		while (e != b) { temp.emplace_back((*b++).second); }
		return temp;
	}
	Vector<S_ptr<ContentsEntity>> MoveBroadcaster::GetSptrNPC() const noexcept
	{
		const auto& npc_list = m_view_list_npc.GetItemListRef();
		auto b = npc_list.data();
		const auto e = b + npc_list.size();
		Vector<S_ptr<ContentsEntity>> temp;
		temp.reserve(e - b);
		while (e != b) { temp.emplace_back((*b++)); }
		return temp;
	}
}