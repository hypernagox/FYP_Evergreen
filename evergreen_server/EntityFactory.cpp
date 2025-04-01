#include "pch.h"
#include "EntityFactory.h"
#include "TickTimer.h"
#include "ContentsBehaviorNode.h"
#include "Queueabler.h"
#include "NaviAgent_Common.h"
#include "PathFinder_Common.h"
#include "Navigator.h"
#include "Collider_Common.h"
#include "HP.h"
#include "Death.h"
#include "Regenerator.h"
#include "RangeMonState.h"
#include "DropItem.h"
#include "DropTable.h"

namespace NagiocpX
{
	S_ptr<ContentsEntity> EntityFactory::CreateMonster(const EntityBuilder& b) noexcept
	{
		const auto monster_entity = CreateContentsEntity(b.group_type, (MONSTER_TYPE_INFO)b.obj_type);

		const auto bt_timer = monster_entity->AddIocpComponent<TickTimerBT>(xnew<SelectorNode>(),300*300);

		const auto& bt_root = bt_timer->GetRootNode();

		monster_entity->AddComp<PositionComponent>();

		bt_timer->SetTickInterval(500);
		
		
		const auto s1 = bt_root->AddChild<SequenceNode>();

		s1->AddChild<RangeCheckNode>(30);
		const auto s2 = s1->AddChild<SequenceNode>();

		s2->AddChild<RangeCheckNode>(100);
		s2->AddChild<AttackNode>();

		s1->AddChild<ChaseNode>();

		bt_root->AddChild<PatrolNode>();

		const auto agent = monster_entity->AddComp<NaviAgent>();
		agent->SetPosComp(monster_entity->GetComp<PositionComponent>());
		agent->InitRandPos(NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0));

		monster_entity->AddComp<PathFinder>()->SetAgent(agent->GetAgentConcreate());
		//monster_entity->AddComp<Collider>()->SetBox(monster_entity->GetComp<PositionComponent>(), { 1,1,1 });
		monster_entity->AddComp<SphereCollider>()->SetSphere(monster_entity->GetComp<PositionComponent>(), 1);

		monster_entity->AddComp<HP>()->InitHP(GET_DATA(int,"Fox","hp")); // TODO 매직넘버
		monster_entity->AddComp<MonsterDeath>();

		monster_entity->SetDeleter<Regenerator>(5000, agent->GetPosComp()->pos);

		monster_entity->AddComp<DropTable>()->SetItemType("Fox");

		return monster_entity;
	}
	S_ptr<ContentsEntity> EntityFactory::CreateNPC(const EntityBuilder& b) noexcept
	{
		const auto entity = CreateContentsEntity(b.group_type, (MONSTER_TYPE_INFO)b.obj_type);
		// TODO: NPC에 필요한 부가정보
		entity->AddComp<PositionComponent>()->pos = { b.x, b.y, b.z };
		return entity;
	}

	S_ptr<ContentsEntity> EntityFactory::CreateRangeMonster(const EntityBuilder& b) noexcept
	{
		const auto monster_entity = CreateContentsEntity(b.group_type, (MONSTER_TYPE_INFO)b.obj_type);
		monster_entity->AddComp<PositionComponent>();
		const auto fsm = monster_entity->AddIocpComponent<TickTimerFSM>();
		fsm->SetTickInterval(500);

		fsm->AddState<RangeMonIdle>();
		fsm->AddState<RangeMonChase>();
		fsm->AddState<RangeMonAttack>();

		fsm->SetDefaultState(RANGE_MON_STATE::IDLE);

		const auto agent = monster_entity->AddComp<NaviAgent>();
		agent->SetPosComp(monster_entity->GetComp<PositionComponent>());
		agent->InitRandPos(NAVIGATION->GetNavMesh(NAVI_MESH_NUM::NUM_0));

		monster_entity->AddComp<PathFinder>()->SetAgent(agent->GetAgentConcreate());
		//monster_entity->AddComp<Collider>()->SetBox(monster_entity->GetComp<PositionComponent>(), { 1,1,1 });
		monster_entity->AddComp<SphereCollider>()->SetSphere(monster_entity->GetComp<PositionComponent>(), 1);

		monster_entity->AddComp<HP>()->InitHP(3);
		monster_entity->AddComp<MonsterDeath>();

		monster_entity->SetDeleter<Regenerator>(5000, agent->GetPosComp()->pos);

		monster_entity->AddComp<DropTable>()->SetItemType("Bear");

		return monster_entity;
	}
	S_ptr<ContentsEntity> EntityFactory::CreateDropItem(const EntityBuilder& b) noexcept
	{
		const auto& b_ = static_cast<const DropItemBuilder&>(b);
		const auto entity = CreateContentsEntity(b_.group_type, (ITEM_TYPE_INFO)b_.obj_type);
		entity->AddComp<PositionComponent>()->pos = { b_.x, b_.y, b_.z };
		const auto item = entity->AddComp<DropItem>();
		item->SetDropItemDetailInfo(b_.item_detail_type);
		item->SetItemStack(b_.item_stack_size);
		return entity;
	}
}