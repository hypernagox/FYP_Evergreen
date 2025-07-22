#include "pch.h"
#include "IocpObject.h"
#include "Queueabler.h"
#include "HP.h"
#include "Death.h"
#include "Cluster.h"
#include "QuestSystem.h"
#include "StatusSystem.h"

void HP::PostDoDmg(const int dmg_, NagiocpX::S_ptr<NagiocpX::ContentsEntity> atkObject, const int hit_count) noexcept
{
	EnqueueCompTask(&HP::DoDmg, dmg_, std::move(atkObject), int{ hit_count });
}

void HP::PostDoHeal(const int heal_) noexcept
{
	EnqueueCompTask(&HP::DoHeal, heal_);
}

void HP::DoDmg(const int dmg_, const NagiocpX::S_ptr<NagiocpX::ContentsEntity> atkObject, const int hit_count) noexcept
{
	const auto owner = GetOwnerEntityRaw();
	if (0 >= m_hp)return;
	if (!owner->IsValid())return;

	{
		// TOOD: 공격시 로직
		//m_hp -= dmg_;
		if (atkObject->GetSession())
		{
			const int origin_hp = m_hp;
			
			const int result_dmg = atkObject->GetComp<StatusSystem>()->ApplyAtk(
				origin_hp,
				m_hp,
				owner
			);
			//std::cout << "데미지 :" << result_dmg << "!!\n";
			atkObject->GetCurCluster()->Broadcast(Create_s2c_NOTIFY_HIT_DMG(owner->GetObjectID(), origin_hp - result_dmg, hit_count));
		}
		else if (owner->GetSession())
		{
			const auto cur_dmg = std::max(dmg_ - owner->GetComp<StatusSystem>()->GetDEF(), 0);
			owner->GetCurCluster()->Broadcast(Create_s2c_NOTIFY_HIT_DMG(owner->GetObjectID(), owner->GetComp<HP>()->GetCurHP() - cur_dmg, hit_count));
			m_hp -= cur_dmg;
		}
		else
		{
			m_hp -= dmg_;
		}
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
	const auto owner = GetOwnerEntityRaw();
	if (!owner->IsValid())return;
	m_hp = std::min(m_hp + heal_, m_maxHP); // TODO 힐 후 해야 할 일 + 최대 상한치 검사
}
