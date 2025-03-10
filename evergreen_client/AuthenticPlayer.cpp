#include "pch.h"
#include "AuthenticPlayer.h"
#include "ServerObject.h"
#include "InputHandler.h"
#include "EntityMovement.h"
#include "PlayerRenderer.h"
#include "MovePacketSender.h"
#include "NaviAgent.h"


bool IsWithinDistance(const DirectX::SimpleMath::Vector3& currentPosition,
	const DirectX::SimpleMath::Vector3& targetPosition,
	const float distance)
{
	const float distanceSquared = (currentPosition - targetPosition).LengthSquared();

	const float maxDistanceSquared = distance * distance;

	return distanceSquared <= maxDistanceSquared;
}

void AuthenticPlayer::UpdatePlayerCamFpsMode(float deltaTime)
{
	Transform* camTrans = m_cameraAnchor->GetTransform();

	if (INSTANCE(Input)->GetMouseMode() == Mouse::Mode::MODE_RELATIVE)
	{
		float mouse_dx = static_cast<float>(INSTANCE(Input)->GetMouseX());
		float mouse_dy = static_cast<float>(INSTANCE(Input)->GetMouseY());
		Vector3 delta = Vector3(mouse_dy, mouse_dx, 0.0f);
		m_cameraAngleAxis += delta * m_fCamSensivity;
		m_cameraAngleAxis.x = std::clamp(m_cameraAngleAxis.x, -89.0f, 89.0f);
	}

	m_cameraAngleAxisSmooth = Vector3::Lerp(m_cameraAngleAxisSmooth, m_cameraAngleAxis, std::min(deltaTime * 16.0f, 1.0f));
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
	
	//TODO: 매직넘버
	if (m_entityMovement->GetVelocity().LengthSquared() == 0.f && 0.f == m_entityMovement->GetAcceleration().LengthSquared())
		m_entityMovement->AddVelocity(vWorldDelta *= .1f);
	else
		m_entityMovement->AddAcceleration(vWorldDelta);
	m_rendererBodyAngleY = std::lerp(m_rendererBodyAngleY, m_cameraAngleAxis.y, deltaTime * 8.0f);
}

void AuthenticPlayer::InitCamDirection()
{
	m_cameraAngleAxis = Vector3::Zero;
	m_cameraAngleAxisSmooth = Vector3::Zero;

	m_pCamera->GetSceneObject()->GetTransform()->SetLocalRotation(Quaternion::Identity);
	GetSceneObject()->GetTransform()->SetLocalRotation(Quaternion::Identity);
}

void AuthenticPlayer::UpdateCameraTransform(Transform* pCameraTransfrom, float deltaTime)
{
	Vector3 wv = GetSceneObject()->GetTransform()->GetLocalPosition() + m_cameraAnchor->GetTransform()->GetLocalPosition();
	const float fMaxDist = 3.0f;
	float target = -fMaxDist;
	float zPos = std::max(target, std::lerp(pCameraTransfrom->GetLocalPosition().z, target, deltaTime * 8.0f));
	float tParam = m_fMoveTime * 0.5f;
	float mParam = 0.04f;
	pCameraTransfrom->SetLocalPosition(Vector3(1.5f + sin(tParam) * mParam, sin(tParam * 2.0f) * mParam, zPos));
	pCameraTransfrom->SetLocalRotation(Quaternion::Identity);
	m_pCamera->SetFov(std::lerp(m_pCamera->GetFov(), m_fovBase + (INSTANCE(Input)->GetMouseRightButton() ? -30.0f * DEG2RAD : 0.0f), deltaTime * 8.0f));
}

void AuthenticPlayer::FireProj()
{
	if constexpr (g_bUseNetWork)
	{
		const auto rad = CommonMath::GetYawFromQuaternion(m_cameraAnchor->GetTransform()->GetLocalRotation());
		m_bSendFlag = true;
		Send(
			Create_c2s_FIRE_PROJ(ToFlatVec3(GetSceneObject()->GetTransform()->GetLocalPosition()), rad)
		);
	}
}

void AuthenticPlayer::DoAttack()
{
	if constexpr (g_bUseNetWork)
	{
		const auto rad = CommonMath::GetYawFromQuaternion(m_cameraAnchor->GetTransform()->GetLocalRotation());
		m_bSendFlag = true;
		Send(
			Create_c2s_PLAYER_ATTACK(rad, ToFlatVec3(GetSceneObject()->GetTransform()->GetLocalPosition()))
		);
	}
}

void AuthenticPlayer::RequestQuest()
{
	const auto t = GetSceneObject()->GetTransform();
	if (IsWithinDistance(t->GetLocalPosition(), { 0,0,0 }, 10.f))
	{	
		Send(Create_c2s_REQUEST_QUEST(0));
	}
}

AuthenticPlayer::AuthenticPlayer(const std::shared_ptr<SceneObject>& object)
	: Component{ object }
{
	m_cameraAnchor = std::make_shared<SceneObject>();
	m_cameraAnchor->GetTransform()->SetLocalPosition(Vector3(0.0f, 3.0f, 0.0f));

	m_cameraObj = std::make_shared<SceneObject>();

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

	const auto input_handler = AddComponent<InputHandler>();

	input_handler->AddKeyFunc(Keyboard::A, KEY_STATE::KEY_HOLD, &AuthenticPlayer::MoveByView, this, Vector3(-1.0f, 0.0f, 0.0f) * 100.0f);
	input_handler->AddKeyFunc(Keyboard::W, KEY_STATE::KEY_HOLD, &AuthenticPlayer::MoveByView, this, Vector3(0.0f, 0.0f, 1.0f) * 100.0f);
	input_handler->AddKeyFunc(Keyboard::S, KEY_STATE::KEY_HOLD, &AuthenticPlayer::MoveByView, this, Vector3(0.0f, 0.0f, -1.0f) * 100.0f);
	input_handler->AddKeyFunc(Keyboard::D, KEY_STATE::KEY_HOLD, &AuthenticPlayer::MoveByView, this, Vector3(1.0f, 0.0f, 0.0f) * 100.0f);

	//input_handler->AddKeyFunc(Keyboard::Space, KEY_STATE::KET_TAP, &AuthenticPlayer::DoAttack, this);
	input_handler->AddKeyFunc(Keyboard::CapsLock, KEY_STATE::KET_TAP, &AuthenticPlayer::RequestQuest, this);

	m_pServerObject = GetComponent<ServerObject>();
	m_entityMovement = AddComponent<EntityMovement>();
	m_playerRenderer = AddComponent<PlayerRenderer>();

	m_entityMovement->SetFriction(40.0f);
}

void AuthenticPlayer::Update(const Time& time, Scene& scene)
{
	Transform* transform = GetSceneObject()->GetTransform();

	m_entityMovement->SetAcceleration(Vector3::Zero);

	const Vector3Int vPrevState = m_vCurState;
	m_vCurState = {};

	if (INSTANCE(Input)->GetKey(Keyboard::Space))
	{
		//const Vector3 UP = Vector3::Up * 10.0f;
		//if (m_bGround)
		//	m_entityMovement->AddAcceleration(UP);
		//m_vCurState.y += (int)UP.y;
	}
	if (INSTANCE(Input)->GetKey(Keyboard::LeftShift))
	{
		MoveByView(Vector3::Down * 10.0f);
	}

	// TODO 하드코딩
	//static float cool_down = 0.f;
	//cool_down -= DT;
	//if (INSTANCE(Input)->GetMouseLeftButtonDown() && 0.f >= cool_down)
	//{
	//	cool_down = 1.f;
	//	DoAttack();
	//}

	const Vector3 velocity = m_entityMovement->GetVelocity();
	m_fMoveTime += Vector2(velocity.x, velocity.z).Length() * time.deltaTime;

	UpdatePlayerCamFpsMode(time.deltaTime);
	UpdateCameraTransform(m_cameraObj->GetTransform(), time.deltaTime);

	float rotationFactor = Vector2(velocity.x, velocity.z).Length() * 0.15f * sin(m_fMoveTime * 1.5f) * PIDIV4;

	// m_rendererBodyAngleY = std::clamp(m_rendererBodyAngleY, m_cameraAngleAxisSmooth.y - 30.0f, m_cameraAngleAxisSmooth.y + 30.0f);
	m_playerRenderer->SetRotation(Quaternion::CreateFromYawPitchRoll(m_rendererBodyAngleY * DEG2RAD + PI, 0.0f, 0.0f));

	const bool vec3int_equal = vPrevState.x == m_vCurState.x && vPrevState.y == m_vCurState.y && vPrevState.z == m_vCurState.z;

	auto sceneObject = GetSceneObject();
	const auto input_handler = sceneObject->GetComponent<InputHandler>();
	m_bSendFlag = input_handler->IsKeyHit();

	const auto navi = GetSceneObject()->GetComponent<ServerObject>()->m_pNaviAgent;
	const auto prev_pos = GetSceneObject()->GetComponent<EntityMovement>()->prev_pos;
	Vector3 temp;
	navi->SetCellPos(prev_pos, transform->GetLocalPosition(), temp);
	transform->SetLocalPosition(temp);
	GetSceneObject()->GetComponent<EntityMovement>()->prev_pos = temp;

	if (INSTANCE(Input)->GetMouseLeftButtonDown())
	{
		const auto state = m_playerRenderer->GetCurrentState();
		if (PlayerRenderer::AnimationState::Idle == state || PlayerRenderer::AnimationState::Run == state || PlayerRenderer::AnimationState::Attack == state)
		{
			m_playerRenderer->Attack();
			DoAttack();
			//std::cout << "공격 시도\n";
		}
	}
	if (INSTANCE(Input)->GetMouseRightButtonDown())
	{
		const auto state = m_playerRenderer->GetCurrentState();
		if (PlayerRenderer::AnimationState::Idle == state || PlayerRenderer::AnimationState::Run == state)
		{
			// m_playerRenderer->Attack();
			FireProj();
			//std::cout << "공격 시도\n";
		}
	}

	//
	//// 무브패킷 센드 업데이트

	m_bSendFlag |=
		INSTANCE(Input)->GetKeyDown(Keyboard::W)
		|| INSTANCE(Input)->GetKeyDown(Keyboard::A)
		|| INSTANCE(Input)->GetKeyDown(Keyboard::S)
		|| INSTANCE(Input)->GetKeyDown(Keyboard::D);


	m_pServerObject->ServerCompUpdate<MovePacketSender>();
}

Vector3 AuthenticPlayer::GetPlayerLook() const noexcept
{
	return Vector3::Transform(Vector3::Forward, m_cameraAnchor->GetTransform()->GetLocalRotation());
}