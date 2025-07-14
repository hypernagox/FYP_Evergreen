#include "pch.h"
#include "MonsterBoss.h"
#include "EntityMovement.h"

void MonsterBoss::OnInitialize()
{
    Monster::OnInitialize();

    m_animation = INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"dragon\\dragon_animations.yac"));
	auto shader = INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"colorPBR.hlsl"));
    m_renderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"dragon\\model.yrms")));
    m_renderer->SetAnimation(m_animation, "Idle", true, false);
    {
        udsdx::Material mat = udsdx::Material(shader);
        mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonBody_albedoOpacity.png")), 0);
        mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonBody_normal.png")), 1);
        mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonBody_metallicRoughness.png")), 2);
        m_renderer->SetMaterial(mat, 5);
    }
    {
        udsdx::Material mat = udsdx::Material(shader);
        mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonCarapace_albedoOpacity.png")), 0);
        mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonCarapace_normal.png")), 1);
        mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonCarapace_metallicRoughness.png")), 2);
        m_renderer->SetMaterial(mat, 4);
    }
    {
        udsdx::Material mat = udsdx::Material(shader);
        mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonFins_albedoOpacity.png")), 0);
        mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonFins_normal.png")), 1);
        mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonFins_metallicRoughness.png")), 2);
        m_renderer->SetMaterial(mat, 3);
    }
    {
        udsdx::Material mat = udsdx::Material(shader);
        mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonHeadB_albedoOpacity.png")), 0);
        mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonHeadB_normal.png")), 1);
        mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonHeadB_metallicRoughness.png")), 2);
        m_renderer->SetMaterial(mat, 2);
    }
    {
        udsdx::Material mat = udsdx::Material(shader);
        mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonSpikes_albedoOpacity.png")), 0);
        mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonSpikes_normal.png")), 1);
        mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonSpikes_metallicRoughness.png")), 2);
        m_renderer->SetMaterial(mat, 1);
    }
    {
        udsdx::Material mat = udsdx::Material(shader);
        mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonTailB_albedoOpacity.png")), 0);
        mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonTailB_normal.png")), 1);
        mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"dragon\\DragonTailB_metallicRoughness.png")), 2);
        m_renderer->SetMaterial(mat, 0);
    }
}

void MonsterBoss::Update(const Time& time, Scene& scene)
{
    Monster::Update(time, scene);
}

void MonsterBoss::OnAnimationStateChange(AnimationState from, AnimationState to)
{
    switch (to)
    {
    case AnimationState::Idle:
        m_renderer->SetAnimation(m_animation, "Idle", true, false);
        break;
    case AnimationState::Run:
        m_renderer->SetAnimation(m_animation, "Run", true, false);
        break;
    case AnimationState::Attack:
        m_renderer->SetAnimation(m_animation, "Attack 2", false, true);
        break;
    }

	Monster::OnAnimationStateChange(from, to);
}
