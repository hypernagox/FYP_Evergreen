#include "pch.h"
#include "BossRoom.h"
#include "TickTimer.h"
#include "BossBehaviorNode.h"
#include "Navigator.h"
#include "PathFinder_Common.h"
#include "HP.h"
#include "Collider_Common.h"
#include "Death.h"

using namespace NagiocpX;

void BossRoom::InitQuestField() noexcept
{
	m_boss = CreateBoss();
	{
		auto m = m_boss;
		const auto pos = Vector3(-47.336597F, 18.003374F, -244.53511F);
		EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m_boss);
	}
	SetQuestBeginPos(Vector3(-19.447096F, 22.77558F, -284.62582F));
	Broadcast2PartyMembers(Create_s2c_BOSS_ROOM_ENTER());
}

S_ptr<ContentsEntity> BossRoom::CreateBoss() noexcept
{

	auto boss_entity = NagiocpX::CreateContentsEntity(Nagox::Enum::GROUP_TYPE_MONSTER, Nagox::Enum::MONSTER_TYPE_BOSS);

	// 보스 초기위치 등 초기세팅
	{
		boss_entity->AddComp<PositionComponent>();
		const auto agent = boss_entity->AddComp<NaviAgent>();
		agent->SetPosComp(boss_entity->GetComp<PositionComponent>());
		const auto pos = Vector3(-47.336597F, 18.003374F, -244.53511F);
		boss_entity->GetComp<PositionComponent>()->pos = pos;
		
		boss_entity->GetComp<NaviAgent>()->Init(pos, NAVI_MESH_TYPE::BOSS_ROOM);
		boss_entity->AddComp<PathFinder>()->SetAgent(agent->GetAgentConcreate());
	}
	// 충돌체 및 HP 설정
	{
		const auto boss_hp = boss_entity->AddComp<HP>();
		boss_hp->InitHP(50);
		const auto boss_col = boss_entity->AddComp<OBBCollider>();
		boss_col->SetOBB(boss_entity->GetComp<PositionComponent>(), Vector3{ 3,3,6 });
		boss_col->m_collider.m_extent = Vector3{ 3,3,6 };
		boss_col->m_collider.m_offSet = Vector3{ 0,1.25f,0 };
		const auto boss_death = boss_entity->AddComp<MonsterDeath>();
	}
	const auto bt_timer = boss_entity->AddIocpComponent<TickTimerBT>(xnew<SelectorNode>());

	const auto& bt_root = bt_timer->GetRootNode();

	bt_timer->SetTickInterval(1);
	bt_timer->SetBTRevaluateInterval(UINT16_MAX);
	// 근거리공격
	{
		
		const auto melee_node = bt_root->AddChild<SequenceNode>();
		const auto choice_atk = melee_node->AddChild<SelectPattern>();
		choice_atk->m_count = 2;
		choice_atk->m_probability = .6f;
		choice_atk->m_origin_prob = .6f;
		const auto melee_atk_node = melee_node->AddChild<SequenceNode>();
		melee_atk_node->AddChild<SelectTarget>();
		melee_atk_node->AddChild<MoveToTarget>();
		melee_atk_node->AddChild<MeleeAtack>();
	}

	// 원거리 파이어볼
	{

		const auto fire_node = bt_root->AddChild<SequenceNode>();
		const auto choice_atk = fire_node->AddChild<SelectPattern>();
		choice_atk->m_probability = .6f;
		choice_atk->m_origin_prob = .6f;
		const auto fire_ball_node = fire_node->AddChild<SequenceNode>();
		fire_node->AddChild<SelectJumpPoint>();
		fire_node->AddChild<ShootFireBall>();
		fire_node->AddChild<ResetPos>();
	}

	// 중앙 메테오
	{

		const auto meteor_node = bt_root->AddChild<SequenceNode>();
		//const auto choice_atk = meteor_node->AddChild<SelectPattern>();
		//choice_atk->m_probability = .6f;
		//choice_atk->m_origin_prob = .6f;
		meteor_node->AddChild<SetMeteorPos>();
		meteor_node->AddChild<FireMeteor>();
		meteor_node->AddChild<ResetPos>();
	}
	return boss_entity;
}
