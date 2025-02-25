#include "pch.h"
#include "HP.h"
#include "Queueabler.h"
#include "IocpObject.h"
#include "ComponentSystem.h"
#include "Death.h"
#include "QuestSystem.h"
#include "Cluster.h"

void HP::PostDoDmg(const int dmg_, NagiocpX::S_ptr<NagiocpX::ContentsEntity> atkObject) noexcept
{
	//if (m_bIsRebirth)return;
	EnqueueCompTaskPushOnly(&HP::DoDmg, dmg_, std::move(atkObject));
}

void HP::PostDoHeal(const int heal_) noexcept
{
	//if (m_bIsRebirth)return;
	EnqueueCompTaskPushOnly(&HP::DoHeal, heal_);
}

void HP::DoDmg(const int dmg_, const NagiocpX::S_ptr<NagiocpX::ContentsEntity> atkObject) noexcept
{
	const auto owner = GetOwnerEntityRaw();
	if (0 >= m_hp)return;
	if (m_bIsRebirth)return;
	if (!owner->IsValid())return;
	m_hp -= dmg_;
	// TODO: 체력까는건 여기서로 바꾸자
	// 현재 몹 AI에 유저때리는패킷이 있는듯
	// dmg수치 나중에 조절 매직넘버임
	if (atkObject->GetSession())
		atkObject->GetCurCluster()->Broadcast(Create_s2c_MONSTER_HIT(owner->GetObjectID(), 1));
	if (0 < m_hp)return; // TODO 체력 표시 패킷 등
	if (const auto death = owner->GetComp<Death>())
		death->ProcessDeath();
	
	if (const auto q = atkObject->GetComp<QuestSystem>())
	{
		// TODO 퀘스트 키값 정하기
		q->PostCheckQuestAchieve(owner->SharedFromThis());
	}
	//m_bIsRebirth = true;
}

void HP::DoHeal(const int heal_) noexcept
{
	m_hp += heal_; // TODO 힐 후 해야 할 일 + 최대 상한치 검사
}
