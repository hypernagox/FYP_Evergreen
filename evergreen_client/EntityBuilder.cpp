#include "pch.h"
#include "EntityBuilder.h"
#include "ServerObject.h"
#include "EntityMovement.h"
#include "PlayerRenderer.h"
#include "Monster.h"
#include "MoveInterpolator.h"
#include "DropItem.h"
#include "DropItemRenderer.h"
#include "../common/json.hpp"
#include "MonsterRenderer.h"

// string 등 무브시맨틱이 유효한 데이터라면 무브시맨틱을 적극 고려하자
extern std::shared_ptr<SceneObject> g_heroObj;

std::shared_ptr<udsdx::SceneObject> EntityBuilderBase::Create_Warrior(EntityBuilderBase* builder)
{
	const auto b = static_cast<DefaultEntityBuilder*>(builder); // 인간적으로 디폴트 빌더인걸 확신하고 그냥 지른다.

	auto instance = std::make_shared<udsdx::SceneObject>();
	instance->GetTransform()->SetLocalPosition(b->obj_pos);

	auto movement = instance->AddComponent<EntityMovement>();
	movement->SetFriction(100.0f);
	auto playerComponent = instance->AddComponent<PlayerRenderer>();
	auto serverComponent = instance->AddComponent<ServerObject>();
	serverComponent->SetObjID(builder->obj_id);

	auto moveInterpolator = serverComponent->AddComp<MoveInterpolator>();
	moveInterpolator->InitInterpolator(b->obj_pos);

	return instance;
}

std::shared_ptr<udsdx::SceneObject> EntityBuilderBase::Create_Monster(EntityBuilderBase* builder)
{
	const auto b = static_cast<DefaultEntityBuilder*>(builder);

	auto instance = std::make_shared<udsdx::SceneObject>();
	instance->GetTransform()->SetLocalPosition(b->obj_pos);

	auto monsterComponent = instance->AddComponent<Monster>();
	auto serverComponent = instance->AddComponent<ServerObject>();
	serverComponent->SetObjID(builder->obj_id);

	auto moveInterpolator = serverComponent->AddComp<MoveInterpolator>();
	moveInterpolator->InitInterpolator(b->obj_pos);

	return instance;
}

std::shared_ptr<udsdx::SceneObject> EntityBuilderBase::Create_NPC(EntityBuilderBase* builder)
{
	const auto b = static_cast<DefaultEntityBuilder*>(builder);

	auto instance = std::make_shared<udsdx::SceneObject>();
	instance->GetTransform()->SetLocalPosition(b->obj_pos);

	instance->AddComponent<EntityMovement>();
	auto serverComponent = instance->AddComponent<ServerObject>();
	serverComponent->SetObjID(builder->obj_id);

	auto moveInterpolator = serverComponent->AddComp<MoveInterpolator>();
	moveInterpolator->InitInterpolator(b->obj_pos);

	// TODO: 원거리 공격 몬스터가 NPC로 생성된다. Create_Moster로 통합하여 타입별로 컴포넌트 생성을 다양화시키는 방향
	auto renderer = instance->AddComponent<MonsterRenderer>();
	return instance;
}

std::shared_ptr<udsdx::SceneObject> EntityBuilderBase::Create_DropItem(EntityBuilderBase* builder)
{
	const auto b = static_cast<DefaultEntityBuilder*>(builder);

	auto instance = std::make_shared<udsdx::SceneObject>();
	instance->GetTransform()->SetLocalPosition(b->obj_pos);

	auto serverComponent = instance->AddComponent<ServerObject>();
	serverComponent->SetObjID(builder->obj_id);

	const auto item = serverComponent->AddComp<DropItem>();
	item->SetItemPos(b->obj_pos);
	item->SetMainHero(g_heroObj); // TODO: g_hero 처형
	auto renderer = instance->AddComponent<DropItemRenderer>();


	renderer->SetDropItem(b->obj_type);

	return instance;
}
