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
	public:
		Cluster(const uint8_t num_of_groups, const ClusterInfo info)noexcept
			: m_vectorHashMapForEntity{ CreateJEMallocArray<VectorHashMap4IDUnsafe<uint32_t, ContentsEntity*>>(num_of_groups) }
			, m_info{ info }
		{}
	public:
		inline void EnterEnqueue(ContentsEntity* const pEntity_)noexcept {
			PostClusterTask(&Cluster::Enter, pEntity_->GetObjectType(), pEntity_->GetObjectID(), static_cast<ContentsEntity* const>(pEntity_));
		}
		inline void LeaveAndDestroyEnqueue(const uint8_t group_type, const uint32_t obj_id)noexcept {
			PostClusterTask(&Cluster::LeaveAndDestroy, uint8_t{ group_type }, c_uint32{ obj_id });
		}
		inline void MigrationEnqueue(const ClusterInfo info, const ContentsEntity* const pEntity_)noexcept {
			PostClusterTask(&Cluster::Migration,rcast(info), pEntity_->GetObjectType(), pEntity_->GetObjectID());
		}
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
		// 내가 소속한 월드는 어디인가
		// 내 섹터넘버는 무엇인가?
		const ClusterInfo m_info;
		const std::span<VectorHashMap4IDUnsafe<uint32_t, ContentsEntity*>> m_vectorHashMapForEntity;
	};
}

