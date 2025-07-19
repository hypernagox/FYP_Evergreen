#include "pch.h"
#include "MonsterBoss.h"
#include "EntityMovement.h"
#include "ServerObject.h"

void MonsterBoss::OnInitialize()
{
    Monster::OnInitialize();

    m_serverObject = GetComponent<ServerObject>();

    m_animation = INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"dragon\\dragon_animations.yac"));
    m_flightAnimation = INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"dragon\\dragon_animations_flight.yac"));

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

    m_stateMachine->AddTransition<Common::AnimationStateTransition<AnimationState>>(AnimationState::BossTakeoff, AnimationState::BossFlyIdle, m_renderer);

    m_stateMachine->AddTransition<Common::FloatStateTransition<AnimationState, std::greater<float>>>(AnimationState::BossFlyIdle, AnimationState::BossFlyRun, m_stateMachine->GetConditionRefFloat("Speed"), 1e-3f);
    m_stateMachine->AddTransition<Common::FloatStateTransition<AnimationState, std::less_equal<float>>>(AnimationState::BossFlyRun, AnimationState::BossFlyIdle, m_stateMachine->GetConditionRefFloat("Speed"), 1e-3f);

    m_stateMachine->AddTransition<Common::AnimationStateTransition<AnimationState>>(AnimationState::BossLanding, AnimationState::Idle, m_renderer);

    InitializeMonster("Boss");
}

void MonsterBoss::Update(const Time& time, Scene& scene)
{
    m_eventTimer.Update(time.deltaTime);

    if (m_isFlyMovement)
    {
        UpdateFlightMovement(time.deltaTime);
    }

    Monster::Update(time, scene);
}

void MonsterBoss::UpdateFlightMovement(float deltaTime)
{
    m_flightTime += deltaTime;
    if (m_flightTime >= m_flightTimeTotal)
    {
        if (m_isTakeoff)
        {
            m_entityMovement->SetPosition(m_flightEndPosition);
        }
        else
        {
            m_stateMachine->SetState(AnimationState::BossLanding);
        }
        m_flightTime = 0.0f;
        m_isFlyMovement = false;
    }
    else
    {
        m_entityMovement->SetPosition(Vector3::SmoothStep(m_flightStartPosition, m_flightEndPosition, m_flightTime / std::max(1e-4f, m_flightTimeTotal)));

        Vector3 direction = m_flightEndPosition - m_flightStartPosition;
        direction.y = 0.0f; // 비행 중에는 y축 방향을 무시하고 수평 이동만 고려
        if (direction.Length() > 1e-4f)
        {
            direction.Normalize();
            m_entityMovement->SetRotation(Quaternion::Slerp(m_entityMovement->GetRotation(), Quaternion::LookRotation(direction, Vector3::Up), deltaTime * 2.0f));
		}
    }
}

// 날아오르는 패턴:
// 먼저 Takeoff 애니메이션을 재생하고, 애니메이션이 끝나면 y 좌표를 +5 더해준 다음 해당 위치로 이동(SmoothStep)하면서 Fly 애니메이션 재생.
// 5를 더해준 이유는 Takeoff 애니메이션이 끝나고 나면 y 좌표가 5만큼 올라가 있기 때문.
void MonsterBoss::OnTakeoffAtPosition(const Vector3& pos)
{
    m_serverObject->SetActive(false);
    m_entityMovement->SetActive(false);
    m_stateMachine->SetState(AnimationState::BossTakeoff);

    m_flightStartPosition = m_entityMovement->GetPosition() + Vector3::Up * 5.0f;
    m_flightEndPosition = pos;
    m_flightTimeTotal = (m_flightEndPosition - m_flightStartPosition).Length() / 10.0f;
    m_isTakeoff = true;
}

// 착륙하는 패턴:
// 먼저 해당 위치에서 y 좌표를 +5 만큼 더해준 위치로 이동(SmoothStep)하면서 Fly 애니메이션을 재생하고, Landing 애니메이션 재생.
// 이동이 모두 끝나면 pos 위치로 이동해야함.
void MonsterBoss::OnLandingAtPosition(const Vector3& pos)
{
    m_flightStartPosition = m_entityMovement->GetPosition();
    m_flightEndPosition = pos + Vector3::Up * 5.0f;
    m_flightTimeTotal = (m_flightEndPosition - m_flightStartPosition).Length() / 10.0f;
    m_isTakeoff = false;
    m_isFlyMovement = true;
}

void MonsterBoss::OnAnimationStateChange(AnimationState from, AnimationState to)
{
    switch (to)
    {
    case AnimationState::BossTakeoff:
		m_renderer->SetAnimation(m_flightAnimation, "Idle Takeoff", false, true);
		break;
    case AnimationState::BossLanding:
        m_renderer->SetAnimation(m_flightAnimation, "Idle Landing", false, true);
        break;
    case AnimationState::BossFlyIdle:
        m_renderer->SetAnimation(m_animation, "Fly Idle", true, false);
		break;
    case AnimationState::BossFlyRun:
		m_renderer->SetAnimation(m_animation, "Fly", true, false);
		break;
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

    switch (from)
    {
    case AnimationState::BossTakeoff:
        m_entityMovement->SetPosition(m_entityMovement->GetPosition() + Vector3::Up * 5.0f);
        m_renderer->SetTransitionFactor(1.0f);

        m_isFlyMovement = true;
        break;

    case AnimationState::BossLanding:
        m_entityMovement->SetPosition(m_flightEndPosition + Vector3::Down * 5.0f);
        m_renderer->SetTransitionFactor(1.0f);

        m_serverObject->SetActive(true);
        m_entityMovement->SetActive(true);
        break;
    }

	Monster::OnAnimationStateChange(from, to);
}