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
		inline Vector<uint64_t> GetViewListCopy()const noexcept
		{
			m_srwLock.lock_shared();
			Vector<uint64_t> viewListForCopy{ m_vecViewListForCopy };
			m_srwLock.unlock_shared();
			return viewListForCopy;
		}
		
		// 자기 자신의 작업을 처리 할 때 만 사용 가능하다.
		inline const auto& GetMyViewList()const noexcept { return  m_vecViewListForCopy; }

		void ReleaseViewList()noexcept
		{
			m_spinLock.lock();
			const S_ptr<ViewListWrapper> temp{ std::move(m_viewListPtr) };
			m_spinLock.unlock();
		}
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
		static void RegisterRemovePacketFunc(const PacketFunc fp_)noexcept {
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
		static S_ptr<SendBuffer> CreateRemovePacket(const ContentsEntity* const pEntity_)noexcept {
			return g_create_remove_pkt(pEntity_);
		}
		static S_ptr<SendBuffer> CreateMovePacket(const ContentsEntity* const pEntity_)noexcept {
			return g_create_move_pkt(pEntity_);
		}
		static S_ptr<SendBuffer> CreateAddPacket(const S_ptr<ContentsEntity>& pEntity_)noexcept {
			return g_create_add_pkt(pEntity_.get());
		}
		static S_ptr<SendBuffer> CreateRemovePacket(const S_ptr<ContentsEntity>& pEntity_)noexcept {
			return g_create_remove_pkt(pEntity_.get());
		}
		static S_ptr<SendBuffer> CreateMovePacket(const S_ptr<ContentsEntity>& pEntity_)noexcept {
			return g_create_move_pkt(pEntity_.get());
		}

	private:
		struct ViewListWrapper
			:public RefCountable {
			HashSet<const ContentsEntity*> view_list;
			~ViewListWrapper()noexcept;
		};
		SpinLock m_spinLock;
		S_ptr<ViewListWrapper> m_viewListPtr = MakeShared<ViewListWrapper>();
		Vector<uint64_t> m_vecViewListForCopy;
		SRWLock m_srwLock;
	private:
		static inline HuristicFunc g_huristic[2] = {};
		static inline PacketFunc g_create_add_pkt = {};
		static inline PacketFunc g_create_remove_pkt = {};
		static inline PacketFunc g_create_move_pkt = {};
	};
}

