#include "pch.h"
#include "ObjectIdentifier.h"
#include "Inventory.h"
#include "EquipmentSystem.h"
#include "ClusterPredicate.h"
#include "Cluster.h"
#include "ClusterInfoHelper.h"
#include "ClientSession.h"

S_ptr<SendBuffer> ObjectIdentifier::CreateNotifyDetailPacket() const noexcept
{
	const auto owner = GetOwnerEntityRaw();
	const auto inv = owner->GetComp<Inventory>();
	const auto equip_sys = inv->GetEquipmentSystem();
	
	const auto weapon_ptr = equip_sys->GetEquipment(Nagox::Enum::EQUIPMENT_TYPE::EQUIPMENT_TYPE_WEAPON);
	const auto armor_ptr = equip_sys->GetEquipment(Nagox::Enum::EQUIPMENT_TYPE::EQUIPMENT_TYPE_ARMOR);
	//std::cout << weapon_ptr->id << std::endl;
	auto pkt = Create_s2c_NOTIFY_USER_DETAIL_INFO(
		owner->GetObjectID(),
		owner->GetClientSession()->m_userName,
		weapon_ptr->id,
		armor_ptr->id
	);

	return pkt;
}

void ObjectIdentifier::BroadcastNotifyEquipmentChange() const noexcept
{
	const auto pkt = CreateNotifyDetailPacket();
	const auto owner = GetOwnerEntityRaw();
	owner->GetComp<NagiocpX::ClusterInfoHelper>()->BroadcastCluster(pkt);
}
