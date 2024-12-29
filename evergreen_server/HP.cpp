#include "pch.h"
#include "HP.h"
#include "Queueabler.h"
#include "IocpObject.h"
#include "ComponentSystem.h"
#include "Death.h"
#include "QuestSystem.h"

void HP::PostDoDmg(const int dmg_, ServerCore::S_ptr<ServerCore::ContentsEntity> atkObject) noexcept
{
	if (m_bIsRebirth)return;
	GetOwnerEntityRaw()->GetQueueabler()->EnqueueAsyncPushOnly(&HP::DoDmg, this, dmg_, std::move(atkObject));
}

void HP::PostDoHeal(const int heal_) noexcept
{
	if (m_bIsRebirth)return;
	GetOwnerEntityRaw()->GetQueueabler()->EnqueueAsyncPushOnly(&HP::DoHeal, this, heal_);
}

void HP::DoDmg(const int dmg_, const ServerCore::S_ptr<ServerCore::ContentsEntity> atkObject) noexcept
{
	// TODO: 퀘스트 관련은 atkObject의 큐로 가야함 이거 문제가있다.
	const auto owner = GetOwnerEntityRaw();
	if (0 >= m_hp)return;
	if (m_bIsRebirth)return;
	if (!owner->IsValid())return;
	m_hp -= dmg_;
	if (0 < m_hp)return; // TODO 체력 표시 패킷 등
	if (const auto death = owner->GetComp<Death>())
		death->ProcessDeath();
	if (const auto q = atkObject->GetComp<QuestSystem>())
	{
		// TODO 퀘스트 키값 정하기
		q->PostCheckQuestAchieve(owner->SharedFromThis());
	}
	m_bIsRebirth = true;
}

void HP::DoHeal(const int heal_) noexcept
{
	m_hp += heal_; // TODO 힐 후 해야 할 일 + 최대 상한치 검사
}
