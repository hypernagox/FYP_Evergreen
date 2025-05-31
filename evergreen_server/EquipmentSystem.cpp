#include "pch.h"
#include "EquipmentSystem.h"
#include "PositionComponent.h"
#include "NaviAgent_Common.h"
#include "ClusterPredicate.h"
#include "Cluster.h"
#include "Queueabler.h"

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
	
	if (const auto navi_agent = victim->GetComp<NaviAgent>())
	{
		// TODO: 장비옵션으로 넉백하기
		const auto& atk_pos = atk_obj->GetComp<PositionComponent>()->pos;
		const auto& victim_pos = victim->GetComp<PositionComponent>()->pos;
		const auto dir = CommonMath::Normalized(victim_pos - atk_pos);
		ClusterPredicate c;
		victim->GetQueueabler()->EnqueueAsync(&NaviAgent::ForcedMovement, navi_agent, dir, 5.f);
		
		victim->GetCurCluster()->Broadcast(c.ClusterPredicate::CreateMovePacket(victim));
	}
	victim_hp -= result_dmg;
	return result_dmg;
}
