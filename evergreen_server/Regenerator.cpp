#include "pch.h"
#include "Regenerator.h"
#include "TaskTimerMgr.h"
#include "Field.h"
#include "PositionComponent.h"
#include "NaviAgent_Common.h"
#include "NavigationMesh.h"
#include "Navigator.h"


void Regenerator::ProcessDestroy(S_ptr<ContentsEntity> entity) noexcept
{
	ServerCore::PrintLogEndl("Start Destroy Routine");
	Mgr(TaskTimerMgr)->ReserveAsyncTask(m_duration, &Regenerator::RegenerateNPC, this, std::move(entity));
}

void Regenerator::RegenerateNPC(S_ptr<ContentsEntity> entity) noexcept
{
	entity->GetComp<NaviAgent>()->SetPos(m_summon_pos);
	entity->SetActive();
	Mgr(FieldMgr)->GetField(0)->EnterFieldNPC(std::move(entity));
}
