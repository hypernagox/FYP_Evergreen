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
#include "GizmoCylinderRenderer.h"
#include "ServerObjectMgr.h"
#include "GuideSystem.h"
#include "InteractiveEntity.h"

// string �� ����ø�ƽ�� ��ȿ�� �����Ͷ�� ����ø�ƽ�� ���� �������

std::shared_ptr<udsdx::SceneObject> EntityBuilderBase::Create_Warrior(EntityBuilderBase* builder)
{
	const auto b = static_cast<DefaultEntityBuilder*>(builder); // �ΰ������� ����Ʈ �����ΰ� Ȯ���ϰ� �׳� ������.

	auto instance = std::make_shared<udsdx::SceneObject>();
	instance->GetTransform()->SetLocalPosition(b->obj_pos);

	auto movement = instance->AddComponent<EntityMovement>();
	movement->SetFriction(0.0f);
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

	// TODO: ���Ÿ� ���� ���Ͱ� NPC�� �����ȴ�. Create_Moster�� �����Ͽ� Ÿ�Ժ��� ������Ʈ ������ �پ�ȭ��Ű�� ����
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
	item->SetMainHero(ServerObjectMgr::GetInst()->GetMainHero()->GetSceneObject());
	auto renderer = instance->AddComponent<DropItemRenderer>();
	auto interactiveEntity = instance->AddComponent<InteractiveEntity>();
	interactiveEntity->SetInteractionText(L"ȹ���ϱ�");

	renderer->SetDropItem(b->obj_type);

	return instance;
}

std::shared_ptr<udsdx::SceneObject> EntityBuilderBase::Create_Harvest(EntityBuilderBase* builder)
{
	// TODO: ���� / ũ�� ..
	const auto b = static_cast<DefaultEntityBuilder*>(builder);
	auto s = std::make_shared<SceneObject>();

	s->GetTransform()->SetLocalPosition(b->obj_pos);
	
	return s;
}
