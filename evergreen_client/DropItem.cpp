#include "pch.h"
#include "DropItem.h"
#include "NetworkMgr.h"
#include "ServerObject.h"

void DropItem::Update() noexcept
{
	if (INSTANCE(udsdx::Input)->GetKeyDown(Keyboard::F))
	{
		const auto hero_pos = m_mainHero->GetTransform()->GetLocalPosition();
		// TODO: 매직넘버 + 거리만으로 해도 될려나?
		if (CommonMath::IsInDistanceDX(hero_pos, m_itemPos, 5.f))
		{
			Send(Create_c2s_ACQUIRE_ITEM(m_owner->GetObjID()));
		}
	}
}

void DropItem::SetItemPos(const Vector3& pos)
{
	m_itemPos = pos;
	GetRootObject()->GetTransform()->SetLocalPosition(pos);
}
