#pragma once
#include "ClusterUpdateQueue.h"
#include "EBR.hpp"

namespace ServerCore
{
	// 1. 입장
	// 2. 퇴장
	// 3. 이주

	class Cluster
	{
		friend class Field;
	public:
		Cluster(const uint8_t num_of_groups, const ClusterInfo info)noexcept
			: m_vectorHashMapForEntity{ CreateJEMallocArray<VectorHashMap4IDUnsafe<uint32_t, ContentsEntity*>>(num_of_groups) }
			, m_info{ info }
		{}
		~Cluster()noexcept;
	public:
		void EnterEnqueue(ContentsEntity* const pEntity_)noexcept {
			PostClusterTask(&Cluster::Enter, pEntity_->GetObjectType(), pEntity_->GetObjectID(), static_cast<ContentsEntity* const>(pEntity_));
		}
		void LeaveAndDestroyEnqueue(const uint8_t group_type, const uint32_t obj_id)noexcept {
			PostClusterTask(&Cluster::LeaveAndDestroy, uint8_t{ group_type }, c_uint32{ obj_id });
		}
		void MigrationEnqueue(const ClusterInfo other_info, const ContentsEntity* const pEntity_)noexcept {
			PostClusterTask(&Cluster::Migration,rcast(other_info), pEntity_->GetObjectType(), pEntity_->GetObjectID());
		}
	public:
		void Broadcast(const S_ptr<SendBuffer>& pkt_)const noexcept;
	public:
		const auto GetAllEntites()const noexcept { return m_vectorHashMapForEntity; }
		const auto& GetSessions()const noexcept { return m_vectorHashMapForEntity.data()->GetItemListRef(); }
		std::span<VectorHashMap4IDUnsafe<uint32_t, ContentsEntity*>> GetEntitesExceptSession()const noexcept {
			const auto b = m_vectorHashMapForEntity.data();
			const auto e = b + m_vectorHashMapForEntity.size();
			return { b + 1,e };
		}
		const auto& GetEntities(const uint8_t group_type)const noexcept { return m_vectorHashMapForEntity[group_type].GetItemListRef(); }
		ClusterInfo GetClusterInfo()const noexcept { return m_info; }
	private:
		
		void Enter(const uint8_t group_type, const uint32_t obj_id, ContentsEntity* const pEntity_)noexcept;
		void LeaveAndDestroy(const uint8_t group_type, const uint32_t obj_id)noexcept;
		void Migration(const ClusterInfo info, const uint8_t group_type, const uint32_t obj_id) noexcept;
	protected:
		template <typename MemFunc, typename... Args>
		void PostClusterTask(const MemFunc memFunc, Args&&... args)noexcept
		{
			ClusterUpdateQueue::PushTask(xnew<ClusterUpdateTask>(
				m_info,
				memFunc,
				std::forward<Args>(args)...));
		}
	private:
		const ClusterInfo m_info;
		const std::span<VectorHashMap4IDUnsafe<uint32_t, ContentsEntity*>> m_vectorHashMapForEntity;
	};
}

