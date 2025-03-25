#include "pch.h"
#include "DropItem.h"
#include "NetworkMgr.h"
#include "ServerObject.h"

void DropItem::Update() noexcept
{
	if (INSTANCE(udsdx::Input)->GetKeyDown(Keyboard::F))
	{
		const auto hero_pos = m_mainHero->GetTransform()->GetLocalPosition();
		// TODO: �����ѹ� + �Ÿ������� �ص� �ɷ���?
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
