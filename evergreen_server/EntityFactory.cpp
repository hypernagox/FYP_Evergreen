#include "pch.h"
#include "EntityFactory.h"
#include "TickTimer.h"
#include "ContentsBehaviorNode.h"
#include "Queueabler.h"

namespace ServerCore
{
	S_ptr<ContentsEntity> EntityFactory::CreateMonster(const EntityBuilder& b) noexcept
	{
		const auto monster_entity = CreateContentsEntity(b.group_type, (MONSTER_TYPE_INFO)b.obj_type);

		const auto bt_timer = monster_entity->AddIocpComponent<TickTimerBT>(xnew<SelectorNode>(),30*30);

		const auto& bt_root = bt_timer->GetRootNode();

		monster_entity->AddComp<PositionComponent>();

		bt_timer->SetTickInterval(200);
		
		

		const auto s1 = bt_root->AddChild<SequenceNode>();

		s1->AddChild<RangeCheckNode>(30);
		const auto s2 = s1->AddChild<SequenceNode>();

		s2->AddChild<RangeCheckNode>(10);
		s2->AddChild<AttackNode>();

		s1->AddChild<ChaseNode>();

		bt_root->AddChild<PatrolNode>();

		return monster_entity;
	}
}