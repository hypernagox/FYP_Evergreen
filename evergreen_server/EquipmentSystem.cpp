#include "pch.h"
#include "EquipmentSystem.h"
#include "PositionComponent.h"
#include "NaviAgent_Common.h"
#include "ClusterPredicate.h"
#include "Cluster.h"
#include "Queueabler.h"
#include "ObjectIdentifier.h"
#include "DataRegistry.h"
#include "DBMgr.h"
#include "DBContentsPacket.hpp"
#include "ClientSession.h"

EquipmentSystem::EquipmentSystem() noexcept
{
	for (auto& equip : m_arrEquip)
	{
		equip = NagiocpX::xnew<Equipment>();
	}
}

EquipmentSystem::~EquipmentSystem() noexcept
{
	for (const auto equip : m_arrEquip)
	{
		if (!equip)continue;
		NagiocpX::xdelete<Equipment>(equip);
	}
}

int EquipmentSystem::ApplyAtk(
	ContentsEntity* const atk_obj,
	const int attacker_atk,
	const int victim_origin_hp,
	int& victim_hp, 
	ContentsEntity* const victim) noexcept
{
	int result_dmg = attacker_atk;
	for (const auto equip : m_arrEquip) {
		if (!equip)continue;
		result_dmg += equip->ApplyAtkEquipment(atk_obj, attacker_atk, victim_origin_hp, victim_hp, victim);
	}
	
	//if (const auto navi_agent = victim->GetComp<NaviAgent>())
	//{
	//	if (const auto e = m_arrEquip[0])
	//	{
	//		if (DATA_TABLE->GetWeaponIDInt("Bow") == e->id)
	//		{
	//			// TODO: 장비옵션으로 넉백하기
	//			const auto& atk_pos = atk_obj->GetComp<PositionComponent>()->pos;
	//			const auto& victim_pos = victim->GetComp<PositionComponent>()->pos;
	//			const auto dir = CommonMath::Normalized(victim_pos - atk_pos);
	//			ClusterPredicate c;
	//			victim->GetQueueabler()->EnqueueAsync(&NaviAgent::ForcedMovement, navi_agent, dir, 5.f);
	//
	//			victim->GetCurCluster()->Broadcast(c.ClusterPredicate::CreateMovePacket(victim));
	//		}
	//	}
	//}
	//std::cout << "Dmg: "<<result_dmg << std::endl;
	victim_hp -= result_dmg;
	return result_dmg;
}

bool EquipmentSystem::SwapEquipment(
	class ContentsEntity* const owner,
	const Nagox::Enum::EQUIPMENT_TYPE equip_type, 
	const uint32_t equip_id,
	const bool use_db) noexcept
{
	// TODO: 추후 변경 필요
	// TODO: 유효성 검사
	if (const auto e = m_arrEquip[equip_type])
	{
		e->id = equip_id;
		const auto equip_stat = DATA_TABLE->GetEquipStat(equip_id);
		e->atk = equip_stat.atk;
		//std::cout << "Atk: " << e->atk << std::endl;
		e->def = equip_stat.def;
		if (Nagox::Enum::EQUIPMENT_TYPE::EQUIPMENT_TYPE_ARMOR == equip_type)
		{
			e->def = equip_id;
		}
		if (use_db)
		{
			s2q_SWAP_EQUIPMENT pkt;
			if (Nagox::Enum::EQUIPMENT_TYPE_WEAPON == equip_type)
			{
				pkt.equip_type = 0;
			}
			else if (Nagox::Enum::EQUIPMENT_TYPE_ARMOR == equip_type)
			{
				pkt.equip_type = 1;
			}
			pkt.equip_id = equip_id;
			pkt.pkt_db_uid = owner->GetClientSession()->m_db_uid;
			RequestQueryServer(pkt);
			owner->GetComp<ObjectIdentifier>()->BroadcastNotifyEquipmentChange();
		}
		return true;
	}
	else
	{
		return false;
	}
}
