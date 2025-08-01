#include "pch.h"
#include "Interaction.h"
#include "DropTable.h"
#include "ClusterInfoHelper.h"
#include "TaskTimerMgr.h"
#include "PositionComponent.h"
#include "NavigationMesh.h"
#include "Navigator.h"
#include "BossBehaviorNode.h"
#include "TickTimer.h"
#include "Cluster.h"

bool HarvestInteraction::DoInteraction(ContentsEntity* const pEntity_) noexcept
{
	if (true == m_isActive.exchange(false))
	{
		const auto owner = GetOwnerEntity();
		owner->SetDetailType(HARVEST_STATE::UNAVAILABLE);
		owner->GetComp<NagiocpX::ClusterInfoHelper>()->BroadcastAllCluster(Create_s2c_CHANGE_HARVEST_STATE(owner->GetObjectID(), false, m_interaction_type));
		owner->GetComp<DropTable>()->TryCreateItem();


		Mgr(TaskTimerMgr)->ReserveAsyncTask(g_harvest_cool_down, [owner = GetOwnerEntity(), this]() {
			owner->SetDetailType(HARVEST_STATE::AVAILABLE);
			m_isActive.store(true);
			owner->GetComp<NagiocpX::ClusterInfoHelper>()->BroadcastAllCluster(Create_s2c_CHANGE_HARVEST_STATE(owner->GetObjectID(), true, m_interaction_type));
			});

		return true;
	}
	return false;
}

bool ClearTreeInteraction::DoInteraction(ContentsEntity* const pEntity_) noexcept
{
	const auto pos = pEntity_->GetComp<PositionComponent>()->pos;
	const auto cur_time = GetTickCount64();
	NagiocpX::LockGuard lock{ m_clear_tree_mutex };
	if (0 >= m_num_of_reward_count)return false;
	if (500 > (cur_time - m_last_get_time))return false;
	m_last_get_time = cur_time;
	const auto owner = GetOwnerEntity();
	--m_num_of_reward_count;
	const auto item_table = owner->GetComp<DropTable>();
	auto temp = pos;
	NAVIGATION->GetNavMesh(m_nav_mesh_type)->findRandomPointAroundCircle(
		&pos.x,
		0.125f * .1f,
		&temp.x
	);
	temp.y -= 1.f;
	item_table->GetOwnerEntityRaw()->GetComp<PositionComponent>()->pos = temp;
	item_table->m_bHasLifeSpan = false;
	item_table->TryCreateItem();
	if (0 == m_num_of_reward_count)
	{
		owner->SetDetailType(HARVEST_STATE::UNAVAILABLE);
		owner->GetComp<NagiocpX::ClusterInfoHelper>()->BroadcastAllCluster(Create_s2c_CHANGE_HARVEST_STATE(0, false, m_interaction_type));
	}
	return true;
}

bool Catapult::DoInteraction(ContentsEntity* const pEntity_) noexcept
{
	if (!m_boss_ptr)return false;
	if (!m_meteor_node)return false;
	if (!m_meteor_node->m_now_meteor)return false;
	if (true == m_meteor_node->m_hit_catapult.exchange(true)) return false;

	const auto target_pos = Vector3(-47.336597F, 18.003374F, -244.53511F);
	const auto dir = CommonMath::Normalized(target_pos - m_boss_ptr->GetComp<PositionComponent>()->pos);
	m_boss_ptr->GetComp<PositionComponent>()->pos = target_pos;
	m_boss_ptr->GetComp<PositionComponent>()->body_angle = atan2f(dir.x, dir.z) * 180.f / 3.141592f;
	m_boss_ptr->GetCurCluster()->Broadcast(Create_s2c_SHOOT_CATAPULT(ToFlatVec(target_pos)));
	m_boss_ptr->GetCurCluster()->Broadcast(Create_s2c_BOSS_FLY(ToFlatVec(target_pos), Nagox::Enum::BOSS_FLY_TYPE_BOSS_FLY_TYPE_2));
	return true;
}
