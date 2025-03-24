#include "pch.h"
#include "Monster.h"
#include "EntityMovement.h"
#include "MonsterHPPanel.h"

Monster::Monster(const std::shared_ptr<SceneObject>& object) : Component(object)
{
	m_rendererObj = std::make_shared<SceneObject>();

	std::shared_ptr<SceneObject> pBody = std::make_shared<SceneObject>();

	auto shader = INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl"));
	m_monsterMaterial = std::make_shared<udsdx::Material>();
	m_monsterMaterial->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"fox\\fox_high_DefaultMaterial_BaseColor.png")));

	m_transformBody = pBody->GetTransform();
	m_rendererObj->AddChild(pBody);
	m_rendererObj->GetTransform()->SetLocalPosition(Vector3::Up * -0.05f);

	m_riggedMeshRenderer = pBody->AddComponent<RiggedMeshRenderer>();
	m_riggedMeshRenderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"fox\\fox.glb")));
	m_riggedMeshRenderer->SetShader(shader);
	m_riggedMeshRenderer->SetMaterial(m_monsterMaterial.get());

	m_rendererObj->GetTransform()->SetLocalScale(Vector3::One / 24.0f);

	m_entityMovement = object->AddComponent<EntityMovement>();

	GetSceneObject()->AddChild(m_rendererObj);

	m_stateMachine = std::make_unique<Common::StateMachine<AnimationState>>(AnimationState::Idle);
	m_riggedMeshRenderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"fox\\fox_idle.fbx")), true);

	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Idle, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);
	m_stateMachine->AddTransition<Common::TimerStateTransition<AnimationState>>(AnimationState::Attack, AnimationState::Idle, 0.365f);
	m_stateMachine->AddOnStateChangeCallback([this](AnimationState from, AnimationState to) { this->OnAnimationStateChange(from, to); });

	auto hpPanelObj = std::make_shared<SceneObject>();
	hpPanelObj->GetTransform()->SetLocalPosition(Vector3::Up * 3.0f);
	m_hpPanel = hpPanelObj->AddComponent<MonsterHPPanel>();
	GetSceneObject()->AddChild(hpPanelObj);
}

Monster::~Monster()
{
}

void Monster::Update(const Time& time, Scene& scene)
{
	m_stateMachine->Update(time.deltaTime);
}

void Monster::OnAttackToPlayer()
{
	*m_stateMachine->GetConditionRefBool("Attack") = true;
}

void Monster::OnAnimationStateChange(AnimationState from, AnimationState to)
{
	switch (to)
	{
	case AnimationState::Idle:
		m_riggedMeshRenderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"fox\\fox_idle.fbx")), true);
		break;
	case AnimationState::Attack:
		m_riggedMeshRenderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"fox\\fox_attack.fbx")), false, true);
		*m_stateMachine->GetConditionRefBool("Attack") = false;
		break;
	}
}

void Monster::OnHit(int afterHealth)
{
	m_hp = afterHealth;
	m_hpPanel->SetHPFraction(m_hp / 3.0f);
}
