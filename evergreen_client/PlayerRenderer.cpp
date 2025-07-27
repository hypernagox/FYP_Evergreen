#include "pch.h"
#include "PlayerRenderer.h"
#include "EntityMovement.h"
#include "ServerObject.h"
#include "AuthenticPlayer.h"
#include "../common/json.hpp"

void PlayerRenderer::OnInitialize()
{
	m_rendererObj = SceneObject::MakeShared();

	GetSceneObject()->AddChild(m_rendererObj);

	m_rendererObj->GetTransform()->SetLocalScale(Vector3::One * GET_DATA(float, "GlobalValues", "CharacterScale", "Value"));

	m_bodyObj = SceneObject::MakeShared();
	m_renderer = m_bodyObj->AddComponent<RiggedMeshRenderer>();

	m_stateMachine = std::make_unique<Common::StateMachine<AnimationState>>(AnimationState::Idle);
	m_stateMachine->AddOnStateChangeCallback([this](AnimationState from, AnimationState to) { this->OnAnimationStateChange(to); });
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Idle, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Attack, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);
	m_stateMachine->AddTransition<Common::TimerStateTransition<AnimationState>>(AnimationState::Attack, AnimationState::AttackEnd, 0.25f);
	m_stateMachine->AddTransition<Common::AnimationStateTransition<AnimationState>>(AnimationState::AttackEnd, AnimationState::Idle, m_renderer);
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::AttackEnd, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);

	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Idle, AnimationState::Dash, m_stateMachine->GetConditionRefBool("Dash"), true);
	m_stateMachine->AddTransition<Common::AnimationStateTransition<AnimationState>>(AnimationState::Dash, AnimationState::Idle, m_renderer);
	
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Idle, AnimationState::Hit, m_stateMachine->GetConditionRefBool("Hit"), true);
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Hit, AnimationState::Hit, m_stateMachine->GetConditionRefBool("Hit"), true);
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::HitEnd, AnimationState::Hit, m_stateMachine->GetConditionRefBool("Hit"), true);
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::Attack, AnimationState::Hit, m_stateMachine->GetConditionRefBool("Hit"), true);

	m_stateMachine->AddTransition<Common::TimerStateTransition<AnimationState>>(AnimationState::Hit, AnimationState::HitEnd, 0.25f);
	m_stateMachine->AddTransition<Common::AnimationStateTransition<AnimationState>>(AnimationState::HitEnd, AnimationState::Idle, m_renderer);
	m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(AnimationState::HitEnd, AnimationState::Attack, m_stateMachine->GetConditionRefBool("Attack"), true);

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
		m_stateMachine->AddTransition<Common::BoolStateTransition<AnimationState>>(runState, AnimationState::Dash, m_stateMachine->GetConditionRefBool("Dash"), true);
		m_stateMachine->AddTransition<Common::FloatStateTransition<AnimationState, std::less_equal<float>>>(runState, AnimationState::Idle, m_stateMachine->GetConditionRefFloat("MoveSpeed"), 0.0f);
	}

	m_stateMachine->AddTransition<Common::AnimationStateTransition<AnimationState>>(AnimationState::Death, AnimationState::Idle, m_renderer);

	m_stateMachine->SetStateUpdateFp(AnimationState::Attack, [this]() {
		static bool flags[4];
		});
}

void PlayerRenderer::InitializeWarrior()
{
	m_renderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"Zelda\\zelda.yrms")));

	m_transformBody = m_bodyObj->GetTransform();
	m_rendererObj->AddChild(m_bodyObj);

	m_transformBody->SetLocalPositionY(-5.5f);

	m_characterType = CharacterType::Warrior;
	OnAnimationStateChange(AnimationState::Idle);

	SetPlayerWeapon(DATA_TABLE->GetWeaponIDInt(GET_DATA(std::string, "Player", "Warrior", "InitWeaponKey")));
	SetPlayerArmor(DATA_TABLE->GetArmorIDInt(GET_DATA(std::string, "Player", "Warrior", "InitArmorKey")));
}

void PlayerRenderer::InitializePriest()
{
	m_renderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"priest\\priest.yrms")));

	m_transformBody = m_bodyObj->GetTransform();
	m_rendererObj->AddChild(m_bodyObj);

	m_transformBody->SetLocalPositionY(-5.5f);

	m_characterType = CharacterType::Priest;
	OnAnimationStateChange(AnimationState::Idle);

	SetPlayerWeapon(DATA_TABLE->GetWeaponIDInt(GET_DATA(std::string, "Player", "Priest", "InitWeaponKey")));
	SetPlayerArmor(DATA_TABLE->GetArmorIDInt(GET_DATA(std::string, "Player", "Priest", "InitArmorKey")));
}

void PlayerRenderer::InitializeArcher()
{
	m_renderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"archer\\archer.yrms")));

	m_transformBody = m_bodyObj->GetTransform();
	m_rendererObj->AddChild(m_bodyObj);

	m_rendererObj->GetTransform()->SetLocalScale(m_rendererObj->GetTransform()->GetLocalScale() * 2.0f);

	m_transformBody->SetLocalPositionY(-3.0f);

	m_characterType = CharacterType::Archer;
	OnAnimationStateChange(AnimationState::Idle);

	SetPlayerWeapon(DATA_TABLE->GetWeaponIDInt(GET_DATA(std::string, "Player", "Archer", "InitWeaponKey")));
	SetPlayerArmor(DATA_TABLE->GetArmorIDInt(GET_DATA(std::string, "Player", "Archer", "InitArmorKey")));
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

	UpdateViewDirection(time.deltaTime);
}

void PlayerRenderer::SetViewDirection(float yaw, float pitch)
{
	Vector3 rendererAxis = m_rendererObj->GetTransform()->GetLocalRotation().ToEuler();
	m_viewYawTarget = LerpAngleRadian(0.0f, yaw * DEG2RAD - rendererAxis.y + PI, 0.4f);
	m_viewPitchTarget = LerpAngleRadian(0.0f, pitch * DEG2RAD - rendererAxis.x, 0.4f);
}

void PlayerRenderer::UpdateViewDirection(float deltaTime)
{
	m_viewYaw = LerpAngleRadian(m_viewYaw, m_viewYawTarget, deltaTime * 8.0f);
	m_viewPitch = LerpAngleRadian(m_viewPitch, m_viewPitchTarget, deltaTime * 8.0f);

	{

		Quaternion rotation = Quaternion::CreateFromYawPitchRoll(0.0f, m_viewYaw, m_viewPitch);
		rotation = Quaternion::Slerp(Quaternion::Identity, rotation, 0.25f);
		Matrix4x4 rotationMatrix = Matrix4x4::CreateFromQuaternion(rotation);
		m_renderer->SetBoneModifier("Bip001 Spine1", rotationMatrix);
		m_renderer->SetBoneModifier("Bip001 Spine2", rotationMatrix);
		m_renderer->SetBoneModifier("Bip001 Neck", rotationMatrix);
		m_renderer->SetBoneModifier("Bip001 Head", rotationMatrix);
	}
	{
		Quaternion rotation = Quaternion::CreateFromYawPitchRoll(m_viewYaw, -m_viewPitch, 0.0f);
		rotation = Quaternion::Slerp(Quaternion::Identity, rotation, 0.25f);
		Matrix4x4 rotationMatrix = Matrix4x4::CreateFromQuaternion(rotation);
		m_renderer->SetBoneModifier("mixamorig:Spine1", rotationMatrix);
		m_renderer->SetBoneModifier("mixamorig:Spine2", rotationMatrix);
		m_renderer->SetBoneModifier("mixamorig:Neck", rotationMatrix);
		m_renderer->SetBoneModifier("mixamorig:Head", rotationMatrix);
	}
}

void PlayerRenderer::OnAnimationStateChange(const AnimationState& state)
{
	std::wstring jogPrefix;
	switch (m_characterType)
	{
	case CharacterType::Warrior:
		jogPrefix = L"Zelda\\AnimationJog\\";
		break;
	case CharacterType::Priest:
		jogPrefix = L"priest\\AnimationJog\\";
		break;
	case CharacterType::Archer:
		jogPrefix = L"archer\\AnimationJog\\";
		break;
	}

	switch (state)
	{
	case AnimationState::Idle:
		m_attackState = 0;
		if (m_characterType == CharacterType::Archer)
			m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"archer\\AnimationJog\\archer_idle.yac")), true);
		else
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
				m_attackState = (m_attackState + 1) % 2;
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
			case CharacterType::Archer:
				m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"archer\\AnimationJog\\archer_attack.yac")), false, true);
				break;
		}
		*m_stateMachine->GetConditionRefBool("Attack") = false;
		break;
	case AnimationState::Dash:
		if (m_characterType == CharacterType::Archer)
			m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"archer\\AnimationJog\\archer_dash.yac")), false, true);
		else
			m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_slide.yac")), false, true);
		*m_stateMachine->GetConditionRefBool("Dash") = false;
		break;
	case AnimationState::Hit:
		if (m_characterType == CharacterType::Archer)
			m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"archer\\AnimationJog\\archer_react.yac")), false, true);
		else
			m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_hit.yac")), false, true);
		*m_stateMachine->GetConditionRefBool("Hit") = false;
		break;
	case AnimationState::Death:
		if (m_characterType == CharacterType::Archer)
			m_renderer->SetAnimation(INSTANCE(Resource)->Load<udsdx::AnimationClip>(RESOURCE_PATH(L"archer\\AnimationJog\\archer_death.yac")), false);
		else
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

void PlayerRenderer::SetPlayerWeapon(int weaponID)
{
	const std::string& weaponName = DATA_TABLE->GetWeaponIDStr(weaponID);

	auto& scaleJson = GET_DATA(nlohmann::ordered_json, "Weapon", weaponName, "Scale");
	auto& rotationJson = GET_DATA(nlohmann::ordered_json, "Weapon", weaponName, "Rotation");
	auto& positionJson = GET_DATA(nlohmann::ordered_json, "Weapon", weaponName, "Position");

	auto toolRenderer = m_bodyObj->GetComponent<RiggedPropRenderer>();
	if (toolRenderer == nullptr)
	{
		toolRenderer = m_bodyObj->AddComponent<RiggedPropRenderer>();
		if (m_characterType == CharacterType::Archer)
			toolRenderer->SetBoneName("mixamorig:LeftHand");
		else
			toolRenderer->SetBoneName("Bip001 R Hand");
	}

	toolRenderer->SetMesh(INSTANCE(Resource)->Load<udsdx::Mesh>(RESOURCE_PATH(GET_DATA(std::wstring, "Weapon", weaponName, "Model"))));
	toolRenderer->SetMaterial(udsdx::Material(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"nprcolor.hlsl")), INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(GET_DATA(std::wstring, "Weapon", weaponName, "ModelDiffuse")))));

	Vector3 scale = Vector3(scaleJson["X"], scaleJson["Y"], scaleJson["Z"]);
	Quaternion rotation = Quaternion::CreateFromYawPitchRoll(rotationJson["Y"] * DEG2RAD, rotationJson["X"] * DEG2RAD, rotationJson["Z"] * DEG2RAD);
	Vector3 position = Vector3(positionJson["X"], positionJson["Y"], positionJson["Z"]);
	Quaternion postRotation = Quaternion::Identity;

	if (m_characterType == CharacterType::Archer)
	{
		scale *= 0.5f;
		postRotation = Quaternion::CreateFromYawPitchRoll(PIDIV2, 0.0f, PIDIV2);
		position *= 0.5f;
		position += Vector3(1.0f, -1.0f, 0.0f);
	}

	toolRenderer->SetPropLocalTransform(Matrix4x4::CreateScale(scale) *	Matrix4x4::CreateFromQuaternion(rotation) *	Matrix4x4::CreateTranslation(position) * Matrix4x4::CreateFromQuaternion(postRotation));
}

void PlayerRenderer::SetPlayerArmor(int armorID)
{
	std::string armorName = DATA_TABLE->GetArmorIDStr(armorID);
	std::wstring texturePostfix = GET_DATA(std::wstring, "Armor", armorName, "TexturePostfix");

	Shader* shader = INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"nprcolor.hlsl"));

	switch (m_characterType)
	{
	case CharacterType::Warrior:
		m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_body_BaseColor" + texturePostfix + L".png"))), 0);
		m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_hair_BaseColor.png"))), 1);
		m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_eye_BaseColor.png"))), 2);
		m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_face_BaseColor.png"))), 3);
		break;
	case CharacterType::Priest:
		m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"priest\\priest_diffuse_2" + texturePostfix + L".png"))), 0);
		m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"priest\\priest_diffuse_0" + texturePostfix + L".png"))), 1);
		m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"priest\\priest_diffuse_2" + texturePostfix + L".png"))), 2);
		m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"priest\\priest_diffuse_1.png"))), 3);
		break;
	case CharacterType::Archer:
		m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"archer\\char_1.png"))), 1);
		m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"archer\\char_2.png"))), 0);
		m_renderer->SetMaterial(udsdx::Material(shader, INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"archer\\char_3.png"))), 2);
		break;
	}
}
