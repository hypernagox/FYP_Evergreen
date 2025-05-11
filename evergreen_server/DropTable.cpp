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
#include "ClusterInfoHelper.h"

void DropTable::SetItemType(const std::string_view mon_name)
{
	m_itemType = DATA_TABLE->GetItemID(GET_DATA(std::string, mon_name, "DropItem"));
}

void DropTable::TryCreateItem() const noexcept
{
	// TODO: 확률과 갯수
	const auto owner = GetOwnerEntityRaw();
	DropItemBuilder b;
	const auto pos = owner->GetComp<PositionComponent>()->pos;
	b.group_type = Nagox::Enum::GROUP_TYPE_DROP_ITEM;
	b.obj_type = m_itemType;
	b.x = pos.x + m_drop_offset.x;
	b.y = pos.y + m_drop_offset.y;
	b.z = pos.z + m_drop_offset.z;
	b.item_detail_type = m_itemType;
	b.item_stack_size = 1;
	b.has_life_span = m_bHasLifeSpan;
	auto item = EntityFactory::CreateDropItem(b);
	const auto temp_ptr = item.get();
	ClusterPredicate p;
	//auto pkt = p.CreateAddPacket(temp_ptr);
	//owner->GetCurField()->EnterFieldWithFloatXYNPC(
	//	PositionComponent::GetXZWithOffsetGlobal(temp_ptr),
	//	std::move(item)
	//);
	//owner->GetComp<NagiocpX::ClusterInfoHelper>()->BroadcastCluster(
	//	std::move(pkt)
	//);
	//
	// 필드에 먼저 넣으면, 필드 클러스터 컨테이너에 삽입과 무브패킷 때문에 Add 발생
	// 근데 바로 누가먹어서 바로 Remove
	// 이후 브로드캐스트하니 Add가 가고 이후 아무일X

	auto pkt = p.CreateAddPacket(temp_ptr);

	//owner->GetComp<NagiocpX::ClusterInfoHelper>()->BroadcastCluster(
	//	std::move(pkt)
	//);

	owner->GetCurField()->EnterFieldWithFloatXYNPC(
		PositionComponent::GetXZWithOffsetGlobal(temp_ptr),
		std::move(item)
	);
	
}
