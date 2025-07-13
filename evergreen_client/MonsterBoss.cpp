#include "pch.h"
#include "MonsterBoss.h"

void MonsterBoss::OnInitialize()
{
	m_riggedMeshRenderer = AddComponent<RiggedMeshRenderer>();
	auto shader = INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl"));
	m_riggedMeshRenderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"dragon\\model.yrms")));
	m_riggedMeshRenderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonBody_albedoOpacity.png"))), 5);
	m_riggedMeshRenderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonCarapace_albedoOpacity.png"))), 4);
	m_riggedMeshRenderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonFins_albedoOpacity.png"))), 3);
	m_riggedMeshRenderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonHeadB_albedoOpacity.png"))), 2);
	m_riggedMeshRenderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonSpikes_albedoOpacity.png"))), 1);
	m_riggedMeshRenderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonTailB_albedoOpacity.png"))), 0);
	m_riggedMeshRenderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"dragon\\animations.yac")), true);

	Monster::OnInitialize();
}

void MonsterBoss::OnAnimationStateChange(AnimationState from, AnimationState to)
{
	Monster::OnAnimationStateChange(from, to);
}
