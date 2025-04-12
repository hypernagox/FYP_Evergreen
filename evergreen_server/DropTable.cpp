#include "pch.h"
#include "DropTable.h"
#include "DataRegistry.h"
#include "EntityFactory.h"
#include "Cluster.h"
#include "PositionComponent.h"
#include "ClusterPredicate.h"
#include "DropItem.h"
#include "Field.h"
#include "FieldMgr.h"

void DropTable::SetItemType(const std::string_view mon_name)
{
	m_itemType = DATA_TABLE->GetItemID(GET_DATA(std::string, mon_name, "DropItem"));
}

void DropTable::TryCreateItem() const noexcept
{
	// TODO: È®·ü°ú °¹¼ö
	const auto owner = GetOwnerEntityRaw();
	DropItemBuilder b;
	const auto pos = owner->GetComp<PositionComponent>()->pos;
	b.group_type = Nagox::Enum::GROUP_TYPE_DROP_ITEM;
	b.obj_type = m_itemType;
	b.x = pos.x;
	b.y = pos.y;
	b.z = pos.z;
	b.item_detail_type = m_itemType;
	b.item_stack_size = 1;
	auto item = EntityFactory::CreateDropItem(b);
	const auto temp_ptr = item.get();
	ClusterPredicate p;
	Mgr(FieldMgr)->GetField(0)->EnterFieldNPC(
		std::move(item)
	);
	owner->GetCurCluster()->Broadcast(p.CreateAddPacket(temp_ptr));
}
