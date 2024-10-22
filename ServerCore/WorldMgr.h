#pragma once
#include "Singleton.hpp"

namespace ServerCore
{
	class World;
	class Field;
	class Cluster;

	class WorldMgr
		:public Singleton<WorldMgr>
	{
		friend class ContentsEntity;
		friend class World;
		struct alignas(64) AtomicNPCPtr
		{
			AtomicS_ptr<ContentsEntity> ptr;
		};
		friend class Singleton;
		WorldMgr();
		~WorldMgr();
	public:
		template <typename T, typename... Args>
		T* const RegisterWolrd(const uint8_t worldID, Args&&... args)noexcept {
			auto temp = MakeShared<T>(std::forward<Args>(args)...);
			const auto temp_ptr = temp.get();
			m_mapWorld.emplace(worldID, std::move(temp));
			temp_ptr->InitWorld();
			return temp_ptr;
		}

		template <typename T, typename... Args>
		T* RegisterField(const uint8_t fieldID, Args&&... args)noexcept {
			const auto temp = xnew<T>(std::forward<Args>(args)...);
			temp->InitField();
			m_mapField.emplace(fieldID, temp);
			return temp;
		}
		template <typename T = Field>
		inline T* GetField(const uint8_t fieldID)noexcept {
			return static_cast<T* const>(m_mapField[fieldID]);
		}

		template <typename T = Cluster>
		inline T* GetCluster(const uint8_t fieldID,const Point2D sectorID)noexcept {
			return GetField(fieldID)->GetCluster(sectorID);
		}

		template <typename T = World>
		inline S_ptr<T> GetWorld(const uint8_t worldID)noexcept { return ServerCore::StaticCast<T>(m_mapWorld[worldID]); }
		S_ptr<ContentsEntity> GetNPC(const uint32_t npc_id)const noexcept;
		void ClearWorld()const noexcept;
	public:
		template <const int32_t num_of_npc>
		constexpr void SetNumOfNPC()noexcept
		{
			if (!m_arrNPC.empty())return;
			static AtomicNPCPtr arr_npc[num_of_npc];
			m_arrNPC = arr_npc;
			m_idxQueue.set_capacity(num_of_npc + 1);
			m_id2Index.reserve(num_of_npc * 2);
			for (int i = 1; i <= num_of_npc; ++i)
			{
				m_idxQueue.push(i);
			}
		}
	private:
		void RegisterNPC(S_ptr<ContentsEntity>& pNPC)noexcept;
		void ReleaseNPC(const ContentsEntity* const pNPC)noexcept;

	private:
		HashMap<uint8_t, Field*> m_mapField;

		tbb::concurrent_unordered_map<uint8_t, S_ptr<World>> m_mapWorld;
		
		mutable tbb::concurrent_unordered_map<uint32_t, uint16_t, std::hash<uint32_t>, std::equal_to<uint32_t>> m_id2Index;
		std::span<AtomicNPCPtr> m_arrNPC;
		tbb::concurrent_bounded_queue<int32> m_idxQueue;
	};
}

