#include "pch.h"
#include "PlayerRenderer.h"
#include "EntityMovement.h"
#include "ServerObject.h"
#include "AuthenticPlayer.h"
#include "../common/json.hpp"

void PlayerRenderer::OnInitialize()
{
	m_rendererObj = std::make_shared<SceneObject>();

	GetSceneObject()->AddChild(m_rendererObj);

	m_rendererObj->GetTransform()->SetLocalScale(Vector3::One * GET_DATA(float, "GlobalValues", "CharacterScale", "Value"));

	m_stateMachine = std::make_unique<Common::StateMachine<AnimationState>>(AnimationState::Idle);
	m_stateMachine->AddOnStateChangeCallback([this](AnimationState from, AnimationState to) { this->OnAnimationStateChange(to); });
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Idle, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Attack, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);
	m_stateMachine->AddTransition<Common::TimerStateTransition<AnimationState>>(AnimationState::Attack, AnimationState::AttackEnd, 0.25f);
	m_stateMachine->AddTransition<Common::TimerStateTransition<AnimationState>>(AnimationState::AttackEnd, AnimationState::Idle, 0.3f);
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::AttackEnd, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);

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

	m_stateMachine->SetStateUpdateFp(AnimationState::Attack, [this]() {
		static bool flags[4];
		});
}

void PlayerRenderer::InitializeWarrior()
{
	m_bodyObj = std::make_shared<SceneObject>();

	m_renderer = m_bodyObj->AddComponent<RiggedMeshRenderer>();
	m_renderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"Zelda\\zelda.yrms")));

	m_transformBody = m_bodyObj->GetTransform();
	m_rendererObj->AddChild(m_bodyObj);

	m_transformBody->SetLocalPositionY(-5.5f);

	SetPlayerWeapon("Master Sword");
	OnAnimationStateChange(AnimationState::Idle);
	m_characterType = CharacterType::Warrior;
	SetEquipmentState(false);
}

void PlayerRenderer::InitializePriest()
{
	m_bodyObj = std::make_shared<SceneObject>();

	m_renderer = m_bodyObj->AddComponent<RiggedMeshRenderer>();
	m_renderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"priest\\priest.yrms")));

	m_transformBody = m_bodyObj->GetTransform();
	m_rendererObj->AddChild(m_bodyObj);

	m_transformBody->SetLocalPositionY(-5.5f);

	SetPlayerWeapon("Staff Priest");
	OnAnimationStateChange(AnimationState::Idle);
	m_characterType = CharacterType::Priest;
	SetEquipmentState(false);
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

void PlayerRenderer::SetEquipmentState(bool isEquipped)
{
	Shader* shader = INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"nprcolor.hlsl"));

	switch (m_characterType)
	{
		case CharacterType::Warrior:
			m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(isEquipped ? L"Zelda\\zelda_body_BaseColor_Armor.png" : L"Zelda\\zelda_body_BaseColor.png"))), 0);
			m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_hair_BaseColor.png"))), 1);
			m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_eye_BaseColor.png"))), 2);
			m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_face_BaseColor.png"))), 3);
			break;
		case CharacterType::Priest:
			m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(isEquipped ? L"priest\\priest_diffuse_2a.png" : L"priest\\priest_diffuse_2.png"))), 0);
			m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(isEquipped ? L"priest\\priest_diffuse_0a.png" : L"priest\\priest_diffuse_0.png"))), 1);
			m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(isEquipped ? L"priest\\priest_diffuse_2a.png" : L"priest\\priest_diffuse_2.png"))), 2);
			m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"priest\\priest_diffuse_1.png"))), 3);
			break;
	}
}

void PlayerRenderer::OnAnimationStateChange(const AnimationState& state)
{
	std::wstring jogPrefix = m_characterType == CharacterType::Warrior ? L"Zelda\\AnimationJog\\" : L"priest\\AnimationJog\\";

	switch (state)
	{
	case AnimationState::Idle:
		m_attackState = 0;
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_stand.yac")), true);
		break;
	case AnimationState::RunForward:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(jogPrefix + L"jog_forward_fast.yac")), true);
		break;
	case AnimationState::RunBackward:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(jogPrefix + L"jog_backward_slow.yac")), true);
		break;
	case AnimationState::RunLeft:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(jogPrefix + L"jog_strafe_left.yac")), true);
		break;
	case AnimationState::RunRight:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(jogPrefix + L"jog_strafe_right.yac")), true);
		break;
	case AnimationState::RunLeftForward:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(jogPrefix + L"jog_forward_diagonal_left.yac")), true);
		break;
	case AnimationState::RunRightForward:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(jogPrefix + L"jog_forward_diagonal_right.yac")), true);
		break;
	case AnimationState::RunLeftBackward:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(jogPrefix + L"jog_backward_diagonal_left.yac")), true);
		break;
	case AnimationState::RunRightBackward:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(jogPrefix + L"jog_backward_diagonal_right.yac")), true);
		break;
	case AnimationState::Attack:
		switch (m_characterType)
		{
			case CharacterType::Warrior:
				switch (m_attackState)
				{
				case 0:
					m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_attack1.yac")), false, true);
					break;
				case 1:
					m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_attack2.yac")), false, true);
					break;
				case 2:
					m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_attack3.yac")), false, true);
					break;
				case 3:
					m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_attack4.yac")), false, true);
					break;
				}
				m_attackState = (m_attackState + 1) % 4;
				break;
			case CharacterType::Priest:
				switch (m_attackState)
				{
				case 0:
					m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"priest\\attack_1.yac")), false, true);
					break;
				case 1:
					m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"priest\\attack_2.yac")), false, true);
					break;
				case 2:
					m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"priest\\attack_3.yac")), false, true);
					break;
				}
				m_attackState = (m_attackState + 1) % 3;
				break;
		}
		*m_stateMachine->GetConditionRefBool("Attack") = false;
		break;
	case AnimationState::Hit:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_hit.yac")), false, true);
		*m_stateMachine->GetConditionRefBool("Hit") = false;
		break;
	case AnimationState::Death:
		m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_die.yac")), false);
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

void PlayerRenderer::SetPlayerWeapon(std::string_view weaponName)
{
	auto& scaleJson = GET_DATA(nlohmann::ordered_json, "Weapon", weaponName, "Scale");
	auto& rotationJson = GET_DATA(nlohmann::ordered_json, "Weapon", weaponName, "Rotation");
	auto& positionJson = GET_DATA(nlohmann::ordered_json, "Weapon", weaponName, "Position");

	auto toolRenderer = m_bodyObj->GetComponent<RiggedPropRenderer>();
	if (toolRenderer == nullptr)
	{
		toolRenderer = m_bodyObj->AddComponent<RiggedPropRenderer>();

		toolRenderer->SetMaterial(udsdx::Material(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"nprcolor.hlsl")), INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(GET_DATA(std::wstring, "Weapon", weaponName, "ModelDiffuse")))));
		toolRenderer->SetBoneName("Bip001 R Hand");
	}

	toolRenderer->SetMesh(INSTANCE(Resource)->Load<udsdx::Mesh>(RESOURCE_PATH(GET_DATA(std::wstring,"Weapon", weaponName, "Model"))));
	toolRenderer->SetPropLocalTransform(
		Matrix4x4::CreateScale(scaleJson["X"], scaleJson["Y"], scaleJson["Z"]) *
		Matrix4x4::CreateFromYawPitchRoll(rotationJson["Y"] * DEG2RAD, rotationJson["X"] * DEG2RAD, rotationJson["Z"] * DEG2RAD) *
		Matrix4x4::CreateTranslation(positionJson["X"], positionJson["Y"], positionJson["Z"])
		);
}
