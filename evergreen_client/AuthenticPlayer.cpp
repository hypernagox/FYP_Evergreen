#include "pch.h"
#include "AuthenticPlayer.h"
#include "ServerObject.h"
#include "InputHandler.h"
#include "EntityMovement.h"
#include "PlayerRenderer.h"
#include "MovePacketSender.h"
#include "NaviAgent.h"
#include "PlayerStatusGUI.h"
#include "PlayerQuickSlotGUI.h"
#include "PlayerInventoryGUI.h"
#include "PlayerCraftGUI.h"
#include "Navigator.h"
#include "GizmoSphereRenderer.h"
#include "GuideSystem.h"
#include "HeightMap.h"

AuthenticPlayer::AuthenticPlayer(const std::shared_ptr<SceneObject>& object)
	: Component{ object }
{
	m_cameraAnchor = std::make_shared<SceneObject>();
	m_cameraObj = std::make_shared<SceneObject>();

	m_pCamera = m_cameraObj->AddComponent<CameraPerspective>();
	m_pCamera->SetClearColor(Color(1.0f, 1.0f, 1.0f, 1.0f));

	// TODO: 아이템 Json 레지스트리에서 종류의 개수를 알아내서 가져오면 좋겠다.
	m_inventory = std::vector<int>(16, 0);
	m_quickSlot = std::vector<int>(MAX_QUICK_SLOT, -1);

	m_cameraAnchorLastPosition = Vector3::Up * 120.0f;

	Start();
}

AuthenticPlayer::~AuthenticPlayer()
{

}

bool IsWithinDistance(const DirectX::SimpleMath::Vector3& currentPosition,
	const DirectX::SimpleMath::Vector3& targetPosition,
	const float distance)
{
	const float distanceSquared = (currentPosition - targetPosition).LengthSquared();

	const float maxDistanceSquared = distance * distance;

	return distanceSquared <= maxDistanceSquared;
}

void AuthenticPlayer::MoveByView(const Vector3& vDelta)
{
	// TODO: 캐릭터의 전반적인 상태 관리 머신에 따른 행동 제어 필요
	if (m_playerRenderer->GetCurrentState() == PlayerRenderer::AnimationState::Attack)
		return;

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
}

void AuthenticPlayer::SetPlayerStatusGUI(PlayerStatusGUI* playerStatusGUI) noexcept
{
	m_playerStatusGUI = playerStatusGUI;

	if (m_playerStatusGUI)
	{
		m_playerStatusGUI->SetMaxHealth(m_iMaxHP);
		m_playerStatusGUI->SetCurrentHealth(m_iCurHP);
	}
}

void AuthenticPlayer::SetPlayerQuickSlotGUI(PlayerQuickSlotGUI* playerQuickSlotGUI) noexcept
{
	m_playerQuickSlotGUI = playerQuickSlotGUI;
}

void AuthenticPlayer::SetPlayerInventoryGUI(PlayerInventoryGUI* playerInventoryGUI) noexcept
{
	m_playerInventoryGUI = playerInventoryGUI;
}

void AuthenticPlayer::SetPlayerCraftGUI(PlayerCraftGUI* playerCraftGUI) noexcept
{
	m_playerCraftGUI = playerCraftGUI;
}

void AuthenticPlayer::OnHit(int afterHP)
{
	m_iCurHP = afterHP;
	if (m_iCurHP <= 0)
		m_iCurHP = m_iMaxHP;
	if (m_playerStatusGUI)
		m_playerStatusGUI->SetCurrentHealth(m_iCurHP);
}

void AuthenticPlayer::OnModifyInventory(uint8_t itemID, int delta)
{
	m_inventory[itemID] += delta;
	m_playerQuickSlotGUI->UpdateSlotContents(m_quickSlot, m_inventory);
	m_playerInventoryGUI->UpdateSlotContents(this, m_inventory);
	m_playerCraftGUI->UpdateSlotContents(this, m_inventory);
}

void AuthenticPlayer::ToggleDebugCamera()
{
	m_bDebugCamera = !m_bDebugCamera;
}

void AuthenticPlayer::SetQuickSlotItemOnBlank(uint8_t itemID)
{
	for (int i = 0; i < m_quickSlot.size(); ++i)
	{
		if (m_quickSlot[i] == -1 || m_quickSlot[i] == itemID)
		{
			SetQuickSlotItem(i, itemID);
			break;
		}
	}
}

void AuthenticPlayer::SetQuickSlotItem(int index, uint8_t itemID)
{
	// 클라이언트 GUI에게 해당 퀵슬롯에 대한 아이템을 설정한다.
	m_quickSlot[index] = itemID;
	m_playerQuickSlotGUI->UpdateSlotContents(m_quickSlot, m_inventory);
	Send(
		Create_c2s_REQUEST_QUICK_SLOT(itemID, (uint8_t)index)
	);
	// TODO: 클라이언트가 서버에게 해당 퀵슬롯에 대한 아이템 설정을 요청한다.
}

void AuthenticPlayer::UseQuickSlotItem(int index)
{
	// TODO: 클라이언트가 서버에게 해당 퀵슬롯에 대한 아이템 사용을 요청한다.
	Send(
		Create_c2s_USE_QUICK_SLOT_ITEM((uint8_t)index)
	);
}

void AuthenticPlayer::CraftItem(int recipeIndex)
{
	// TODO: 아이템을 조합하는 패킷을 서버에 전송
	Send(Create_c2s_CRAFT_ITEM(recipeIndex));
	DebugConsole::Log("아이템 조합 요청");
}

void AuthenticPlayer::RequestQuestStart()
{
	Send(Create_c2s_QUEST_START());
}

void AuthenticPlayer::RequestQuestEnd()
{
	Send(Create_c2s_QUEST_END());
}

void AuthenticPlayer::UpdateCameraTransform(Transform* pCameraTransfrom, float deltaTime)
{
	// Region: Camera Rotation Control
	m_cameraAngleAxisSmooth = Vector3::Lerp(m_cameraAngleAxisSmooth, m_cameraAngleAxis, std::min(deltaTime * 16.0f, 1.0f));
	m_cameraAnchor->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(m_cameraAngleAxisSmooth * DEG2RAD));

	// Region: Mouse Scrolling Control
	int mouseScroll = INSTANCE(Input)->GetMouseScroll();
	if (m_lastMouseScroll != mouseScroll)
	{
		m_cameraDistance += static_cast<float>(mouseScroll - m_lastMouseScroll) * -0.01f;
		m_cameraDistance = std::clamp(m_cameraDistance, 0.5f, 10.0f);
	}
	m_lastMouseScroll = mouseScroll;

	// Region: Camera Anchor Position Control
	Vector3 playerPosition = GetSceneObject()->GetTransform()->GetLocalPosition();
	Vector3 anchorPosition = playerPosition + Vector3::Up * 1.5f;
	Vector3 interpolatedAnchorPosition = Vector3::Lerp(m_cameraAnchorLastPosition, anchorPosition, deltaTime * 8.0f);
	m_cameraAnchor->GetTransform()->SetLocalPosition(interpolatedAnchorPosition - playerPosition);
	m_cameraAnchorLastPosition = m_cameraAnchor->GetTransform()->GetWorldPosition();

	// Region: Camera Z Distance / Screen Offset Control
	bool focused = INSTANCE(Input)->GetMouseRightButton();
	float distanceTarget = focused ? 1.0f : m_cameraDistance;

	m_cameraDistanceSmooth = std::lerp(m_cameraDistanceSmooth, -distanceTarget, deltaTime * 8.0f);
	float tParam = m_fMoveTime * 0.5f;
	float mParam = 0.04f;
	m_cameraXOffsetSmooth = std::lerp(m_cameraXOffsetSmooth, focused ? 0.25f : 0.0f, deltaTime * 8.0f);
	pCameraTransfrom->SetLocalPosition(Vector3(sin(tParam) * mParam + m_cameraXOffsetSmooth, sin(tParam * 2.0f) * mParam, m_cameraDistanceSmooth));

	// Region: Camera Position Postprocess
	if (m_heightMap)
	{
		Transform* pAnchorTransform = m_cameraAnchor->GetTransform();
		const float TerrainSize = GET_DATA(float, "TerrainSize", "Value");
		pAnchorTransform->ValidateMatrixRecursive();
		Matrix4x4 cameraWorldMatrix = pAnchorTransform->GetWorldSRTMatrix();
		Matrix4x4 terrainWorldMatrix = Matrix4x4::CreateScale(TerrainSize) * Matrix4x4::CreateTranslation(Vector3(-0.5f, 0.0f, -0.5f) * TerrainSize);
		Matrix4x4 cameraToTerrain = cameraWorldMatrix * terrainWorldMatrix.Invert();

		Vector3 terrainPos = Vector3::Transform(pCameraTransfrom->GetLocalPosition(), cameraToTerrain);
		float height = m_heightMap->GetHeight(terrainPos.x * m_heightMap->GetPixelWidth(), terrainPos.z * m_heightMap->GetPixelHeight());
		terrainPos.y = std::max(terrainPos.y, height + 0.1f / TerrainSize);
		pCameraTransfrom->SetLocalPosition(Vector3::Transform(terrainPos, cameraToTerrain.Invert()));
	}

	// Region: Camera FOV Control
	m_pCamera->SetFov(std::lerp(m_pCamera->GetFov(), focused ? 30.0f * DEG2RAD : m_fovBase, deltaTime * 8.0f));
}

void AuthenticPlayer::UpdateCameraTransformDebug(Transform* pCameraTransfrom, float deltaTime)
{
	m_cameraAnchor->GetTransform()->SetLocalPosition(Vector3::Zero);
	m_cameraAnchor->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(0.0f, PIDIV2, 0.0f));
	m_cameraObj->GetTransform()->SetLocalPosition(Vector3::Forward * 64.0f);
}

void AuthenticPlayer::TryClickScreen()
{
	m_playerRenderer->Attack();
	DoAttack();
	//std::cout << "공격 시도\n";
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
	input_handler->AddKeyFunc(Keyboard::D1, KEY_STATE::KET_TAP, &AuthenticPlayer::UseQuickSlotItem, this, 0);
	input_handler->AddKeyFunc(Keyboard::D2, KEY_STATE::KET_TAP, &AuthenticPlayer::UseQuickSlotItem, this, 1);
	input_handler->AddKeyFunc(Keyboard::D3, KEY_STATE::KET_TAP, &AuthenticPlayer::UseQuickSlotItem, this, 2);
	input_handler->AddKeyFunc(Keyboard::D0, KEY_STATE::KET_TAP, &AuthenticPlayer::ToggleDebugCamera, this);

	// 퀘 시작 요청
	input_handler->AddKeyFunc(Keyboard::B, KEY_STATE::KET_TAP, &AuthenticPlayer::RequestQuestStart, this);
	// TODO: 퀘 클리어 판정이 아직 없어서 클라의 중단 요청
	input_handler->AddKeyFunc(Keyboard::N, KEY_STATE::KET_TAP, &AuthenticPlayer::RequestQuestEnd, this);


	//input_handler->AddKeyFunc(Keyboard::Space, KEY_STATE::KET_TAP, &AuthenticPlayer::DoAttack, this);
	input_handler->AddKeyFunc(Keyboard::CapsLock, KEY_STATE::KET_TAP, &AuthenticPlayer::RequestQuest, this);

	m_entityMovement = AddComponent<EntityMovement>();
	m_playerRenderer = AddComponent<PlayerRenderer>();
	m_pServerObject = AddComponent<ServerObject>();

	m_entityMovement->SetFriction(40.0f);
}

void AuthenticPlayer::Update(const Time& time, Scene& scene)
{
	Transform* transform = GetSceneObject()->GetTransform();

	Quaternion playerRotation = Quaternion::CreateFromYawPitchRoll(m_rendererBodyAngleY * DEG2RAD + PI, 0.0f, 0.0f);
	m_playerRenderer->SetRotation(playerRotation);
	m_entityMovement->SetForward(Vector3::Transform(Vector3::Forward, playerRotation));
	m_entityMovement->SetAcceleration(Vector3::Zero);

	const Vector3Int vPrevState = m_vCurState;
	m_vCurState = {};

	const Vector3 velocity = m_entityMovement->GetVelocity();
	m_fMoveTime += Vector2(velocity.x, velocity.z).Length() * time.deltaTime;

	if (INSTANCE(Input)->GetMouseMode() == Mouse::Mode::MODE_RELATIVE)
	{
		float mouse_dx = static_cast<float>(INSTANCE(Input)->GetMouseX());
		float mouse_dy = static_cast<float>(INSTANCE(Input)->GetMouseY());
		Vector3 delta = Vector3(mouse_dy, mouse_dx, 0.0f);
		m_cameraAngleAxis += delta * m_fCamSensivity;
		m_cameraAngleAxis.x = std::clamp(m_cameraAngleAxis.x, -80.0f, 80.0f);
	}

	if (m_bDebugCamera)
		UpdateCameraTransformDebug(m_cameraObj->GetTransform(), time.deltaTime);
	else
		UpdateCameraTransform(m_cameraObj->GetTransform(), time.deltaTime);

	const bool vec3int_equal = vPrevState.x == m_vCurState.x && vPrevState.y == m_vCurState.y && vPrevState.z == m_vCurState.z;

	auto sceneObject = GetSceneObject();
	const auto input_handler = sceneObject->GetComponent<InputHandler>();
	m_bSendFlag = input_handler->IsKeyHit();

	const auto navi = GetSceneObject()->GetComponent<ServerObject>()->m_pNaviAgent;
	const auto prev_pos = GetSceneObject()->GetComponent<EntityMovement>()->prev_pos;
	Vector3 temp;
	navi->SetCellPos(DT, prev_pos, transform->GetLocalPosition(), temp);
	transform->SetLocalPosition(temp);
	GetSceneObject()->GetComponent<EntityMovement>()->prev_pos = temp;

	if (INSTANCE(Input)->GetMouseRightButtonDown())
	{
		const auto state = m_playerRenderer->GetCurrentState();
		if (PlayerRenderer::AnimationState::Attack != state)
		{
			// m_playerRenderer->Attack();
			FireProj();
			//std::cout << "공격 시도\n";
		}
	}

	if (INSTANCE(Input)->GetKeyDown(Keyboard::Space))
	{
		Send(Create_c2s_CHANGE_HARVEST_STATE());
	}

	if (INSTANCE(Input)->GetKeyDown(Keyboard::P))
	{
		const auto pos = GetSceneObject()->GetTransform()->GetLocalPosition();
		std::cout << std::format("x: {}, y: {}, z: {} \n", pos.x, pos.y, pos.z);
		
		const Vector3 end = { -119.499115f,75,13.64f }; // 마을 중앙
		GuideSystem::GetInst()->ToggleFlag();
	}
	if (INSTANCE(Input)->GetKeyDown(Keyboard::K))
	{
		GuideSystem::GetInst()->ToggleFlag();
		if (GuideSystem::GetInst()->temp_force_pos == Vector3::Zero)
			GuideSystem::GetInst()->temp_force_pos = { -119.499115f,75,13.64f };
		else
			GuideSystem::GetInst()->temp_force_pos = Vector3::Zero;
	}
	GuideSystem::GetInst()->UpdateGuideSystem();
	// 무브패킷 센드 업데이트
	m_bSendFlag |=
		INSTANCE(Input)->GetKeyDown(Keyboard::W)
		|| INSTANCE(Input)->GetKeyDown(Keyboard::A)
		|| INSTANCE(Input)->GetKeyDown(Keyboard::S)
		|| INSTANCE(Input)->GetKeyDown(Keyboard::D);
}

Vector3 AuthenticPlayer::GetPlayerLook() const noexcept
{
	return Vector3::Transform(Vector3::Forward, m_cameraAnchor->GetTransform()->GetLocalRotation());
}