#include "NagiocpXPch.h"
#include "Field.h"
#include "Cluster.h"
#include "FieldMgr.h"
#include "ClusterInfoHelper.h"

namespace NagiocpX
{
	//void Field::EnterFieldNPC(const S_ptr<ContentsEntity>& pEntity_)noexcept {
	//	const auto entity_ptr = pEntity_.get();
	//	Mgr(FieldMgr)->RegisterNPC(const_cast<S_ptr<ContentsEntity>&>(pEntity_));
	//	if (pEntity_)return;
	//	EnterField(entity_ptr);
	//}

	void Field::EnterFieldWithXYNPC(const uint8_t start_x, const uint8_t start_y, const S_ptr<ContentsEntity>& pEntity_)noexcept {
		const auto entity_ptr = pEntity_.get();
		Mgr(FieldMgr)->RegisterNPC(const_cast<S_ptr<ContentsEntity>&>(pEntity_));
		if (pEntity_)return;
		EnterFieldWithXY(start_x, start_y, entity_ptr);
	}

	void Field::EnterFieldWithFloatXYNPC(const float start_x, const float start_y, const S_ptr<ContentsEntity>& pEntity_) noexcept
	{
		const auto x = (uint8_t)(start_x / (float)m_cluster_x_scale);
		const auto y = (uint8_t)(start_y / (float)m_cluster_y_scale);
		EnterFieldWithXYNPC(x, y, pEntity_);
	}

	//void Field::EnterField(ContentsEntity* const pEntity_)noexcept {
	//	const auto cluster = GetCluster(m_start_x, m_start_y);
	//	pEntity_->IncRefEnterCluster();
	//	pEntity_->SetClusterFieldInfoUnsafe(cluster->GetClusterFieldInfo());
	//	std::atomic_thread_fence(std::memory_order_release);
	//	cluster->EnterEnqueue(pEntity_);
	//}

	void Field::EnterFieldWithXY(const uint8_t start_x, const uint8_t start_y, ContentsEntity* const pEntity_)noexcept {
		const auto cluster = GetCluster(start_x, start_y);
		pEntity_->IncRefEnterCluster();
		pEntity_->SetClusterFieldInfoUnsafe(cluster->GetClusterFieldInfo());
		std::atomic_thread_fence(std::memory_order_release);
		cluster->EnterEnqueue(pEntity_);
	}

	void Field::EnterFieldWithFloatXY(const float start_x, const float start_y, ContentsEntity* const pEntity_) noexcept
	{
		const auto x = (uint8_t)(start_x / (float)m_cluster_x_scale);
		const auto y = (uint8_t)(start_y / (float)m_cluster_y_scale);
		EnterFieldWithXY(x, y, pEntity_);
	}

	void Field::DestroyFieldTLS() noexcept
	{

		const auto threadIdx = NagiocpX::GetCurThreadIdx();
		const auto row = (size_t)GetNumOfClusterRow();

		const std::span<XVector<NagiocpX::Cluster*>> clusters{
			tl_vecClusters[threadIdx],
			row
		};

		for (const auto cluster : clusters | std::views::join)
		{
			NagiocpX::xdelete<NagiocpX::Cluster>(cluster);
		}

		NagiocpX::DeleteJEMallocArray(clusters);
	}

	Field* const Field::GetFieldInternal(const uint8_t fieldID) noexcept
	{
		const auto& field_table = Mgr(FieldMgr)->GetFieldTable();
		const auto iter = field_table.find(fieldID);
		return field_table.end() != iter ? iter->second : nullptr;
	}
}