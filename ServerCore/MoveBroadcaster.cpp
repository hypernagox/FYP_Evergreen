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
	thread_local VectorSetUnsafe<std::pair<uint32_t, const ContentsEntity*>, XHashMap> new_view_list_session;
	thread_local VectorSetUnsafe<const ContentsEntity*, XHashMap> new_view_list_npc;

	void MoveBroadcaster::BroadcastMove(const BroadcastHelper& helper) noexcept
	{
		extern thread_local VectorSetUnsafe<std::pair<uint32_t, const ContentsEntity*>, XHashMap> new_view_list_session;
		extern thread_local VectorSetUnsafe<const ContentsEntity*, XHashMap> new_view_list_npc;
		new_view_list_session.GetItemListRef().clear();
		
		const auto pOwnerEntity = GetOwnerEntityRaw();

		const auto clusters = ClusterInfoHelper::GetAdjClusters(pOwnerEntity);

		const auto thisSession = pOwnerEntity->GetSession();
		
		const auto move_pkt = helper.CreateMovePacket(pOwnerEntity);
		const auto add_pkt = helper.CreateAddPacket(pOwnerEntity);
		
		for (const Cluster* const cluster : clusters)
		{
			const auto& sessions = cluster->GetSessions();
			auto b = sessions.data();
			const auto e = b + sessions.size();
			while (e != b)
			{
				const ContentsEntity* const entity_ptr = (*b++);
				if (pOwnerEntity != entity_ptr && helper.Filter4Session(pOwnerEntity, entity_ptr))
				{
					TryInsertSession(helper, add_pkt, move_pkt, std::make_pair(entity_ptr->GetObjectID(), entity_ptr), thisSession);
				}
			}
		}

		ProcessTryEraseSession(helper, thisSession);

		new_view_list_npc.GetItemListRef().clear();

		for (const Cluster* const cluster : clusters)
		{
			for (const auto& entities : cluster->GetEntitesExceptSession())
			{
				const auto& entity_list = entities.GetItemListRef();
				auto b = entity_list.data();
				const auto e = b + entity_list.size();
				while (e != b)
				{
					const ContentsEntity* const entity_ptr = (*b++);
					if (helper.Filter4NPC(pOwnerEntity, entity_ptr))
					{
						TryInsertNPC(helper, entity_ptr, thisSession);
					}
				}
			}
		}

		ProcessTryEraseNPC(helper, thisSession);
	}

	void MoveBroadcaster::BroadcastPacket(const S_ptr<SendBuffer>& pSendBuff_) const noexcept
	{
		const auto g_main_server = ServerService::GetMainService();
		const auto& session_list = m_view_list_session.GetItemListRef();
		auto b = session_list.data();
		const auto e = b + session_list.size();
		while (e != b) { 
			if (const auto entity = g_main_server->GetSession((*b++).first))
				entity->GetSession()->SendAsync(pSendBuff_);
		}
	}
	XVector<S_ptr<ContentsEntity>> MoveBroadcaster::GetSptrSession() const noexcept
	{
		const auto& session_list = m_view_list_session.GetItemListRef();
		auto b = session_list.data();
		const auto e = b + session_list.size();
		XVector<S_ptr<ContentsEntity>> temp;
		temp.reserve(e - b);
		while (e != b) { temp.emplace_back((*b++).second); }
		return temp;
	}
	XVector<S_ptr<ContentsEntity>> MoveBroadcaster::GetSptrNPC() const noexcept
	{
		const auto& npc_list = m_view_list_npc.GetItemListRef();
		auto b = npc_list.data();
		const auto e = b + npc_list.size();
		XVector<S_ptr<ContentsEntity>> temp;
		temp.reserve(e - b);
		while (e != b) { temp.emplace_back((*b++)); }
		return temp;
	}

	void MoveBroadcaster::TryInsertSession(
		  const BroadcastHelper& helper
		, const S_ptr<SendBuffer>& add_pkt
		, const S_ptr<SendBuffer>& move_pkt
		, const std::pair<uint32_t, const ContentsEntity*>&& id_ptr
		, const PacketSession* const thisSession) noexcept
	{
		extern thread_local VectorSetUnsafe<std::pair<uint32_t, const ContentsEntity*>, XHashMap> new_view_list_session;
		new_view_list_session.AddItem(id_ptr);
		const auto pSession = id_ptr.second->GetSession();
		if (m_view_list_session.AddItem(id_ptr))
		{
			thisSession->SendAsync(helper.CreateAddPacket(id_ptr.second));

			pSession->SendAsync(add_pkt);
		}
		else
		{
			pSession->SendAsync(move_pkt);
		}
	}

	void MoveBroadcaster::TryInsertNPC(
		  const BroadcastHelper& helper
		, const ContentsEntity* const entity_ptr
		, const PacketSession* const thisSession) noexcept
	{
		extern thread_local VectorSetUnsafe<const ContentsEntity*, XHashMap> new_view_list_npc;
		new_view_list_npc.AddItem(entity_ptr);
		if (m_view_list_npc.AddItem(entity_ptr))
		{
			entity_ptr->IncRef();
			thisSession->SendAsync(helper.CreateAddPacket(entity_ptr));
		}
	}

	void MoveBroadcaster::ProcessTryEraseSession(const BroadcastHelper& helper, const PacketSession* const thisSession) noexcept
	{
		extern thread_local VectorSetUnsafe<std::pair<uint32_t, const ContentsEntity*>, XHashMap> new_view_list_session;
		const auto service = Service::GetMainService();
		const auto remove_pkt = helper.CreateRemovePacket(GetOwnerObjectID());
		auto& view_list_session = m_view_list_session.GetItemListRef();
		for (auto iter = view_list_session.begin(); iter != view_list_session.end();)
		{
			const auto& id_ptr = *iter;

			if (!new_view_list_session.TryEraseItem(id_ptr))
			{
				iter = m_view_list_session.EraseItemAndGetIter(id_ptr);

				thisSession->SendAsync(helper.CreateRemovePacket(id_ptr.first));

				if (const auto entity_ptr = service->GetSession(id_ptr.first))
					entity_ptr->GetSession()->SendAsync(remove_pkt);
			}
			else
			{
				++iter;
			}
		}
	}

	void MoveBroadcaster::ProcessTryEraseNPC(const BroadcastHelper& helper, const PacketSession* const thisSession) noexcept
	{
		extern thread_local VectorSetUnsafe<const ContentsEntity*, XHashMap> new_view_list_npc;
		auto& view_list_npc = m_view_list_npc.GetItemListRef();
		for (auto iter = view_list_npc.begin(); iter != view_list_npc.end();)
		{
			const ContentsEntity* const entity_ptr = *iter;

			if (!new_view_list_npc.TryEraseItem(entity_ptr))
			{
				const auto id = entity_ptr->GetObjectID();
				iter = m_view_list_npc.EraseItemAndGetIter(entity_ptr);

				entity_ptr->DecRef();
				thisSession->SendAsync(helper.CreateRemovePacket(id));
			}
			else
			{
				++iter;
			}
		}
	}
}