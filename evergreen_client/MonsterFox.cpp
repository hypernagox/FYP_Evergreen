#include "MonsterFox.h"

void MonsterFox::OnInitialize()
{
	Monster::OnInitialize();

	m_rendererObj = std::make_shared<SceneObject>();

	std::shared_ptr<SceneObject> pBody = std::make_shared<SceneObject>();

	auto shader = INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"nprcolor.hlsl"));
	m_monsterMaterial = std::make_shared<udsdx::Material>();
	m_monsterMaterial->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"fox\\fox_high_DefaultMaterial_BaseColor.png")));

	m_transformBody = pBody->GetTransform();
	m_rendererObj->AddChild(pBody);
	m_rendererObj->GetTransform()->SetLocalPosition(Vector3::Up * -0.05f);

	m_riggedMeshRenderer = pBody->AddComponent<RiggedMeshRenderer>();
	m_riggedMeshRenderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"fox\\fox.yrms")));
	m_riggedMeshRenderer->SetShader(shader);
	m_riggedMeshRenderer->SetMaterial(m_monsterMaterial.get());
	m_riggedMeshRenderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"fox\\fox_idle.yac")), true);

	m_rendererObj->GetTransform()->SetLocalScale(Vector3::One / 48.0f);

	GetSceneObject()->AddChild(m_rendererObj);

	InitializeMonster("Fox");
}

void MonsterFox::OnAnimationStateChange(AnimationState from, AnimationState to)
{
	switch (to)
	{
	case AnimationState::Idle:
		m_riggedMeshRenderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"fox\\fox_idle.yac")), true);
		break;
	case AnimationState::Run:
		m_riggedMeshRenderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"fox\\fox_run_animation.yac")), true);
		break;
	case AnimationState::Attack:
		m_riggedMeshRenderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"fox\\fox_attack.yac")), false, true);
		break;
	}

	Monster::OnAnimationStateChange(from, to);
}
