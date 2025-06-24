#include "MonsterBear.h"

MonsterBear::MonsterBear(const std::shared_ptr<SceneObject>& object) : Monster(object)
{
	m_rendererObj = std::make_shared<SceneObject>();

	std::shared_ptr<SceneObject> pBody = std::make_shared<SceneObject>();

	auto shader = INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl"));
	m_monsterMaterial = std::make_shared<udsdx::Material>();
	m_monsterMaterial->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"bear\\bear_BaseColor.png")));

	m_transformBody = pBody->GetTransform();
	m_rendererObj->AddChild(pBody);

	m_riggedMeshRenderer = pBody->AddComponent<RiggedMeshRenderer>();
	m_riggedMeshRenderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"bear\\bear_tpose.yrms")));
	m_riggedMeshRenderer->SetShader(shader);
	m_riggedMeshRenderer->SetMaterial(m_monsterMaterial.get());
	m_riggedMeshRenderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"bear\\bear_idle.yac")), true);

	m_rendererObj->GetTransform()->SetLocalScale(Vector3::One * 2.0f);

	GetSceneObject()->AddChild(m_rendererObj);
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
