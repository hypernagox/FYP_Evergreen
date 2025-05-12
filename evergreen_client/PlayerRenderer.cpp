#include "pch.h"
#include "PlayerRenderer.h"
#include "EntityMovement.h"
#include "ServerObject.h"
#include "AuthenticPlayer.h"
#include "../common/json.hpp"

PlayerRenderer::PlayerRenderer(const std::shared_ptr<SceneObject>& object) : Component(object)
{
	m_rendererObj = std::make_shared<SceneObject>();

	InitializePriest();

	GetSceneObject()->AddChild(m_rendererObj);

	m_rendererObj->GetTransform()->SetLocalScale(Vector3::One * GET_DATA(float, "CharacterScale", "Value"));

	m_stateMachine = std::make_unique<Common::StateMachine<AnimationState>>(AnimationState::Idle);
	m_stateMachine->AddOnStateChangeCallback([this](AnimationState from, AnimationState to) { this->OnAnimationStateChange(to); });
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Idle, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Attack, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);
	m_stateMachine->AddTransition<Common::TimerStateTransition<AnimationState>>(AnimationState::Attack, AnimationState::Idle, 0.8f);
	
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Hit, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);

	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Idle, AnimationState::Hit, m_stateMachine->GetConditionRefBool("Hit"), true);
	m_stateMachine->AddTransition<Common::TimerStateTransition<AnimationState>>(AnimationState::Hit, AnimationState::Idle, 0.365f);
	
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Hit, AnimationState::Death, m_stateMachine->GetConditionRefBool("Death"), true);

	const AnimationState runStates[] = {
		AnimationState::RunBackward,
		AnimationState::RunRightBackward,
		AnimationState::RunRight,
		AnimationState::RunRightForward,
		AnimationState::RunForward,
		AnimationState::RunLeftForward,
		AnimationState::RunLeft,
		AnimationState::RunLeftBackward
	};

	m_stateMachine->AddTransition<Common::FloatStateTransition<AnimationState, std::greater<float>>>(AnimationState::Idle, AnimationState::RunIntermediate, m_stateMachine->GetConditionRefFloat("MoveSpeed"), 0.0f);
	m_stateMachine->AddTransition<Common::FloatStateTransition<AnimationState, std::less_equal<float>>>(AnimationState::RunIntermediate, AnimationState::Idle, m_stateMachine->GetConditionRefFloat("MoveSpeed"), 0.0f);
	for (int i = 0; i < 8; ++i)
	{
		const AnimationState runState = runStates[i];
		m_stateMachine->AddTransition<Common::IntStateTransition<AnimationState, std::equal_to<int>>>(AnimationState::RunIntermediate, runState, m_stateMachine->GetConditionRefInt("MoveAngle"), i);
		m_stateMachine->AddTransition<Common::IntStateTransition<AnimationState, std::not_equal_to<int>>>(runState, AnimationState::RunIntermediate, m_stateMachine->GetConditionRefInt("MoveAngle"), i);
		m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(runState, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);
		m_stateMachine->AddTransition<Common::FloatStateTransition<AnimationState, std::less_equal<float>>>(runState, AnimationState::Idle, m_stateMachine->GetConditionRefFloat("MoveSpeed"), 0.0f);
	}

	m_stateMachine->AddTransition<Common::TimerStateTransition<AnimationState>>(AnimationState::Death, AnimationState::Idle, 2.f);
	
	OnAnimationStateChange(AnimationState::Idle);
}

void PlayerRenderer::InitializeWarrior()
{
	std::shared_ptr<SceneObject> pBody = std::make_shared<SceneObject>();

	m_renderer = pBody->AddComponent<RiggedMeshRenderer>();
	m_renderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"Zelda\\zelda.glb")));
	m_renderer->SetShader(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl")));

	m_transformBody = pBody->GetTransform();
	m_rendererObj->AddChild(pBody);

	m_transformBody->SetLocalPositionY(-5.5f);

	m_toolMaterial = std::make_shared<udsdx::Material>();
	m_toolMaterial->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\Weapon\\zelda sword\\zeldasword_albedo.jpg")));

	auto toolRenderer = pBody->AddComponent<RiggedPropRenderer>();
	toolRenderer->SetMesh(INSTANCE(Resource)->Load<udsdx::Mesh>(RESOURCE_PATH(L"Zelda\\Weapon\\zelda sword\\zeldasword.obj")));
	toolRenderer->SetShader(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl")));
	toolRenderer->SetMaterial(m_toolMaterial.get());
	toolRenderer->SetBoneName("Bip001 R Hand");
	toolRenderer->SetPropLocalTransform(Matrix4x4::CreateScale(16.0f) * Matrix4x4::CreateFromYawPitchRoll(0.0f, -PIDIV2, 0.0f) * Matrix4x4::CreateTranslation(3.0f, 1.0f, -12.0f));

	for (int i = 0; i < m_playerMaterials.size(); ++i)
	{
		m_playerMaterials[i] = std::make_shared<udsdx::Material>();
		m_renderer->SetMaterial(m_playerMaterials[i].get(), i);
	}
	m_playerMaterials[2]->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_eye_BaseColor.png")));
	m_playerMaterials[3]->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_face_BaseColor.png")));
	m_playerMaterials[0]->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_body_BaseColor.png")));
	m_playerMaterials[1]->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_hair_BaseColor.png")));
}

void PlayerRenderer::InitializePriest()
{
	std::shared_ptr<SceneObject> pBody = std::make_shared<SceneObject>();

	m_renderer = pBody->AddComponent<RiggedMeshRenderer>();
	m_renderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"priest\\priest.fbx")));
	m_renderer->SetShader(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"colornotex.hlsl")));

	m_transformBody = pBody->GetTransform();
	m_rendererObj->AddChild(pBody);

	m_transformBody->SetLocalPositionY(-5.5f);

	m_toolMaterial = std::make_shared<udsdx::Material>();
	m_toolMaterial->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\Weapon\\saber excalibur\\saberexcalibur_albedo.png")));

	auto toolRenderer = pBody->AddComponent<RiggedPropRenderer>();
	toolRenderer->SetMesh(INSTANCE(Resource)->Load<udsdx::Mesh>(RESOURCE_PATH(L"Zelda\\Weapon\\saber excalibur\\saberexcalibur.obj")));
	toolRenderer->SetShader(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl")));
	toolRenderer->SetMaterial(m_toolMaterial.get());
	toolRenderer->SetBoneName("Bip001 R Hand");
	toolRenderer->SetPropLocalTransform(Matrix4x4::CreateScale(0.1f) * Matrix4x4::CreateFromYawPitchRoll(0.0f, -PIDIV2, 0.0f) * Matrix4x4::CreateTranslation(3.0f, 1.0f, -12.0f));
}

void PlayerRenderer::Update(const Time& time, Scene& scene)
{
	EntityMovement* entityMovement = GetComponent<EntityMovement>();
	Vector3 acceleration = Vector3::Zero;
	if (entityMovement != nullptr)
	{
		acceleration = entityMovement->GetAcceleration();
	}
	int moveAngleInt = -1;
	if (acceleration.LengthSquared() > 0.1f)
	{
		const Vector3 forward = Vector3::Transform(Vector3::Forward, m_rendererObj->GetTransform()->GetWorldRotation());
		const float moveAngle = std::atan2f(forward.x, forward.z) - std::atan2f(acceleration.x, acceleration.z);
		moveAngleInt = static_cast<int>(moveAngle * 4.0f / PI + 12.5f) % 8;
	}
	float magnitude = Vector2(acceleration.x, acceleration.z).LengthSquared();
	*m_stateMachine->GetConditionRefFloat("MoveSpeed") = magnitude;
	*m_stateMachine->GetConditionRefInt("MoveAngle") = moveAngleInt;
	m_stateMachine->Update(time.deltaTime);
}

void PlayerRenderer::OnAnimationStateChange(const AnimationState& state)
{
	switch (state)
	{
	case AnimationState::Idle:
		m_attackState = 0;
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_stand.fbx")), true);
		break;
	case AnimationState::RunForward:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\AnimationJog\\jog_forward_fast.fbx")), true);
		break;
	case AnimationState::RunBackward:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\AnimationJog\\jog_backward_slow.fbx")), true);
		break;
	case AnimationState::RunLeft:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\AnimationJog\\jog_strafe_left.fbx")), true);
		break;
	case AnimationState::RunRight:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\AnimationJog\\jog_strafe_right.fbx")), true);
		break;
	case AnimationState::RunLeftForward:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\AnimationJog\\jog_forward_diagonal_left.fbx")), true);
		break;
	case AnimationState::RunRightForward:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\AnimationJog\\jog_forward_diagonal_right.fbx")), true);
		break;
	case AnimationState::RunLeftBackward:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\AnimationJog\\jog_backward_diagonal_left.fbx")), true);
		break;
	case AnimationState::RunRightBackward:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\AnimationJog\\jog_backward_diagonal_right.fbx")), true);
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

bool PlayerRenderer::GetIsRunning() const
{
	return m_stateMachine->GetCurrentState() == AnimationState::RunForward ||
		m_stateMachine->GetCurrentState() == AnimationState::RunBackward ||
		m_stateMachine->GetCurrentState() == AnimationState::RunLeft ||
		m_stateMachine->GetCurrentState() == AnimationState::RunRight ||
		m_stateMachine->GetCurrentState() == AnimationState::RunLeftForward ||
		m_stateMachine->GetCurrentState() == AnimationState::RunRightForward ||
		m_stateMachine->GetCurrentState() == AnimationState::RunLeftBackward ||
		m_stateMachine->GetCurrentState() == AnimationState::RunRightBackward;
}
