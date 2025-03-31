#include "pch.h"
#include "PlayerRenderer.h"
#include "EntityMovement.h"
#include "ServerObject.h"
#include "AuthenticPlayer.h"
#include "../common/json.hpp"

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
	m_playerMaterials[2]->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_eye_BaseColor.png")));
	m_playerMaterials[3]->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_face_BaseColor.png")));
	m_playerMaterials[0]->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_body_BaseColor.png")));
	m_playerMaterials[1]->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_hair_BaseColor.png")));

	auto sceneObject = GetSceneObject();
	sceneObject->AddChild(m_rendererObj);

	m_rendererObj->GetTransform()->SetLocalScale(Vector3::One * GET_DATA(float, "CharacterScale", "Value"));

	m_stateMachine = std::make_unique<Common::StateMachine<AnimationState>>(AnimationState::Idle);
	m_stateMachine->AddOnStateChangeCallback([this](AnimationState from, AnimationState to) { this->OnAnimationStateChange(to); });
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Idle, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Run, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Attack, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);
	m_stateMachine->AddTransition<Common::TimerStateTransition<AnimationState>>(AnimationState::Attack, AnimationState::Idle, 1.f);
	
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Hit, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);

	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Idle, AnimationState::Hit, m_stateMachine->GetConditionRefBool("Hit"), true);
	m_stateMachine->AddTransition<Common::TimerStateTransition<AnimationState>>(AnimationState::Hit, AnimationState::Idle, 0.365f);
	
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Hit, AnimationState::Death, m_stateMachine->GetConditionRefBool("Death"), true);


	m_stateMachine->AddTransition<Common::FloatStateTransition<AnimationState, std::greater_equal<float>>>(AnimationState::Idle, AnimationState::Run, m_stateMachine->GetConditionRefFloat("MoveSpeed"), 10.0f);
	m_stateMachine->AddTransition<Common::FloatStateTransition<AnimationState, std::less<float>>>(AnimationState::Run, AnimationState::Idle, m_stateMachine->GetConditionRefFloat("MoveSpeed"), 10.0f);


	m_stateMachine->AddTransition<Common::TimerStateTransition<AnimationState>>(AnimationState::Death, AnimationState::Idle, 2.f);

	{
		m_toolMaterial = std::make_shared<udsdx::Material>();
		m_toolMaterial->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\Weapon\\zelda sword\\zeldasword_albedo.jpg")));

		auto toolRenderer = pBody->AddComponent<RiggedPropRenderer>();
		toolRenderer->SetMesh(INSTANCE(Resource)->Load<udsdx::Mesh>(RESOURCE_PATH(L"Zelda\\Weapon\\zelda sword\\zeldasword.obj")));
		toolRenderer->SetShader(shader);
		toolRenderer->SetMaterial(m_toolMaterial.get());
		toolRenderer->SetBoneName("Bip001 R Hand");
		toolRenderer->SetPropLocalTransform(Matrix4x4::CreateScale(16.0f) * Matrix4x4::CreateFromYawPitchRoll(0.0f, -PIDIV2, 0.0f) * Matrix4x4::CreateTranslation(3.0f, 1.0f, -12.0f));
	}
	
	OnAnimationStateChange(AnimationState::Idle);
}

PlayerRenderer::~PlayerRenderer()
{
}

void PlayerRenderer::Update(const Time& time, Scene& scene)
{
	m_stateMachine->Update(time.deltaTime);
	const Vector3 velocity = GetComponent<EntityMovement>()->GetVelocity();
	float mag = Vector2(velocity.x, velocity.z).LengthSquared();
	*m_stateMachine->GetConditionRefFloat("MoveSpeed") = mag;
}

void PlayerRenderer::OnAnimationStateChange(const AnimationState& state)
{
	switch (state)
	{
	case AnimationState::Idle:
		m_attackState = 0;
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_stand.fbx")), true);
		break;
	case AnimationState::Run:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_run.fbx")), true);
		break;
	case AnimationState::Attack:
		switch (m_attackState)
		{
		case 0:
			m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_attack1.fbx")), false, true);
			break;
		case 1:
			m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_attack2.fbx")), false, true);
			break;
		case 2:
			m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_attack3.fbx")), false, true);
			break;
		case 3:
			m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_attack4.fbx")), false, true);
			break;
		}
		m_attackState = (m_attackState + 1) % 4;
		*m_stateMachine->GetConditionRefBool("Attack") = false;
		break;
	case AnimationState::Hit:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_hit.fbx")), false, true);
		*m_stateMachine->GetConditionRefBool("Hit") = false;
		break;
	case AnimationState::Death:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_die.fbx")), false);
		*m_stateMachine->GetConditionRefBool("Death") = false;
		break;
	}
}