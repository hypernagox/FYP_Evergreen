#include "pch.h"
#include "AuthenticPlayer.h"
#include "ServerObject.h"
#include "InputHandler.h"
#include "EntityMovement.h"
#include "PlayerRenderer.h"
#include "MovePacketSender.h"

void AuthenticPlayer::UpdatePlayerCamFpsMode(float deltaTime)
{
	Transform* camTrans = m_cameraAnchor->GetTransform();
	float mouse_dx = static_cast<float>(INSTANCE(Input)->GetMouseX());
	float mouse_dy = static_cast<float>(INSTANCE(Input)->GetMouseY());
	Vector3 delta = Vector3(mouse_dy, mouse_dx, 0.0f);
	m_cameraAngleAxis += delta * m_fCamSensivity;
	m_cameraAngleAxis.x = std::clamp(m_cameraAngleAxis.x, -89.0f, 89.0f);

	m_cameraAngleAxisSmooth = m_cameraAngleAxisSmooth + (m_cameraAngleAxis - m_cameraAngleAxisSmooth) * deltaTime * 16.0f;
	camTrans->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(m_cameraAngleAxisSmooth * DEG2RAD));
}

void AuthenticPlayer::MoveByView(const Vector3& vDelta)
{
	const float deltaTime = DT;
	Vector3 temp{};
	vDelta.Normalize(temp);
	m_vCurState.x += (int)temp.x;
	m_vCurState.y += (int)temp.y;
	m_vCurState.z += (int)temp.z;

	Vector3 vWorldDelta = Vector3::Transform(vDelta, Quaternion::CreateFromYawPitchRoll(Vector3(0.0f, m_cameraAngleAxis.y * DEG2RAD, 0.0f)));
	m_entityMovement->AddAcceleration(vWorldDelta);
	m_rendererBodyAngleY = std::lerp(m_rendererBodyAngleY, m_cameraAngleAxis.y, deltaTime * 8.0f);
}

void AuthenticPlayer::InitCamDirection()
{
	m_cameraAngleAxis = Vector3::Zero;
	m_cameraAngleAxisSmooth = Vector3::Zero;
	m_cameraAnchor->GetTransform()->SetLocalRotation(Quaternion::Identity);

	m_pCamera->GetSceneObject()->GetTransform()->SetLocalRotation(Quaternion::Identity);
	GetSceneObject()->GetTransform()->SetLocalRotation(Quaternion::Identity);
}

void AuthenticPlayer::UpdateCameraTransform(Transform* pCameraTransfrom, float deltaTime)
{
	Vector3 wv = GetSceneObject()->GetTransform()->GetLocalPosition() + m_cameraAnchor->GetTransform()->GetLocalPosition();
	const float fMaxDist = 5.0f;
	switch (m_curCamMode)
	{
	case 0:
		pCameraTransfrom->SetLocalPosition(Vector3::Zero);
		pCameraTransfrom->SetLocalRotation(Quaternion::Identity);
		break;
	case 1:
	{
		float target = -fMaxDist;
		pCameraTransfrom->SetLocalPosition(Vector3(0.0f, 0.0f, max(target, std::lerp(pCameraTransfrom->GetLocalPosition().z, target, deltaTime * 8.0f))));
		pCameraTransfrom->SetLocalRotation(Quaternion::Identity);
	}
	break;
	case 2:
	{
		float target = fMaxDist;
		pCameraTransfrom->SetLocalPosition(Vector3(0.0f, 0.0f, std::min(target, std::lerp(pCameraTransfrom->GetLocalPosition().z, target, deltaTime * 8.0f))));
		pCameraTransfrom->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(PI, 0.0f, 0.0f));
		break;
	}
	}
	float tParam = m_fMoveTime;
	float mParam = 0.04f;
	pCameraTransfrom->SetLocalPosition(Vector3(sin(tParam) * mParam, sin(tParam * 2.0f) * mParam, pCameraTransfrom->GetLocalPosition().z));
}

AuthenticPlayer::AuthenticPlayer(const std::shared_ptr<SceneObject>& object)
	: Component{ object }
{

	m_fpChangeCamMode[0] = [this]() noexcept {
		m_cameraObj->GetTransform()->SetLocalPosition(Vector3::Zero);
		m_cameraObj->GetTransform()->SetLocalRotation(Quaternion::Identity);
		};
	m_fpChangeCamMode[1] = [this]() noexcept {
		m_cameraObj->GetTransform()->SetLocalPosition(Vector3::Zero);
		m_cameraObj->GetTransform()->SetLocalRotation(Quaternion::Identity);
		};
	m_fpChangeCamMode[2] = [this]() noexcept {
		m_cameraObj->GetTransform()->SetLocalPosition(Vector3::Zero);
		m_cameraObj->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(0.0f, PI, 0.0f));
		};

	m_cameraAnchor = std::make_shared<SceneObject>();
	m_cameraAnchor->GetTransform()->SetLocalPosition(Vector3(0.0f, 1.7f, 0.0f));

	m_cameraObj = std::make_shared<SceneObject>();
	m_fpChangeCamMode[m_curCamMode]();

	m_pCamera = m_cameraObj->AddComponent<CameraPerspective>();
	m_pCamera->SetClearColor(Color(1.0f, 1.0f, 1.0f, 1.0f));

	Start();
}

AuthenticPlayer::~AuthenticPlayer()
{

}

void AuthenticPlayer::Start()
{
	InitCamDirection();

	auto sceneObject = GetSceneObject();
	sceneObject->AddChild(m_cameraAnchor);
	m_cameraAnchor->AddChild(m_cameraObj);

	const auto input_handler = sceneObject->GetComponent<InputHandler>();

	input_handler->AddKeyFunc(Keyboard::A, KEY_STATE::KEY_HOLD, &AuthenticPlayer::MoveByView, this, Vector3(-1.0f, 0.0f, 0.0f) * 100.0f);
	input_handler->AddKeyFunc(Keyboard::W, KEY_STATE::KEY_HOLD, &AuthenticPlayer::MoveByView, this, Vector3(0.0f, 0.0f, 1.0f) * 100.0f);
	input_handler->AddKeyFunc(Keyboard::S, KEY_STATE::KEY_HOLD, &AuthenticPlayer::MoveByView, this, Vector3(0.0f, 0.0f, -1.0f) * 100.0f);
	input_handler->AddKeyFunc(Keyboard::D, KEY_STATE::KEY_HOLD, &AuthenticPlayer::MoveByView, this, Vector3(1.0f, 0.0f, 0.0f) * 100.0f);

	m_playerRenderer = sceneObject->GetComponent<PlayerRenderer>();
	m_entityMovement = sceneObject->AddComponent<EntityMovement>();
	m_pServerObject = sceneObject->GetComponent<ServerObject>();
}

void AuthenticPlayer::Update(const Time& time, Scene& scene)
{
	const Transform* transform = GetSceneObject()->GetTransform();

	m_entityMovement->SetAcceleration(Vector3::Down * 40.0f);

	const Vector3Int vPrevState = m_vCurState;
	m_vCurState = {};

	if (INSTANCE(Input)->GetKey(Keyboard::E))
	{
		m_fovBase = max(m_fovBase + PIDIV4 * time.deltaTime, PIDIV2);
	}
	if (INSTANCE(Input)->GetKey(Keyboard::Space))
	{
		const Vector3 UP = Vector3::Up * 10.0f;
		if (m_bGround)
			m_entityMovement->AddAcceleration(UP);
		m_vCurState.y += (int)UP.y;
	}
	if (INSTANCE(Input)->GetKey(Keyboard::LeftShift))
	{
		MoveByView(Vector3::Down * 10.0f);
	}
	if (INSTANCE(Input)->GetKeyDown(Keyboard::F5))
	{
		m_curCamMode = (m_curCamMode + 1) % 3;
		m_fpChangeCamMode[m_curCamMode]();
	}

	const Vector3 velocity = m_entityMovement->GetVelocity();
	m_fMoveTime += Vector2(velocity.x, velocity.z).Length() * time.deltaTime;

	UpdatePlayerCamFpsMode(time.deltaTime);
	UpdateCameraTransform(m_cameraObj->GetTransform(), time.deltaTime);

	m_pCamera->SetFov(std::lerp(m_pCamera->GetFov(), m_fovBase * (INSTANCE(Input)->GetKey(Keyboard::LeftControl) ? 1.5f : 1.0f), time.deltaTime * 8.0f));

	float rotationFactor = Vector2(velocity.x, velocity.z).Length() * 0.15f * sin(m_fMoveTime * 1.5f) * PIDIV4;

	m_rendererBodyAngleY = std::clamp(m_rendererBodyAngleY, m_cameraAngleAxisSmooth.y - 30.0f, m_cameraAngleAxisSmooth.y + 30.0f);
	m_playerRenderer->SetRotation(Quaternion::CreateFromYawPitchRoll(m_rendererBodyAngleY * DEG2RAD + PI, 0.0f, 0.0f));

	const bool vec3int_equal = vPrevState.x == m_vCurState.x && vPrevState.y == m_vCurState.y && vPrevState.z == m_vCurState.z;

	auto sceneObject = GetSceneObject();
	const auto input_handler = sceneObject->GetComponent<InputHandler>();
	m_bSendFlag = (!vec3int_equal) || input_handler->IsKeyHit();


	// 무브패킷 센드 업데이트
	m_pServerObject->ServerCompUpdate<MovePacketSender>();
	//
	m_bSendFlag = false;
}

Vector3 AuthenticPlayer::GetPlayerLook() const noexcept
{
	return Vector3::Transform(Vector3::Forward, m_cameraAnchor->GetTransform()->GetLocalRotation());
}