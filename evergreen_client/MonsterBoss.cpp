#include "pch.h"
#include "MonsterBoss.h"
#include "EntityMovement.h"
#include "ServerObject.h"
#include "BossStatusGUI.h"
#include "GameScene.h"
#include "CylinderParticleEmitter.h"

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

    m_stateMachine->AddTransition<Common::FloatStateTransition<AnimationState, std::greater<float>>>(AnimationState::BossFlyIdle, AnimationState::BossFlyRun, m_stateMachine->GetConditionRefFloat("Speed"), 0.0f);
    m_stateMachine->AddTransition<Common::FloatStateTransition<AnimationState, std::less_equal<float>>>(AnimationState::BossFlyRun, AnimationState::BossFlyIdle, m_stateMachine->GetConditionRefFloat("Speed"), 0.0f);

    m_stateMachine->AddTransition<Common::AnimationStateTransition<AnimationState>>(AnimationState::BossSwingLeft, AnimationState::BossSwingRight, m_renderer);
    m_stateMachine->AddTransition<Common::AnimationStateTransition<AnimationState>>(AnimationState::BossSwingRight, AnimationState::BossBreathe, m_renderer);
    m_stateMachine->AddTransition<Common::AnimationStateTransition<AnimationState>>(AnimationState::BossBreathe, AnimationState::Idle, m_renderer);

    m_stateMachine->AddTransition<Common::AnimationStateTransition<AnimationState>>(AnimationState::BossLanding, AnimationState::Idle, m_renderer);

    m_bossStatusGUI = SceneObject::MakeShared();
    auto bossStatusComponent = m_bossStatusGUI->AddComponent<BossStatusGUI>();

    InitializeMonster("Boss");
}

void MonsterBoss::OnAttach()
{
    auto scene = GetSceneObject()->GetScene();
    if (scene != nullptr)
    {
        auto gameScene = dynamic_cast<GameScene*>(scene);
        if (gameScene != nullptr)
        {
            gameScene->AddInterfaceObject(m_bossStatusGUI, true);
            gameScene->SetBossMonsterCinematic(GetSceneObject());
        }
    }
}

void MonsterBoss::OnDetach()
{
    m_bossStatusGUI->RemoveFromParent();
}

void MonsterBoss::Update(const Time& time, Scene& scene)
{
    m_eventTimer.Update(time.deltaTime);

    if (m_isFlyMovement)
        UpdateFlightMovement(time.deltaTime);
    if (m_isTakeoff)
    {
        if (!m_isFlyMovement)
            m_entityMovement->SetRotation(Quaternion::Slerp(m_entityMovement->GetRotation(), Quaternion::CreateFromYawPitchRoll(m_flightViewAngle * DEG2RAD + PI, 0.0f, 0.0f), time.deltaTime * 2.0f));
        m_wingTimer -= time.deltaTime;
        if (m_wingTimer <= 0.0f)
        {
            m_wingTimer = 1.0f;
            m_wingSoundInstance = INSTANCE(Resource)->Load<udsdx::AudioClip>(RESOURCE_PATH(L"audio\\dragon_wing.wav"))->CreateInstance3D(GetTransform()->GetWorldPosition());
        }
    }

    Monster::Update(time, scene);
}

void MonsterBoss::OnHit(int afterHealth)
{
    Monster::OnHit(afterHealth);
    m_bossStatusGUI->GetComponent<BossStatusGUI>()->SetHPFraction(static_cast<float>(afterHealth) / m_maxHP);
}

void MonsterBoss::OnDeath()
{
    auto remainObject = SceneObject::MakeShared();
    auto remainComponent = remainObject->AddComponent<MonsterRemains>();
    remainComponent->InitializeMonster(m_rendererObj->GetTransform(), m_renderer, &m_animation->GetAnimation("Death"), true);

    if (Scene* scene = GetSceneObject()->GetScene())
    {
        scene->AddObject(remainObject);
    }
}

void MonsterBoss::UpdateFlightMovement(float deltaTime)
{
    m_flightTime += deltaTime;
    if (m_flightTime >= m_flightTimeTotal)
    {
        if (m_isTakeoff)
        {
            m_entityMovement->SetPosition(m_flightEndPosition);
            m_soundInstance = INSTANCE(Resource)->Load<udsdx::AudioClip>(RESOURCE_PATH(L"audio\\dragon_attack.wav"))->CreateInstance3D(GetTransform()->GetWorldPosition());
        }
        else if (m_stateMachine->GetCurrentState() != AnimationState::BossFlyDeath)
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
//
// 보스가 기절한 상태에서 해당 함수가 호출되는 것은, 기절이 해제되는 경우로 해당 로직을 처리해야 한다.
void MonsterBoss::OnLandingAtPosition(const Vector3& pos)
{
    if (m_stateMachine->GetCurrentState() == AnimationState::BossFlyDeath)
    {
        m_stateMachine->SetState(AnimationState::Idle);
        m_serverObject->SetActive(true);
        m_entityMovement->SetActive(true);
    }
    else
    {
        m_flightStartPosition = m_entityMovement->GetPosition();
        m_flightEndPosition = pos + Vector3::Up * 5.0f;
        m_flightTimeTotal = (m_flightEndPosition - m_flightStartPosition).Length() / 10.0f;
        m_isTakeoff = false;
        m_isFlyMovement = true;
    }
}

// 보스가 비행 중 투석기에 맞아 떨어지는 패턴:
//
void MonsterBoss::OnCatapultHit(const Vector3& hitPosition)
{
    m_stateMachine->SetState(AnimationState::BossFlyDeath);

    m_flightStartPosition = m_entityMovement->GetPosition();
    m_flightEndPosition = hitPosition;
    m_flightTimeTotal = 0.733f;
    m_isTakeoff = false;
    m_isFlyMovement = true;
}

void MonsterBoss::OnPhaseChange()
{
    m_stateMachine->SetState(AnimationState::BossSwingLeft);
}

void MonsterBoss::OnBreathAttack()
{
    m_stateMachine->SetState(AnimationState::BossBreathe);
}

void MonsterBoss::AppendBreatheEmitter()
{
    Vector3 bossPosition = GetTransform()->GetWorldPosition();
    Quaternion bossRotation = GetTransform()->GetWorldRotation();

    auto bossParticleAnchor = SceneObject::MakeShared();
    bossParticleAnchor->GetTransform()->SetLocalPosition(bossPosition + Vector3::Transform(Vector3(0.0f, 1.7574f, -3.5323f), bossRotation));
    bossParticleAnchor->GetTransform()->SetLocalRotation(Quaternion::Concatenate(bossRotation, Quaternion::CreateFromYawPitchRoll(PI, 0.0f, 0.0f)));
    
    if (Scene* scene = GetSceneObject()->GetScene())
    {
		scene->AddObject(bossParticleAnchor);
	}

    {
        auto particleEmitterObj = SceneObject::MakeShared();
        particleEmitterObj->GetTransform()->SetLocalScale(Vector3(0.01f, 0.01f, 1.0f));
        auto particleEmitter = particleEmitterObj->AddComponent<CylinderParticleEmitter>();
        particleEmitter->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"particle\\clouds.png")));
        particleEmitter->SetColor(Vector3(1.0f, 1.0f, 1.0f) * 1.0f);
        particleEmitter->SetDrawCount(16);
        particleEmitter->GetEmitterParameter().LifeTimeMin = 1.0f;
        particleEmitter->GetEmitterParameter().LifeTimeMax = 2.0f;
        particleEmitter->GetEmitterParameter().SpeedMin = 4.0f;
        particleEmitter->GetEmitterParameter().SpeedMax = 8.0f;
        particleEmitter->GetEmitterParameter().SpeedLifeExp = 0.5f;
        particleEmitter->GetEmitterParameter().SizeMin = Vector2::One * 2.0f;
        particleEmitter->GetEmitterParameter().SizeMax = Vector2::One * 4.0f;
        particleEmitter->GetEmitterParameter().SizeLifeExp = Vector2::One * 0.5f;
        particleEmitter->GetEmitterParameter().AlphaLifeExp = -1.0f;
        particleEmitter->SetEmitLoop(false);
        particleEmitter->SetAutoDestroy(true);
        particleEmitter->SetOrientedByDirection(false);
        particleEmitter->Play();

        bossParticleAnchor->AddChild(particleEmitterObj);
    }

    {
        auto particleEmitterObj = SceneObject::MakeShared();
        particleEmitterObj->GetTransform()->SetLocalScale(Vector3(0.01f, 0.01f, 1.0f));
        auto particleEmitter = particleEmitterObj->AddComponent<CylinderParticleEmitter>();
        particleEmitter->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"particle\\clouds.png")));
        particleEmitter->SetColor(Vector3(1.0f, 0.6f, 0.0f) * 10.0f);
        particleEmitter->SetDrawCount(16);
        particleEmitter->GetEmitterParameter().LifeTimeMin = 0.5f;
        particleEmitter->GetEmitterParameter().LifeTimeMax = 1.0f;
        particleEmitter->GetEmitterParameter().SpeedMin = 0.1f;
        particleEmitter->GetEmitterParameter().SpeedMax = 8.0f;
        particleEmitter->GetEmitterParameter().SpeedLifeExp = 0.5f;
        particleEmitter->GetEmitterParameter().SizeMin = Vector2::One * 1.0f;
        particleEmitter->GetEmitterParameter().SizeMax = Vector2::One * 2.0f;
        particleEmitter->GetEmitterParameter().SizeLifeExp = Vector2::One * 0.5f;
        particleEmitter->GetEmitterParameter().AlphaLifeExp = -1.0f;
        particleEmitter->SetEmitLoop(false);
        particleEmitter->SetAutoDestroy(true);
        particleEmitter->SetOrientedByDirection(false);
        particleEmitter->Play();

        bossParticleAnchor->AddChild(particleEmitterObj);
    }

    {
        auto particleEmitterObj = SceneObject::MakeShared();
        particleEmitterObj->GetTransform()->SetLocalScale(Vector3(0.01f, 0.01f, 1.0f));
        auto particleEmitter = particleEmitterObj->AddComponent<CylinderParticleEmitter>();
        particleEmitter->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"particle\\clouds.png")));
        particleEmitter->SetColor(Vector3(1.0f, 0.1f, 0.0f) * 10.0f);
        particleEmitter->SetDrawCount(16);
        particleEmitter->GetEmitterParameter().LifeTimeMin = 0.25f;
        particleEmitter->GetEmitterParameter().LifeTimeMax = 0.5f;
        particleEmitter->GetEmitterParameter().SpeedMin = 0.1f;
        particleEmitter->GetEmitterParameter().SpeedMax = 8.0f;
        particleEmitter->GetEmitterParameter().SpeedLifeExp = 0.5f;
        particleEmitter->GetEmitterParameter().SizeMin = Vector2::One * 1.0f;
        particleEmitter->GetEmitterParameter().SizeMax = Vector2::One * 2.0f;
        particleEmitter->GetEmitterParameter().SizeLifeExp = Vector2::One * 0.5f;
        particleEmitter->GetEmitterParameter().AlphaLifeExp = -1.0f;
        particleEmitter->SetEmitLoop(false);
        particleEmitter->SetAutoDestroy(true);
        particleEmitter->SetOrientedByDirection(false);
        particleEmitter->Play();

        bossParticleAnchor->AddChild(particleEmitterObj);
    }
}

void MonsterBoss::SetFlightViewAngle(float angle)
{
    m_flightViewAngle = angle;
}

void MonsterBoss::OnAnimationStateChange(AnimationState from, AnimationState to)
{
    std::cout << "MonsterBoss::OnAnimationStateChange: " << static_cast<int>(from) << " -> " << static_cast<int>(to) << std::endl;
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
    case AnimationState::BossFlyDeath:
        m_renderer->SetAnimation(m_flightAnimation, "Fly Death 3", false, true);
        break;
    case AnimationState::BossSwingLeft:
        m_renderer->SetAnimation(m_animation, "Tail Whip L", false, true);
        break;
    case AnimationState::BossSwingRight:
        m_renderer->SetAnimation(m_animation, "Tail Whip R", false, true);
        break;
    case AnimationState::BossBreathe:
        m_renderer->SetAnimation(m_animation, "Breathe Fire", false, true);
        m_eventTimer.RegisterEvent(0.5f, [this]() {
            if (m_stateMachine->GetCurrentState() != AnimationState::BossBreathe)
            {
                return;
            }
            AppendBreatheEmitter();
            });
        break;
    case AnimationState::Idle:
        m_renderer->SetAnimation(m_animation, "Idle", true, false);
        break;
    case AnimationState::Run:
        m_renderer->SetAnimation(m_animation, "Run", true, false);
        break;
    case AnimationState::Attack:
        m_renderer->SetAnimation(m_animation, "Attack 2", false, true);
        {
            m_soundInstance = INSTANCE(Resource)->Load<udsdx::AudioClip>(RESOURCE_PATH(L"audio\\dragon_impact.wav"))->CreateInstance();
            m_soundInstance->Play();
        }
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