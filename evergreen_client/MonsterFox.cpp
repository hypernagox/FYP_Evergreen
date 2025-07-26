#include "MonsterFox.h"

void MonsterFox::OnInitialize()
{
	Monster::OnInitialize();

	m_renderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"fox\\model.yrms")));
	m_renderer->SetMaterial(udsdx::Material(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"nprcolor.hlsl")), INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"fox\\fox_high_DefaultMaterial_BaseColor.png"))));
	m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"fox\\fox_idle.yac")), true);
	m_rendererObj->GetTransform()->SetLocalScale(Vector3::One * GET_DATA(float, "GlobalValues", "CharacterScale", "Value"));

	InitializeMonster("Fox");
}

void MonsterFox::OnDeath()
{
	auto remainObject = SceneObject::MakeShared();
	auto remainComponent = remainObject->AddComponent<MonsterRemains>();
	remainComponent->InitializeMonster(m_rendererObj->GetTransform(), m_renderer, &INSTANCE(Resource)->Load<AnimationClip>(RESOURCE_PATH(L"fox\\fox_die.yac"))->GetAnimation());
	
	if (Scene* scene = GetSceneObject()->GetScene())
	{
		scene->AddObject(remainObject);
	}
}

void MonsterFox::OnAnimationStateChange(AnimationState from, AnimationState to)
{
	switch (to)
	{
	case AnimationState::Idle:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"fox\\fox_idle.yac")), true);
		break;
	case AnimationState::Run:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"fox\\fox_run_animation.yac")), true);
		break;
	case AnimationState::Attack:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"fox\\fox_attack.yac")), false, true);
		break;
	}

	Monster::OnAnimationStateChange(from, to);
}
