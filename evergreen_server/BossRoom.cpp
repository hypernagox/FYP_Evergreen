#include "pch.h"
#include "BossRoom.h"
#include "TickTimer.h"
#include "BossBehaviorNode.h"
#include "Navigator.h"
#include "PathFinder_Common.h"
#include "HP.h"
#include "Collider_Common.h"
#include "Death.h"
#include "Interaction.h"

using namespace NagiocpX;

void BossRoom::InitQuestField() noexcept
{
	m_boss = CreateBoss();
	{
		auto m = m_boss;
		const auto pos = Vector3(-47.336597F, 18.003374F, -244.53511F);
		EnterFieldWithFloatXYNPC(pos.x + 512.f, pos.z + 512.f, m_boss);
	}
	m_nav_mesh_type = NAVI_MESH_TYPE::BOSS_ROOM;
	SetClearTreePos(Vector3(-47.336597F, 18.003374F, -244.53511F));
	SetQuestBeginPos(Vector3(-19.447096F, 22.77558F, -284.62582F));
	Broadcast2PartyMembers(Create_s2c_BOSS_ROOM_ENTER());
	++m_mon_count;
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
		boss_hp->InitHP(GET_DATA(int, "Monster", "Boss", "hp"));
		const auto boss_col = boss_entity->AddComp<OBBCollider>();
		boss_col->SetOBB(boss_entity->GetComp<PositionComponent>(), Vector3{ 3,3,6 });
		boss_col->m_collider.m_extent = Vector3{ 3,3,6 };
		boss_col->m_collider.m_offSet = Vector3{ 0,1.25f,0 };
		const auto boss_death = boss_entity->AddComp<MonsterDeath>();
	}
	const auto bt_timer = boss_entity->AddIocpComponent<TickTimerBT>(xnew<BossStorageNode>());

	const auto& bt_root = bt_timer->GetRootNode();

	bt_timer->SetTickInterval(1);
	bt_timer->SetBTRevaluateInterval(UINT32_MAX);


	// 페이즈 전환 체크
	{

		const auto phase_node = bt_root->AddChild<SequenceNode>();
		phase_node->AddChild<PhaseCheckNode>();
	}



	// 근거리공격
	{

		const auto melee_node = bt_root->AddChild<SequenceNode>();
		const auto choice_atk = melee_node->AddChild<SelectPattern>();
		choice_atk->m_count = 2;
		choice_atk->m_probability = .6f;
		choice_atk->m_origin_prob = .6f;
		const auto melee_atk_node = melee_node->AddChild<SequenceNode>();
		melee_atk_node->AddChild<SelectTarget>();
		melee_atk_node->AddChild<MoveToTarget>()->m_dist;
		melee_atk_node->AddChild<MeleeAtack>();
	}


	// 브레스공격
	{

		const auto melee_node = bt_root->AddChild<SequenceNode>();
		const auto choice_atk = melee_node->AddChild<SelectPattern>();
		choice_atk->m_count = 1;
		choice_atk->m_probability = .6f;
		choice_atk->m_origin_prob = .6f;
		choice_atk->m_bIsBreath = true;
		const auto melee_atk_node = melee_node->AddChild<SequenceNode>();
		melee_atk_node->AddChild<SelectTarget>();
		melee_atk_node->AddChild<MoveToTarget>()->m_dist;
		melee_atk_node->AddChild<FireBreath>();
	}

	// 원거리 파이어볼
	//{
	//
	//	const auto fire_node = bt_root->AddChild<SequenceNode>();
	//	const auto choice_atk = fire_node->AddChild<SelectPattern>();
	//	choice_atk->m_probability = .6f;
	//	choice_atk->m_origin_prob = .6f;
	//	const auto fire_ball_node = fire_node->AddChild<SequenceNode>();
	//	fire_ball_node->AddChild<SelectJumpPoint>();
	//	fire_ball_node->AddChild<ShootFireBall>();
	//	fire_ball_node->AddChild<ResetPos>();
	//}

	// 중앙 메테오
	{
		 
		const auto meteor_node = bt_root->AddChild<SequenceNode>();
		//const auto choice_atk = meteor_node->AddChild<SelectPattern>();
		//choice_atk->m_probability = .6f;
		//choice_atk->m_origin_prob = .6f;
		meteor_node->AddChild<SetMeteorPos>();
		const auto fire_meteor_node = meteor_node->AddChild<FireMeteor>();
		meteor_node->AddChild<ResetPos>();
	
	
		auto catapult_entity = NagiocpX::CreateContentsEntity(Nagox::Enum::GROUP_TYPE_NPC, Nagox::Enum::NPC_TYPE_CATAPULT);
		const auto catapult = catapult_entity->AddComp<Catapult>();
		const auto pos_comp = catapult_entity->AddComp<PositionComponent>();
		pos_comp->pos = Vector3(-59.153553F, 30.0F, -287.9226F);
		const auto catapult_pos = pos_comp->pos;
		catapult->m_boss_ptr = boss_entity;
		catapult->m_meteor_node = fire_meteor_node;
		
		EnterFieldWithFloatXYNPC(catapult_pos.x + 512.f, catapult_pos.z + 512.f, catapult_entity);
	}
	return boss_entity;
}
