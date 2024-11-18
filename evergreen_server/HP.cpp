#include "pch.h"
#include "HP.h"
#include "Queueabler.h"
#include "IocpObject.h"
#include "ComponentSystem.h"
#include "Death.h"

void HP::PostDoDmg(const int dmg_) noexcept
{
	if (m_bIsRebirth)return;
	GetOwnerEntityRaw()->GetQueueabler()->EnqueueAsyncPushOnly(&HP::DoDmg, this, dmg_);
}

void HP::PostDoHeal(const int heal_) noexcept
{
	if (m_bIsRebirth)return;
	GetOwnerEntityRaw()->GetQueueabler()->EnqueueAsyncPushOnly(&HP::DoHeal, this, heal_);
}

void HP::DoDmg(const int dmg_) noexcept
{
	const auto owner = GetOwnerEntityRaw();
	if (0 >= m_hp)return;
	if (m_bIsRebirth)return;
	if (!owner->IsValid())return;
	m_hp -= dmg_;
	if (0 < m_hp)return; // TODO ü�� ǥ�� ��Ŷ ��
	if (const auto death = owner->GetComp<Death>())
		death->ProcessDeath();
	m_bIsRebirth = true;
}

void HP::DoHeal(const int heal_) noexcept
{
	m_hp += heal_; // TODO �� �� �ؾ� �� �� + �ִ� ����ġ �˻�
}
