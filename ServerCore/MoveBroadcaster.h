#pragma once
#include "ServerCorePch.h"
#include "ContentsComponent.h"
#include "BroadcastHelper.h"

namespace ServerCore
{
	class ContentsEntity;
	class Session;
	class SendBuffer;
	class PacketSession;
	class Sector;

	class MoveBroadcaster
		:public ContentsComponent
	{
		friend class ClusterInfoHelper;
	public:
		CONSTRUCTOR_CONTENTS_COMPONENT(MoveBroadcaster)
		
		~MoveBroadcaster()noexcept { 
			for (const auto npc_ptr : m_view_list_npc.GetItemListRef())npc_ptr->DecRef();
		}
	public:
		void BroadcastMove(const BroadcastHelper& helper)noexcept;
		void RequestHandler(const BroadcastHandler& handler)noexcept { handler.HandleBroadcast(this); }
		void BroadcastPacket(const S_ptr<SendBuffer>& pSendBuff_)const noexcept;
		XVector<S_ptr<ContentsEntity>> GetSptrSession()const noexcept;
		XVector<S_ptr<ContentsEntity>> GetSptrNPC()const noexcept;
		const auto& GetViewListSession()const noexcept { return m_view_list_session.GetItemListRef(); }
		const auto& GetViewListNPC()const noexcept { return m_view_list_npc.GetItemListRef(); }
	public:
		static void RegisterGlobalHelper(BroadcastHelper* const helper)noexcept { 
			NAGOX_ASSERT(nullptr == g_global_helper);
			g_global_helper = helper;
		}
	public:
		static S_ptr<SendBuffer> CreateAddPacket(const ContentsEntity* const pEntity_)noexcept {
			return g_global_helper->CreateAddPacket(pEntity_);
		}
		static S_ptr<SendBuffer> CreateMovePacket(const ContentsEntity* const pEntity_)noexcept {
			return g_global_helper->CreateMovePacket(pEntity_);
		}
		static S_ptr<SendBuffer> CreateAddPacket(const S_ptr<ContentsEntity>& pEntity_)noexcept {
			return g_global_helper->CreateAddPacket(pEntity_.get());
		}
		static S_ptr<SendBuffer> CreateMovePacket(const S_ptr<ContentsEntity>& pEntity_)noexcept {
			return g_global_helper->CreateAddPacket(pEntity_.get());
		}
		static S_ptr<SendBuffer> CreateRemovePacket(const uint32_t obj_id)noexcept {
			return g_global_helper->CreateRemovePacket(obj_id);
		}
		static bool GlobalFilter4Session(const ContentsEntity* const a, const ContentsEntity* const b)noexcept{
			return g_global_helper->Filter4Session(a, b);
		}
		static bool GlobalFilter4NPC(const ContentsEntity* const a, const ContentsEntity* const b)noexcept {
			return g_global_helper->Filter4NPC(a, b);
		}
		static const auto GetGlobalBroadcastHelper()noexcept { return g_global_helper; }
	private:
		void TryInsertSession(
			  const BroadcastHelper& helper
			, const S_ptr<SendBuffer>& add_pkt
			, const S_ptr<SendBuffer>& move_pkt
			, const std::pair<uint32_t, const ContentsEntity*>&& id_ptr
			, const PacketSession* const thisSession)noexcept;
		void TryInsertNPC(
			  const BroadcastHelper& helper
			, const ContentsEntity* const entity_ptr
			, const PacketSession* const thisSession)noexcept;
	private:
		void ProcessTryEraseSession(const BroadcastHelper& helper, const PacketSession* const thisSession)noexcept;
		void ProcessTryEraseNPC(const BroadcastHelper& helper, const PacketSession* const thisSession)noexcept;
	private:
		ServerCore::VectorSetUnsafe<std::pair<uint32_t,const ContentsEntity*>> m_view_list_session;
		ServerCore::VectorSetUnsafe<const ContentsEntity*> m_view_list_npc;
	private:
		constinit static inline BroadcastHelper* g_global_helper = nullptr;
	};
}

