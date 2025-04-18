#pragma once
#include "NagiocpXPch.h"
#include "Singleton.hpp"

namespace NagiocpX
{
	class Field;
	class Cluster;

	class FieldMgr
		:public Singleton<FieldMgr>
	{
		friend class ContentsEntity;
		friend class Field;
		friend class ThreadMgr;
		struct alignas(64) AtomicNPCPtr
		{
			AtomicS_ptr<ContentsEntity> ptr;
		};
		friend class Singleton;
		FieldMgr();
		~FieldMgr();
	public:
		template <typename T, typename... Args>
		T* RegisterField(const uint8_t fieldID, Args&&... args)noexcept {
			const auto temp = xnew<T>(std::forward<Args>(args)...);
			temp->InitFieldGlobal();
			m_mapField.emplace(fieldID, temp);
			return temp;
		}
		S_ptr<ContentsEntity> GetNPC(const uint32_t npc_id)const noexcept;
		void ClearField()const noexcept;
	public:
		template <const int32_t num_of_npc>
		constexpr void SetNumOfNPC()noexcept
		{
			if (!m_arrNPC.empty())return;
			static AtomicNPCPtr arr_npc[num_of_npc + 1];
			m_arrNPC = arr_npc;
			m_idxQueue.set_capacity(num_of_npc + 1);
			m_id2Index.reserve(num_of_npc * 2);
			for (int i = 1; i <= num_of_npc; ++i)
			{
				m_idxQueue.push(i);
			}
		}
	public:
		void RegisterNPC(S_ptr<ContentsEntity>& pNPC)noexcept;
		void RegisterNPC(S_ptr<ContentsEntity>&& pNPC)noexcept { RegisterNPC(pNPC); }
		void ReleaseNPC(const ContentsEntity* const pNPC)noexcept;
	public:
		const auto& GetFieldTable()const noexcept { return m_mapField; }
	private:
		void InitTLSinField();
		void DestroyTLSinField();

		void ShrinkToFitBeforeStart()noexcept;
	private:
		std::unordered_map<uint8_t, Field*> m_mapField;

		mutable tbb::concurrent_unordered_map<uint32_t, uint16_t, std::hash<uint32_t>, std::equal_to<uint32_t>> m_id2Index;
		std::span<AtomicNPCPtr> m_arrNPC;
		tbb::concurrent_bounded_queue<int32> m_idxQueue;
	};
}

