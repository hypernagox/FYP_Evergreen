#include "pch.h"
#include "Monster.h"
#include "EntityMovement.h"
#include "MonsterHPPanel.h"
#include "GameScene.h"

void Monster::OnInitialize()
{
	m_rendererObj = std::make_shared<SceneObject>();
	m_renderer = m_rendererObj->AddComponent<RiggedMeshRenderer>();
	GetSceneObject()->AddChild(m_rendererObj);

	m_entityMovement = GetSceneObject()->AddComponent<EntityMovement>();

	m_stateMachine = std::make_unique<Common::StateMachine<AnimationState>>(AnimationState::Idle);

	m_stateMachine->AddTransition<Common::FloatStateTransition<AnimationState, std::greater<float>>>(AnimationState::Idle, AnimationState::Run, m_stateMachine->GetConditionRefFloat("Speed"), 1e-3f);
	m_stateMachine->AddTransition<Common::FloatStateTransition<AnimationState, std::less_equal<float>>>(AnimationState::Run, AnimationState::Idle, m_stateMachine->GetConditionRefFloat("Speed"), 1e-3f);

	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Idle, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Run, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);

	m_stateMachine->AddTransition<Common::AnimationStateTransition<AnimationState>>(AnimationState::Attack, AnimationState::Idle, m_renderer);
	m_stateMachine->AddOnStateChangeCallback([this](AnimationState from, AnimationState to) { this->OnAnimationStateChange(from, to); });

	auto hpPanelObj = std::make_shared<SceneObject>();
	hpPanelObj->GetTransform()->SetLocalPosition(Vector3::Up * 1.5f);
	m_hpPanel = hpPanelObj->AddComponent<MonsterHPPanel>();
	GetSceneObject()->AddChild(hpPanelObj);
}

void Monster::InitializeMonster(std::string_view monsterType)
{
	m_maxHP = GET_DATA(int, "Monster", monsterType, "hp");
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
}
