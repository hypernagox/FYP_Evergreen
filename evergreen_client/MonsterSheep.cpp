#include "pch.h"
#include "MonsterSheep.h"

using namespace udsdx;

void MonsterSheep::OnInitialize()
{
	Monster::OnInitialize();

	m_renderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"sheep\\model.yrms")));
	m_renderer->SetMaterial(udsdx::Material(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"nprcolor.hlsl")), INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"sheep\\sheep_BaseColor.png"))));
	m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"sheep\\sheep_idle.yac")), true);
	m_rendererObj->GetTransform()->SetLocalScale(Vector3::One * GET_DATA(float, "GlobalValues", "CharacterScale", "Value"));

	InitializeMonster("Sheep");
}

void MonsterSheep::OnDeath()
{
	auto remainObject = SceneObject::MakeShared();
	auto remainComponent = remainObject->AddComponent<MonsterRemains>();
	remainComponent->InitializeMonster(m_rendererObj->GetTransform(), m_renderer, &INSTANCE(Resource)->Load<AnimationClip>(RESOURCE_PATH(L"sheep\\sheep_die.yac"))->GetAnimation());

	if (Scene* scene = GetSceneObject()->GetScene())
	{
		scene->AddObject(remainObject);
	}
}

void MonsterSheep::OnAnimationStateChange(AnimationState from, AnimationState to)
{
	switch (to)
	{
	case AnimationState::Idle:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"sheep\\sheep_idle.yac")), true);
		break;
	case AnimationState::Run:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"sheep\\sheep_run.yac")), true);
		break;
	case AnimationState::Attack:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"sheep\\sheep_impact2.yac")), false, true);
		break;
	}

	Monster::OnAnimationStateChange(from, to);
}

