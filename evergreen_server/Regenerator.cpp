#include "pch.h"
#include "Regenerator.h"
#include "TaskTimerMgr.h"
#include "Field.h"
#include "PositionComponent.h"
#include "NaviAgent_Common.h"
#include "NavigationMesh.h"
#include "Navigator.h"
#include "Queueabler.h"

void Regenerator::ProcessDestroy(S_ptr<ContentsEntity> entity) noexcept
{
	if (!IsFieldRunning(entity))return;
	entity->GetQueueabler()->EnqueueAsyncTimer(m_duration, &Regenerator::RegenerateNPC, this, std::move(entity));
}

void Regenerator::RegenerateNPC(S_ptr<ContentsEntity> entity) noexcept
{
	if (!IsFieldRunning(entity))return;
	entity->GetComp<NaviAgent>()->SetPos(m_summon_pos);
	entity->SetActive();
	entity->GetClusterFieldInfo().curFieldPtr->EnterFieldNPC(std::move(entity));
}

bool Regenerator::IsFieldRunning(S_ptr<ContentsEntity>& entity) noexcept
{
	if (m_targetField)
	{
		if (!m_targetField->IsRunning())
		{
			m_targetField.reset();
			entity->ResetDeleter();
			return false;
		}
	}
	return true;
}
