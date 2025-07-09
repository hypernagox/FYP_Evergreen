#include "MonsterBear.h"

void MonsterBear::OnInitialize()
{
	Monster::OnInitialize();

	m_rendererObj = std::make_shared<SceneObject>();

	std::shared_ptr<SceneObject> pBody = std::make_shared<SceneObject>();

	m_transformBody = pBody->GetTransform();
	m_rendererObj->AddChild(pBody);

	m_riggedMeshRenderer = pBody->AddComponent<RiggedMeshRenderer>();
	m_riggedMeshRenderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"bear\\bear_tpose.yrms")));
	m_riggedMeshRenderer->SetMaterial(udsdx::Material(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"nprcolor.hlsl")), INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"bear\\bear_BaseColor.png"))));
	m_riggedMeshRenderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"bear\\bear_idle.yac")), true);

	m_rendererObj->GetTransform()->SetLocalScale(Vector3::One * 2.0f);

	GetSceneObject()->AddChild(m_rendererObj);

	InitializeMonster("Bear");
}

void MonsterBear::OnAnimationStateChange(AnimationState from, AnimationState to)
{
	switch (to)
	{
	case AnimationState::Idle:
		m_riggedMeshRenderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"bear\\bear_idle.yac")), true);
		break;
	case AnimationState::Run:
		m_riggedMeshRenderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"bear\\bear_walk.yac")), true);
		break;
	case AnimationState::Attack:
		m_riggedMeshRenderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"bear\\bear_attack.yac")), false, true);
		break;
	}

	Monster::OnAnimationStateChange(from, to);
}
