#include "pch.h"
#include "EntityBuilder.h"
#include "ServerObject.h"
#include "EntityMovement.h"
#include "PlayerRenderer.h"
#include "MonsterFox.h"
#include "MonsterBear.h"
#include "MoveInterpolator.h"
#include "DropItem.h"
#include "DropItemRenderer.h"
#include "../common/json.hpp"
#include "MonsterRenderer.h"
#include "GizmoCylinderRenderer.h"
#include "ServerObjectMgr.h"
#include "GuideSystem.h"
#include "InteractiveEntity.h"
#include "GizmoSectorRenderer.h"

// string 등 무브시맨틱이 유효한 데이터라면 무브시맨틱을 적극 고려하자

std::shared_ptr<udsdx::SceneObject> EntityBuilderBase::Create_Player(EntityBuilderBase* builder)
{
	const auto b = static_cast<DefaultEntityBuilder*>(builder); // 인간적으로 디폴트 빌더인걸 확신하고 그냥 지른다.

	auto instance = std::make_shared<udsdx::SceneObject>();
	instance->GetTransform()->SetLocalPosition(b->obj_pos);

	auto movement = instance->AddComponent<EntityMovement>();
	movement->SetFriction(0.0f);
	auto playerComponent = instance->AddComponent<PlayerRenderer>();
	switch (builder->obj_type)
	{
	case 0:
		playerComponent->InitializeWarrior();
		break;
	case 1:
		playerComponent->InitializePriest();
		break;
	default:
		playerComponent->InitializePriest();
		break;
	}

	auto serverComponent = instance->AddComponent<ServerObject>();
	auto interactiveEntity = instance->AddComponent<InteractiveEntity>();
	interactiveEntity->SetInteractionText(L"상호작용 (파티초대)");
	interactiveEntity->SetInteractionCallback([id = builder->obj_id]() {
		Send(Create_c2s_INVITE_PARTY_QUEST(id));
		});
	serverComponent->SetObjID(builder->obj_id);

	auto moveInterpolator = serverComponent->AddComp<MoveInterpolator>();
	moveInterpolator->InitInterpolator(b->obj_pos);
	if (ServerObjectMgr::GetInst()->m_weaponMap.contains(b->obj_id))
	{
		playerComponent->SetPlayerWeapon(DATA_TABLE->GetWeaponIDStr(ServerObjectMgr::GetInst()->m_weaponMap[b->obj_id]));
		ServerObjectMgr::GetInst()->m_weaponMap.erase(b->obj_id);
	}
	return instance;
}

std::shared_ptr<udsdx::SceneObject> EntityBuilderBase::Create_Monster(EntityBuilderBase* builder)
{
	const auto b = static_cast<DefaultEntityBuilder*>(builder);
	switch (b->obj_type)
	{
	case Nagox::Enum::MONSTER_TYPE::MONSTER_TYPE_FOX:
	{
		auto instance = std::make_shared<udsdx::SceneObject>();
		instance->GetTransform()->SetLocalPosition(b->obj_pos);

		auto monsterComponent = instance->AddComponent<MonsterFox>();
		auto serverComponent = instance->AddComponent<ServerObject>();
		serverComponent->SetObjID(builder->obj_id);

		auto moveInterpolator = serverComponent->AddComp<MoveInterpolator>();
		moveInterpolator->InitInterpolator(b->obj_pos);

		return instance;
		break;
	}
	case Nagox::Enum::MONSTER_TYPE::MONSTER_TYPE_SHEEP:
	{
		// TODO: 양 생성
		break;
	}
	case Nagox::Enum::MONSTER_TYPE::MONSTER_TYPE_BEAR:
	{
		auto instance = std::make_shared<udsdx::SceneObject>();
		instance->GetTransform()->SetLocalPosition(b->obj_pos);
		
		auto monsterComponent = instance->AddComponent<MonsterBear>();
		auto serverComponent = instance->AddComponent<ServerObject>();
		serverComponent->SetObjID(builder->obj_id);

		auto moveInterpolator = serverComponent->AddComp<MoveInterpolator>();
		moveInterpolator->InitInterpolator(b->obj_pos);

		//TODO: 매직넘버
		const auto sector = instance->AddComponent<GizmoSectorRenderer>();
		sector->SetAngle(62.5f);
		sector->SetRadius(4.5f);
		return instance;
	}
	case Nagox::Enum::MONSTER_TYPE::MONSTER_TYPE_GOBLIN:
	{
		const auto b = static_cast<DefaultEntityBuilder*>(builder);

		auto instance = std::make_shared<udsdx::SceneObject>();
		instance->GetTransform()->SetLocalPosition(b->obj_pos);

		instance->AddComponent<EntityMovement>();
		auto serverComponent = instance->AddComponent<ServerObject>();
		serverComponent->SetObjID(builder->obj_id);

		auto moveInterpolator = serverComponent->AddComp<MoveInterpolator>();
		moveInterpolator->InitInterpolator(b->obj_pos);

		auto renderer = instance->AddComponent<MonsterRenderer>();
		return instance;
		break;
	}
	default:
		break;
	}
}

std::shared_ptr<udsdx::SceneObject> EntityBuilderBase::Create_NPC(EntityBuilderBase* builder)
{
	// TODO: 이제부턴 진짜 NPC임
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
	uint32_t owner_id = ServerObjectMgr::GetInst()->GetMainHero()->GetObjID();

	auto instance = std::make_shared<udsdx::SceneObject>();
	instance->GetTransform()->SetLocalPosition(b->obj_pos);

	auto serverComponent = instance->AddComponent<ServerObject>();
	serverComponent->SetObjID(builder->obj_id);

	const auto item = serverComponent->AddComp<DropItem>();
	item->SetItemPos(b->obj_pos);
	item->SetMainHero(ServerObjectMgr::GetInst()->GetMainHero()->GetSceneObject());
	auto renderer = instance->AddComponent<DropItemRenderer>();
	auto interactiveEntity = instance->AddComponent<InteractiveEntity>();
	interactiveEntity->SetInteractionText(L"획득하기");
	interactiveEntity->SetInteractionCallback([owner_id, id = builder->obj_id]() {
		// TODO: 플레이어가 상호작용을 통해 아이템을 주웠을 때의 코드 영역
		// 기존 인자인 owner_id(줍는 플레이어)에 더해 프로토콜에서 주울 아이템의 id를 추가적으로 정의해주어야 한다.
		// 
		 Send(Create_c2s_ACQUIRE_ITEM(owner_id, id));
		});

	renderer->SetDropItem(b->obj_type);

	return instance;
}

std::shared_ptr<udsdx::SceneObject> EntityBuilderBase::Create_Harvest(EntityBuilderBase* builder)
{
	// TODO: 종류 / 크기 ..
	const auto b = static_cast<DefaultEntityBuilder*>(builder);
	auto s = std::make_shared<SceneObject>();

	s->GetTransform()->SetLocalPosition(b->obj_pos);
	
	return s;
}
