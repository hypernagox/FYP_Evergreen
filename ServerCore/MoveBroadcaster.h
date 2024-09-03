#pragma once
#include "ID_Ptr.hpp"

namespace ServerCore
{
	class ContentsEntity;
	class Session;
	class SendBuffer;
	class PacketSession;
	class Sector;
	class World;

	enum SECTOR_STATE
	{
		NOT_EMPTY = 0,

		USER_EMPTY = 1 << 0,
		NPC_EMPTY = 1 << 1,
		
		EMPTY = 3,
		STOP = 1 << 2,

		IDLE,
		WORK,

		NONE,
	};

	class MoveBroadcaster
	{
	public:
		MoveBroadcaster(const ContentsEntity* const pOwnerEntity)noexcept :m_pOwnerEntity{ pOwnerEntity } { m_vecViewListForCopy.reserve(1024); }
		~MoveBroadcaster()noexcept;
		using HuristicFunc = bool(*)(const ContentsEntity* const, const ContentsEntity* const)noexcept;
		using PacketFunc = S_ptr<SendBuffer>(*)(const ContentsEntity* const)noexcept;
	public:
		void ProcessAddObject(const Session*const owner_session, S_ptr<ContentsEntity> other_entity)noexcept;
		void ProcessRemoveObject(const Session* const owner_session, S_ptr<ContentsEntity> other_entity)noexcept;
	public:
		const int BroadcastMove(const float x, const float y, const Vector<Sector*> sectors)noexcept;
		void ReleaseViewList()noexcept;
		template <typename T = World>
		constexpr inline const World* const GetCurWorld()const noexcept { return static_cast<const T* const>(m_curWorld.load(std::memory_order_acquire)); }
		inline void SetWorld(const World* const pWorld)noexcept { m_curWorld.store(pWorld, std::memory_order_release); }
		inline void InitSectorXY(const uint8_t x, const uint8_t y)noexcept { m_cur_sectorXY = Point2D{ x,y }; }
		inline const auto& GetViewListCopy()const noexcept 
		{
			extern thread_local Vector<uint64_t> LViewListForCopy;
			m_srwLock.lock_shared();
			LViewListForCopy = m_vecViewListForCopy;
			m_srwLock.unlock_shared();
			return LViewListForCopy;
		}
		inline const auto& GetViewListUnsafe()const noexcept { return m_viewList; }
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
		Point2D m_cur_sectorXY;
		HashSet<const ContentsEntity*> m_viewList;
		const ContentsEntity* const m_pOwnerEntity;
		std::atomic<const World*> m_curWorld;
		Vector<uint64_t> m_vecViewListForCopy;
		SRWLock m_srwLock;
	private:
		static inline HuristicFunc g_huristic[2] = {};
		static inline PacketFunc g_create_add_pkt = {};
		static inline PacketFunc g_create_remove_pkt = {};
		static inline PacketFunc g_create_move_pkt = {};
	};
}
