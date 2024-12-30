#include "pch.h"
#include "PlayerRenderer.h"
#include "EntityMovement.h"
#include "ServerObject.h"

extern std::shared_ptr<SceneObject> g_heroObj;

PlayerRenderer::PlayerRenderer(const std::shared_ptr<SceneObject>& object) : Component(object)
{
	m_rendererObj = std::make_shared<SceneObject>();

	std::shared_ptr<SceneObject> pBody = std::make_shared<SceneObject>();

	auto shader = INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl"));

	m_transformBody = pBody->GetTransform();
	m_rendererObj->AddChild(pBody);

	m_renderer = pBody->AddComponent<RiggedMeshRenderer>();
	m_renderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"Zelda\\zelda.glb")));
	m_renderer->SetShader(shader);

	for (int i = 0; i < m_playerMaterials.size(); ++i)
	{
		m_playerMaterials[i] = std::make_shared<udsdx::Material>();
		m_renderer->SetMaterial(m_playerMaterials[i].get(), i);
	}
	m_playerMaterials[2]->SetMainTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_eye_BaseColor.png")));
	m_playerMaterials[3]->SetMainTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_face_BaseColor.png")));
	m_playerMaterials[0]->SetMainTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_body_BaseColor.png")));
	m_playerMaterials[1]->SetMainTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_hair_BaseColor.png")));

	auto sceneObject = GetSceneObject();
	sceneObject->AddChild(m_rendererObj);

	m_rendererObj->GetTransform()->SetLocalScale(Vector3::One / 32);

	m_stateMachine = std::make_unique<Common::StateMachine<AnimationState>>(AnimationState::Idle);
	m_stateMachine->AddOnStateChangeCallback([this](AnimationState from, AnimationState to) { this->OnAnimationStateChange(to); });
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Idle, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Run, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);
	m_stateMachine->AddTransition<Common::TimerStateTransition<AnimationState>>(AnimationState::Attack, AnimationState::Idle, 0.365f);
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Idle, AnimationState::Hit, m_stateMachine->GetConditionRefBool("Hit"), true);
	m_stateMachine->AddTransition<Common::TimerStateTransition<AnimationState>>(AnimationState::Hit, AnimationState::Idle, 0.365f);
	m_stateMachine->AddTransition<Common::FloatStateTransition<AnimationState, std::greater_equal<float>>>(AnimationState::Idle, AnimationState::Run, m_stateMachine->GetConditionRefFloat("MoveSpeed"), 10.0f);
	m_stateMachine->AddTransition<Common::FloatStateTransition<AnimationState, std::less<float>>>(AnimationState::Run, AnimationState::Idle, m_stateMachine->GetConditionRefFloat("MoveSpeed"), 10.0f);
	OnAnimationStateChange(AnimationState::Idle);
}

PlayerRenderer::~PlayerRenderer()
{
}

void PlayerRenderer::Update(const Time& time, Scene& scene)
{
	m_stateMachine->Update(time.deltaTime);

	const bool flag = g_heroObj->GetComponent<ServerObject>()->GetObjID() == GetSceneObject()->GetComponent<ServerObject>()->GetObjID();
	if (INSTANCE(Input)->GetMouseLeftButtonDown() && flag)
	{
		Attack();
	}
	const Vector3 velocity = GetComponent<EntityMovement>()->GetVelocity();
	float mag = Vector2(velocity.x, velocity.z).LengthSquared();
	*m_stateMachine->GetConditionRefFloat("MoveSpeed") = mag;
}

void PlayerRenderer::OnAnimationStateChange(const AnimationState& state)
{
	switch (state)
	{
	case AnimationState::Idle:
		m_renderer->SetAnimation("Bip001|stand|BaseLayer");
		break;
	case AnimationState::Run:
		m_renderer->SetAnimation("Bip001|run|BaseLayer");
		break;
	case AnimationState::Attack:
		m_renderer->SetAnimation("Bip001|attack1|BaseLayer");
		*m_stateMachine->GetConditionRefBool("Attack") = false;
		break;
	case AnimationState::Hit:
		m_renderer->SetAnimation("Bip001|hit|BaseLayer");
		*m_stateMachine->GetConditionRefBool("Hit") = false;
		break;
	case AnimationState::Death:
		m_renderer->SetAnimation("Bip001|die|BaseLayer");
		*m_stateMachine->GetConditionRefBool("Death") = false;
		break;
	}
}