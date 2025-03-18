#include "pch.h"
#include "IocpObject.h"
#include "Queueabler.h"
#include "HP.h"
#include "Death.h"
#include "Cluster.h"
#include "QuestSystem.h"

void HP::PostDoDmg(const int dmg_, NagiocpX::S_ptr<NagiocpX::ContentsEntity> atkObject) noexcept
{
	EnqueueCompTaskPushOnly(&HP::DoDmg, dmg_, std::move(atkObject));
}

void HP::PostDoHeal(const int heal_) noexcept
{
	EnqueueCompTaskPushOnly(&HP::DoHeal, heal_);
}

void HP::DoDmg(const int dmg_, const NagiocpX::S_ptr<NagiocpX::ContentsEntity> atkObject) noexcept
{
	const auto owner = GetOwnerEntityRaw();
	if (0 >= m_hp)return;
	if (!owner->IsValid())return;
	m_hp -= dmg_;
	if (atkObject->GetSession())
	{
		atkObject->GetCurCluster()->Broadcast(Create_s2c_NOTIFY_HIT_DMG(owner->GetObjectID(), owner->GetComp<HP>()->GetCurHP() - dmg_));
	}
	else if (owner->GetSession())
	{
		owner->GetCurCluster()->Broadcast(Create_s2c_NOTIFY_HIT_DMG(owner->GetObjectID(), owner->GetComp<HP>()->GetCurHP() - dmg_));
	}
	if (0 < m_hp)return;
	if (const auto death = owner->GetComp<Death>())
	{
		death->ProcessDeath();
	}
	if (const auto q = atkObject->GetComp<QuestSystem>())
	{
		q->PostCheckQuestAchieve(owner->SharedFromThis());
	}
}

void HP::DoHeal(const int heal_) noexcept
{
	m_hp = std::min(m_hp + heal_, m_maxHP); // TODO 힐 후 해야 할 일 + 최대 상한치 검사
	// TODO: 힐패킷
}
