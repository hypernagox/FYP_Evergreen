#include "MonsterBear.h"

void MonsterBear::OnInitialize()
{
	Monster::OnInitialize();

	m_renderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"bear\\model.yrms")));
	m_renderer->SetMaterial(udsdx::Material(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"nprcolor.hlsl")), INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"bear\\bear_BaseColor.png"))));
	m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"bear\\bear_idle.yac")), true);

	m_rendererObj->GetTransform()->SetLocalScale(Vector3::One * 2.0f);

	InitializeMonster("Bear");
}

void MonsterBear::OnAnimationStateChange(AnimationState from, AnimationState to)
{
	switch (to)
	{
	case AnimationState::Idle:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"bear\\bear_idle.yac")), true);
		break;
	case AnimationState::Run:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"bear\\bear_walk.yac")), true);
		break;
	case AnimationState::Attack:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"bear\\bear_attack.yac")), false, true);
		break;
	}

	Monster::OnAnimationStateChange(from, to);
}
