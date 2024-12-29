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
	// TODO: ����Ʈ ������ atkObject�� ť�� ������ �̰� �������ִ�.
	const auto owner = GetOwnerEntityRaw();
	if (0 >= m_hp)return;
	if (m_bIsRebirth)return;
	if (!owner->IsValid())return;
	m_hp -= dmg_;
	if (0 < m_hp)return; // TODO ü�� ǥ�� ��Ŷ ��
	if (const auto death = owner->GetComp<Death>())
		death->ProcessDeath();
	if (const auto q = atkObject->GetComp<QuestSystem>())
	{
		// TODO ����Ʈ Ű�� ���ϱ�
		q->PostCheckQuestAchieve(owner->SharedFromThis());
	}
	m_bIsRebirth = true;
}

void HP::DoHeal(const int heal_) noexcept
{
	m_hp += heal_; // TODO �� �� �ؾ� �� �� + �ִ� ����ġ �˻�
}
