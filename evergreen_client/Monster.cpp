#include "pch.h"
#include "Monster.h"
#include "EntityMovement.h"
#include "MonsterHPPanel.h"
#include "GameScene.h"
#include "SphereParticleEmitter.h"

void Monster::OnInitialize()
{
	m_rendererObj = SceneObject::MakeShared();
	m_renderer = m_rendererObj->AddComponent<RiggedMeshRenderer>();
	GetSceneObject()->AddChild(m_rendererObj);

	m_hpPanel = AddComponent<MonsterHPPanel>();
	m_entityMovement = GetSceneObject()->AddComponent<EntityMovement>();

	m_stateMachine = std::make_unique<Common::StateMachine<AnimationState>>(AnimationState::Idle);

	m_stateMachine->AddTransition<Common::FloatStateTransition<AnimationState, std::greater<float>>>(AnimationState::Idle, AnimationState::Run, m_stateMachine->GetConditionRefFloat("Speed"), 1e-3f);
	m_stateMachine->AddTransition<Common::FloatStateTransition<AnimationState, std::less_equal<float>>>(AnimationState::Run, AnimationState::Idle, m_stateMachine->GetConditionRefFloat("Speed"), 1e-3f);

	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Idle, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Run, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);

	m_stateMachine->AddTransition<Common::AnimationStateTransition<AnimationState>>(AnimationState::Attack, AnimationState::Idle, m_renderer);
	m_stateMachine->AddOnStateChangeCallback([this](AnimationState from, AnimationState to) { this->OnAnimationStateChange(from, to); });
}

void Monster::InitializeMonster(std::string_view monsterType)
{
	m_maxHP = GET_DATA(int, "Monster", monsterType, "hp");
	m_hpPanel->SetText(GET_DATA(std::wstring, "Monster", monsterType, "Name"));
	m_hp = m_maxHP;
}

void Monster::Update(const Time& time, Scene& scene)
{
	float distance = (m_lastPosition - m_entityMovement->GetPosition()).Length();
	*m_stateMachine->GetConditionRefFloat("Speed") = distance;
	m_lastPosition = m_entityMovement->GetPosition();
	m_stateMachine->Update(time.deltaTime);

	if (GameScene* gameScene = dynamic_cast<GameScene*>(&scene))
	{
		gameScene->AddMinimapMark(GetTransform()->GetLocalPosition());
	}

	m_renderer->SetBoneModifier("Bip001 Spine1", Matrix4x4::CreateFromYawPitchRoll(0.0f, 0.0f, -PIDIV4 * m_hitfactor));
	m_renderer->SetBoneModifier("mixamorig:Spine1", Matrix4x4::CreateFromYawPitchRoll(0.0f, PIDIV4 * m_hitfactor, 0.0f));
	m_hitfactor = std::lerp(m_hitfactor, 0.0f, time.deltaTime * 16.0f);
}

void Monster::OnAttackToPlayer()
{
	*m_stateMachine->GetConditionRefBool("Attack") = true;
}

void Monster::OnAnimationStateChange(AnimationState from, AnimationState to)
{
	switch (to)
	{
	case AnimationState::Attack:
		*m_stateMachine->GetConditionRefBool("Attack") = false;
		break;
	}
}

void Monster::OnHit(int afterHealth)
{
	m_hp = afterHealth;
	m_hpPanel->SetHPFraction(static_cast<float>(afterHealth) / m_maxHP);
	m_hitfactor = 1.0f;

	{
		auto hitParticleObj = SceneObject::MakeShared();
		hitParticleObj->GetTransform()->SetLocalPosition(GetTransform()->GetLocalPosition() + Vector3::Up);
		{
			auto particleEmitter = hitParticleObj->AddComponent<SphereParticleEmitter>();
			particleEmitter->SetColor(Vector3(1.0f, 0.8326f, 0.0f) * 10.0f);
			particleEmitter->SetDrawCount(16);
			particleEmitter->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"particle\\hit_triangle.png")));
			particleEmitter->GetEmitterParameter().LifeTimeMin = 0.2f;
			particleEmitter->GetEmitterParameter().LifeTimeMax = 0.3f;
			particleEmitter->GetEmitterParameter().SizeMin = Vector2(0.2f, 0.2f);
			particleEmitter->GetEmitterParameter().SizeMax = Vector2(2.0f, 0.2f);
			particleEmitter->GetEmitterParameter().SizeLifeExp = Vector2::One * 0.25f;
			particleEmitter->GetEmitterParameter().AlphaLifeExp = -1.0f;
			particleEmitter->SetEmitLoop(false);
			particleEmitter->SetOrientedByDirection(true);
			particleEmitter->SetAutoDestroy(true);
			particleEmitter->Play();
		}

		auto starParticleObj = SceneObject::MakeShared();
		starParticleObj->GetTransform()->SetLocalPosition(GetTransform()->GetLocalPosition() + Vector3::Up);
		{
			auto particleEmitter = starParticleObj->AddComponent<SphereParticleEmitter>();
			particleEmitter->SetColor(Vector3(1.0f, 0.8326f, 0.0f) * 4.0f);
			particleEmitter->SetDrawCount(16);
			particleEmitter->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"particle\\star.png")));
			particleEmitter->GetEmitterParameter().LifeTimeMin = 0.4f;
			particleEmitter->GetEmitterParameter().LifeTimeMax = 0.6f;
			particleEmitter->GetEmitterParameter().SpeedMin = 4.0f;
			particleEmitter->GetEmitterParameter().SpeedMax = 6.0f;
			particleEmitter->GetEmitterParameter().SpeedLifeExp = 0.25f;
			particleEmitter->GetEmitterParameter().SizeMin = Vector2(0.2f, 0.2f);
			particleEmitter->GetEmitterParameter().SizeMax = Vector2(0.2f, 0.2f);
			particleEmitter->GetEmitterParameter().SizeLifeExp = Vector2::One * -0.25f;
			particleEmitter->GetEmitterParameter().RotationMin = 3.0f;
			particleEmitter->GetEmitterParameter().RotationMax = 6.0f;
			particleEmitter->GetEmitterParameter().RotationLifeExp = 0.25f;
			particleEmitter->SetEmitLoop(false);
			particleEmitter->SetAutoDestroy(true);
			particleEmitter->Play();
		}

		if (Scene* scene = GetSceneObject()->GetScene())
		{
			scene->AddObject(hitParticleObj);
			scene->AddObject(starParticleObj);
		}
	}

	if (m_hp <= 0)
	{
		GetSceneObject()->SetActive(false);
		OnDeath();
	}
}

void MonsterRemains::OnInitialize()
{
	m_rendererObj = SceneObject::MakeShared();
	m_renderer = m_rendererObj->AddComponent<RiggedMeshRenderer>();
	GetSceneObject()->AddChild(m_rendererObj);
}

void MonsterRemains::OnActive()
{
	m_soundInstance = INSTANCE(Resource)->Load<udsdx::AudioClip>(RESOURCE_PATH(L"audio\\monster_death.wav"))->CreateInstance();
	m_soundInstance->SetVolume(0.3f);
	m_soundInstance->Play();
}

void MonsterRemains::InitializeMonster(Transform* bodyTransform, RiggedMeshRenderer* renderer, const Animation* deathAnimation)
{
	auto mesh = renderer->GetMesh();
	int submeshCount = static_cast<int>(mesh->GetSubmeshes().size());

	m_renderer->SetMesh(mesh);
	for (int submeshIndex = 0; submeshIndex < submeshCount; ++submeshIndex)
		m_renderer->SetMaterial(renderer->GetMaterial(submeshIndex), submeshIndex);
	m_renderer->SetAnimation(deathAnimation, false, true);

	GetTransform()->SetLocalPosition(bodyTransform->GetWorldPosition());
	GetTransform()->SetLocalRotation(bodyTransform->GetWorldRotation());
	m_rendererObj->GetTransform()->SetLocalScale(bodyTransform->GetLocalScale());
}

void MonsterRemains::Update(const Time& time, Scene& scene)
{
	if (!m_renderer->IsAnimationPlaying())
	{
		m_lifeTime -= time.deltaTime;
		if (m_lifeTime <= 0.0f)
		{
			GetSceneObject()->RemoveFromParent();
		}
		else
		{
			GetTransform()->SetLocalScale(std::powf(m_lifeTime, 0.25f));
		}
	}
}