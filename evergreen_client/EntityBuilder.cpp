#include "pch.h"
#include "EntityBuilder.h"
#include "ServerObject.h"
#include "EntityMovement.h"
#include "PlayerRenderer.h"
#include "MonsterFox.h"
#include "MonsterSheep.h"
#include "MonsterBear.h"
#include "MonsterBoss.h"
#include "MoveInterpolator.h"
#include "DropItem.h"
#include "DropItemRenderer.h"
#include "../common/json.hpp"
#include "NPCRenderer.h"
#include "GizmoCylinderRenderer.h"
#include "ServerObjectMgr.h"
#include "GuideSystem.h"
#include "InteractiveEntity.h"
#include "GizmoSectorRenderer.h"
#include "GizmoBoxRenderer.h"
#include "ForcedMovement.h"
#include "PlayerTagGUI.h"
#include "DropItemAcquireRenderer.h"

// string 등 무브시맨틱이 유효한 데이터라면 무브시맨틱을 적극 고려하자

std::shared_ptr<udsdx::SceneObject> EntityBuilderBase::Create_Player(EntityBuilderBase* builder)
{
	const auto b = static_cast<DefaultEntityBuilder*>(builder); // 인간적으로 디폴트 빌더인걸 확신하고 그냥 지른다.

	auto instance = udsdx::SceneObject::MakeShared();
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
		playerComponent->InitializeArcher();
		break;
	}

	auto serverComponent = instance->AddComponent<ServerObject>();
	serverComponent->AddComp<ForcedMovement>();
	auto interactiveEntity = instance->AddComponent<InteractiveEntity>();
	interactiveEntity->SetInteractionText(L"상호작용 (파티초대)");
	interactiveEntity->SetInteractionCallback([id = builder->obj_id]() {
		Send(Create_c2s_INVITE_PARTY_QUEST(id));
		});
	serverComponent->SetObjID(builder->obj_id);

	auto moveInterpolator = serverComponent->AddComp<MoveInterpolator>();
	moveInterpolator->InitInterpolator(b->obj_pos);
	auto playerTag = instance->AddComponent<PlayerTagGUI>();
	if (ServerObjectMgr::GetInst()->m_equipmentMap.contains(b->obj_id))
	{
		const auto& [weapon_id, armor_id, username] = ServerObjectMgr::GetInst()->m_equipmentMap[b->obj_id];
		// TODO: 플레이어 이름 변경을 위한 네임태그 컴포넌트 수집
		playerComponent->SetPlayerWeapon(weapon_id);
		playerComponent->SetPlayerArmor(armor_id);
		playerTag->SetText(Common::DataRegistry::Str2Wstr(username));
		ServerObjectMgr::GetInst()->m_equipmentMap.erase(b->obj_id);
	}
	return instance;
}

std::shared_ptr<udsdx::SceneObject> EntityBuilderBase::Create_Monster(EntityBuilderBase* builder)
{
	const auto b = static_cast<DefaultEntityBuilder*>(builder);
	switch (b->obj_type)
	{
	case Nagox::Enum::MONSTER_TYPE::MONSTER_TYPE_BOSS:
	{
		auto instance = udsdx::SceneObject::MakeShared();
		instance->GetTransform()->SetLocalPosition(b->obj_pos);
		auto serverComponent = instance->AddComponent<ServerObject>();
		serverComponent->SetObjID(builder->obj_id);
		const auto col = instance->AddComponent<GizmoBoxRenderer>();
		col->SetSize(Vector3{ 3,3,6 });
		col->SetOffset(Vector3{ 0,1.25f,0 });
		
		auto moveInterpolator = serverComponent->AddComp<MoveInterpolator>();
		moveInterpolator->InitInterpolator(b->obj_pos);
		auto monsterComponent = instance->AddComponent<MonsterBoss>();
		GET_BOSS = instance;
		return instance;
	}
	
	case Nagox::Enum::MONSTER_TYPE::MONSTER_TYPE_FOX:
	{
		auto instance = udsdx::SceneObject::MakeShared();
		instance->GetTransform()->SetLocalPosition(b->obj_pos);
		auto serverComponent = instance->AddComponent<ServerObject>();
		auto moveInterpolator = serverComponent->AddComp<MoveInterpolator>();
		auto monsterComponent = instance->AddComponent<MonsterFox>();

		serverComponent->SetObjID(builder->obj_id);
		moveInterpolator->InitInterpolator(b->obj_pos);

		return instance;
	}
	case Nagox::Enum::MONSTER_TYPE::MONSTER_TYPE_SHEEP:
	{
		auto instance = udsdx::SceneObject::MakeShared();
		instance->GetTransform()->SetLocalPosition(b->obj_pos);
		auto serverComponent = instance->AddComponent<ServerObject>();
		auto moveInterpolator = serverComponent->AddComp<MoveInterpolator>();
		auto monsterComponent = instance->AddComponent<MonsterSheep>();

		serverComponent->SetObjID(builder->obj_id);
		moveInterpolator->InitInterpolator(b->obj_pos);
		return instance;
	}
	case Nagox::Enum::MONSTER_TYPE::MONSTER_TYPE_BEAR:
	{
		auto instance = udsdx::SceneObject::MakeShared();
		instance->GetTransform()->SetLocalPosition(b->obj_pos);

		auto serverComponent = instance->AddComponent<ServerObject>();
		auto moveInterpolator = serverComponent->AddComp<MoveInterpolator>();
		auto monsterComponent = instance->AddComponent<MonsterBear>();
		serverComponent->SetObjID(builder->obj_id);

		moveInterpolator->InitInterpolator(b->obj_pos);

		//TODO: 매직넘버
		std::shared_ptr<SceneObject> gizmoObject = SceneObject::MakeShared();
		gizmoObject->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(PI, 0.0f, 0.0f));
		instance->AddChild(gizmoObject);

		const auto sector = gizmoObject->AddComponent<GizmoSectorRenderer>();
		sector->SetAngle(62.5f);
		sector->SetRadius(4.5f);

		return instance;
	}
	case Nagox::Enum::MONSTER_TYPE::MONSTER_TYPE_GOBLIN:
	{
		const auto b = static_cast<DefaultEntityBuilder*>(builder);

		auto instance = udsdx::SceneObject::MakeShared();
		instance->GetTransform()->SetLocalPosition(b->obj_pos);

		instance->AddComponent<EntityMovement>();
		auto serverComponent = instance->AddComponent<ServerObject>();
		auto moveInterpolator = serverComponent->AddComp<MoveInterpolator>();
		serverComponent->SetObjID(builder->obj_id);

		moveInterpolator->InitInterpolator(b->obj_pos);

		auto renderer = instance->AddComponent<NPCRenderer>();
		return instance;
	}
	}
	return nullptr; // TODO: 예외처리
}

std::shared_ptr<udsdx::SceneObject> EntityBuilderBase::Create_NPC(EntityBuilderBase* builder)
{
	// TODO: 이제부턴 진짜 NPC임
	const auto b = static_cast<DefaultEntityBuilder*>(builder);

	switch (b->obj_type)
	{
	case Nagox::Enum::NPC_TYPE::NPC_TYPE_CHIEF:
	{
		auto instance = udsdx::SceneObject::MakeShared();
		instance->GetTransform()->SetLocalPosition(b->obj_pos);

		auto serverComponent = instance->AddComponent<ServerObject>();
		serverComponent->SetObjID(builder->obj_id);

		auto moveInterpolator = serverComponent->AddComp<MoveInterpolator>();
		moveInterpolator->InitInterpolator(b->obj_pos);

		auto renderer = instance->AddComponent<NPCRenderer>();
		return instance;
	}
	// TODO: 투석기 그리기
	case Nagox::Enum::NPC_TYPE::NPC_TYPE_CATAPULT:
	{
		auto instance = udsdx::SceneObject::MakeShared();
		instance->GetTransform()->SetLocalPosition(b->obj_pos);

		auto serverComponent = instance->AddComponent<ServerObject>();
		serverComponent->SetObjID(builder->obj_id);

		auto moveInterpolator = serverComponent->AddComp<MoveInterpolator>();
		moveInterpolator->InitInterpolator(b->obj_pos);

		auto rendererObject = SceneObject::MakeShared();
		rendererObject->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(PIDIV2, 0.0f, 0.0f));
		instance->AddChild(rendererObject);

		auto renderer = rendererObject->AddComponent<RiggedMeshRenderer>();

		udsdx::Material mat = udsdx::Material(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"colornormal.hlsl")));
		mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"catapult\\Catapult_BaseColor.png")), 0);
		mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"catapult\\Catapult_Normal.png")), 1);

		renderer->SetMaterial(mat);
		renderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"catapult\\catapult.yrms")));
		renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"catapult\\idle.yac")), true);
		instance->AddComponent<EntityMovement>();

		auto interactiveEntity = instance->AddComponent<InteractiveEntity>();
		interactiveEntity->SetInteractionText(L"투석기 활성화");
		interactiveEntity->SetInteractionCallback([renderer]() {
			Send(Create_c2s_SHOOT_CATAPULT(ToFlatVec3(Vector3{}))); // TODO: 정확한 위치
			static std::unique_ptr<SoundEffectInstance> g_effectSound;
			g_effectSound = INSTANCE(Resource)->Load<AudioClip>(RESOURCE_PATH(L"audio\\catapult.wav"))->CreateInstance3D(renderer->GetTransform()->GetWorldPosition());
			renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"catapult\\throw.yac")), false, true);
			});

		return instance;
	}
	default:
		auto instance = udsdx::SceneObject::MakeShared();
		instance->GetTransform()->SetLocalPosition(b->obj_pos);

		auto serverComponent = instance->AddComponent<ServerObject>();
		serverComponent->SetObjID(builder->obj_id);

		auto moveInterpolator = serverComponent->AddComp<MoveInterpolator>();
		moveInterpolator->InitInterpolator(b->obj_pos);

		auto renderer = instance->AddComponent<NPCRenderer>();
		return instance;
	}
}

std::shared_ptr<udsdx::SceneObject> EntityBuilderBase::Create_DropItem(EntityBuilderBase* builder)
{
	const auto b = static_cast<DefaultEntityBuilder*>(builder);
	uint32_t owner_id = ServerObjectMgr::GetInst()->GetMainHero()->GetObjID();

	auto instance = udsdx::SceneObject::MakeShared();
	instance->GetTransform()->SetLocalPosition(b->obj_pos);

	auto serverComponent = instance->AddComponent<ServerObject>();
	serverComponent->SetObjID(builder->obj_id);

	const auto item = serverComponent->AddComp<DropItem>();
	item->SetItemPos(b->obj_pos);
	item->SetMainHero(ServerObjectMgr::GetInst()->GetMainHero()->GetSceneObject());
	auto renderer = instance->AddComponent<DropItemRenderer>();
	auto interactiveEntity = instance->AddComponent<InteractiveEntity>();
	interactiveEntity->SetInteractionText(L"획득하기");
	interactiveEntity->SetInteractionCallback([instance_weak = std::weak_ptr(instance), owner_id, id = builder->obj_id]() {
		// TODO: 플레이어가 상호작용을 통해 아이템을 주웠을 때의 코드 영역
		// 기존 인자인 owner_id(줍는 플레이어)에 더해 프로토콜에서 주울 아이템의 id를 추가적으로 정의해주어야 한다.
		if (instance_weak.expired())
			return;
		auto instance = instance_weak.lock();
		if (Scene* target = instance->GetScene())
		{
			// 아이템 획득 이펙트 생성
			std::shared_ptr<SceneObject> acquireObject = SceneObject::MakeShared();
			auto renderer = acquireObject->AddComponent<DropItemAcquireRenderer>();
			renderer->Initialize(instance->GetComponentsInChildren<MeshRenderer>()[0]);
			target->AddObject(acquireObject);

			instance->SetActive(false);
		}
		 Send(Create_c2s_ACQUIRE_ITEM(owner_id, id));
		});

	renderer->SetDropItem(b->obj_type);

	return instance;
}

std::shared_ptr<udsdx::SceneObject> EntityBuilderBase::Create_Harvest(EntityBuilderBase* builder)
{
	// TODO: 종류 / 크기 ..
	const auto b = static_cast<DefaultEntityBuilder*>(builder);
	auto s = SceneObject::MakeShared();

	s->GetTransform()->SetLocalPosition(b->obj_pos);
	
	return s;
}
