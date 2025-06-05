#include "pch.h"
#include "ObjectIdentifier.h"
#include "Inventory.h"
#include "EquipmentSystem.h"
#include "ClusterPredicate.h"
#include "Cluster.h"
#include "ClusterInfoHelper.h"

S_ptr<SendBuffer> ObjectIdentifier::CreateNotifyDetailPacket() const noexcept
{
	const auto owner = GetOwnerEntityRaw();
	const auto inv = owner->GetComp<Inventory>();
	const auto equip_sys = inv->GetEquipmentSystem();
	
	const auto weapon_ptr = equip_sys->GetEquipment(Nagox::Enum::EQUIPMENT_TYPE::EQUIPMENT_TYPE_WEAPON);
	std::cout << weapon_ptr->id << std::endl;
	auto pkt = Create_s2c_NOTIFY_USER_DETAIL_INFO(
		owner->GetObjectID(),
		weapon_ptr->id,
		0
	);

	return pkt;
}

void ObjectIdentifier::BroadcastNotifyEquipmentChange() const noexcept
{
	const auto pkt = CreateNotifyDetailPacket();
	const auto owner = GetOwnerEntityRaw();
	owner->GetComp<NagiocpX::ClusterInfoHelper>()->BroadcastCluster(pkt);
}
