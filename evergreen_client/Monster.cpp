#include "pch.h"
#include "Monster.h"
#include "EntityMovement.h"

Monster::Monster(const std::shared_ptr<SceneObject>& object) : Component(object)
{
	m_rendererObj = std::make_shared<SceneObject>();

	std::shared_ptr<SceneObject> pBody = std::make_shared<SceneObject>();

	auto shader = INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl"));
	m_monsterMaterial = std::make_shared<udsdx::Material>();
	m_monsterMaterial->SetMainTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"fox\\fox_high_DefaultMaterial_BaseColor.png")));

	m_transformBody = pBody->GetTransform();
	m_rendererObj->AddChild(pBody);
	m_rendererObj->GetTransform()->SetLocalPosition(Vector3::Up * -0.05f);

	auto pBodyMesh = pBody->AddComponent<MeshRenderer>();
	pBodyMesh->SetMesh(INSTANCE(Resource)->Load<udsdx::Mesh>(RESOURCE_PATH(L"fox\\fox_low.fbx")));
	pBodyMesh->SetShader(shader);
	pBodyMesh->SetMaterial(m_monsterMaterial.get());

	m_transformBody->SetLocalScale(Vector3::One * 0.025f);

	m_entityMovement = object->AddComponent<EntityMovement>();

	GetSceneObject()->AddChild(m_rendererObj);
}

Monster::~Monster()
{
}

void Monster::Update(const Time& time, Scene& scene)
{
}