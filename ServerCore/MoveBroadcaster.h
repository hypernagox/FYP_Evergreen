#pragma once
#include "ServerCorePch.h"
#include "ContentsComponent.h"

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
		
		~MoveBroadcaster()noexcept = default;
		using HuristicFunc = bool(*)(const ContentsEntity* const, const ContentsEntity* const)noexcept;
		using PacketFunc = S_ptr<SendBuffer>(*)(const ContentsEntity* const)noexcept;
	public:
		void BroadcastMove()noexcept;
		void BroadcastPacket(const S_ptr<SendBuffer>& pSendBuff_)const noexcept;
		Vector<S_ptr<ContentsEntity>> GetSptrSession()const noexcept;
		Vector<S_ptr<ContentsEntity>> GetSptrNPC()const noexcept;
		const auto& GetViewListSession()const noexcept { return m_view_list_session.GetItemListRef(); }
		const auto& GetViewListNPC()const noexcept { return m_view_list_npc.GetItemListRef(); }
	public:
		static void RegisterHuristicFunc2Session(const HuristicFunc fp_)noexcept {
			if (g_huristic[0])return;
			g_huristic[0] = fp_;
		}
		static void RegisterHuristicFunc2NPC(const HuristicFunc fp_)noexcept {
			if (g_huristic[1])return;
			g_huristic[1] = fp_;
		}
		static void RegisterAddPacketFunc(const PacketFunc fp_)noexcept {
			if (g_create_add_pkt)return;
			g_create_add_pkt = fp_;
		}
		static void RegisterRemovePacketFunc(S_ptr<SendBuffer>(*const fp_)(const uint32_t)noexcept)noexcept {
			if (g_create_remove_pkt)return;
			g_create_remove_pkt = fp_;
		}
		static void RegisterMovePacketFunc(const PacketFunc fp_)noexcept {
			if (g_create_move_pkt)return;
			g_create_move_pkt = fp_;
		}
	public:
		static S_ptr<SendBuffer> CreateAddPacket(const ContentsEntity* const pEntity_)noexcept {
			return g_create_add_pkt(pEntity_);
		}
		static S_ptr<SendBuffer> CreateMovePacket(const ContentsEntity* const pEntity_)noexcept {
			return g_create_move_pkt(pEntity_);
		}
		static S_ptr<SendBuffer> CreateAddPacket(const S_ptr<ContentsEntity>& pEntity_)noexcept {
			return g_create_add_pkt(pEntity_.get());
		}
		static S_ptr<SendBuffer> CreateMovePacket(const S_ptr<ContentsEntity>& pEntity_)noexcept {
			return g_create_move_pkt(pEntity_.get());
		}
		static S_ptr<SendBuffer> CreateRemovePacket(const uint32_t obj_id)noexcept {
			return g_create_remove_pkt(obj_id);
		}
	private:
		ServerCore::VectorSetUnsafe<std::pair<uint32_t,const ContentsEntity*>> m_view_list_session;
		ServerCore::VectorSetUnsafe<std::pair<uint32_t,const ContentsEntity*>> m_view_list_npc;
	private:
		static inline HuristicFunc g_huristic[2] = {};
		static inline PacketFunc g_create_add_pkt = {};
		static inline S_ptr<SendBuffer>(*g_create_remove_pkt)(const uint32_t)noexcept;
		static inline PacketFunc g_create_move_pkt = {};
	};
}

