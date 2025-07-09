#include "pch.h"
#include "MonsterRenderer.h"
#include "GizmoSphereRenderer.h"

using namespace udsdx;

// TODO: MonsterRenderer는 몬스터 타입에 따라 다양화 시키는게 좋을 것 같다. 추후 상속이나 참조를 통해 속성을 변경할 수 있어야 한다.

void MonsterRenderer::OnInitialize()
{
	m_rendererObject = std::make_shared<udsdx::SceneObject>();
	m_rendererObject->GetTransform()->SetLocalScale(Vector3::One * 0.015f);

	auto renderer = m_rendererObject->AddComponent<udsdx::RiggedMeshRenderer>();
	renderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"sheep\\sheep_max.yrms")));
	renderer->SetMaterial(udsdx::Material(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"nprcolor.hlsl")), INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"sheep\\sheep_BaseColor.png"))), 0);
	renderer->SetMaterial(udsdx::Material(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"nprcolor.hlsl")), INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"goblin\\Goblin(Wizard)\\goblin_notex_DefaultMaterial_BaseColor.png"))), 0);
	renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"sheep\\sheep_run.yac")), true);

	GetSceneObject()->AddChild(m_rendererObject);
}
