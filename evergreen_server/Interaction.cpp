#include "pch.h"
#include "Interaction.h"
#include "DropTable.h"
#include "ClusterInfoHelper.h"
#include "TaskTimerMgr.h"

bool HarvestInteraction::DoInteraction(ContentsEntity* const pEntity_) noexcept
{
	if (true == m_isActive.exchange(false))
	{
		const auto owner = GetOwnerEntity();
		owner->SetDetailType(HARVEST_STATE::UNAVAILABLE);
		owner->GetComp<NagiocpX::ClusterInfoHelper>()->BroadcastAllCluster(Create_s2c_CHANGE_HARVEST_STATE(owner->GetObjectID(), false));
		owner->GetComp<DropTable>()->TryCreateItem();


		Mgr(TaskTimerMgr)->ReserveAsyncTask(g_harvest_cool_down, [this]() {
			const auto owner = GetOwnerEntity();
			owner->SetDetailType(HARVEST_STATE::AVAILABLE);
			m_isActive.store(true);
			owner->GetComp<NagiocpX::ClusterInfoHelper>()->BroadcastAllCluster(Create_s2c_CHANGE_HARVEST_STATE(owner->GetObjectID(), true));
			});

		return true;
	}
	return false;
}
