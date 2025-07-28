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
#include "PlayerEquipmentGUI.h"
#include "PlayerCraftGUI.h"
#include "Navigator.h"
#include "GizmoSphereRenderer.h"
#include "GameGUIFacade.h"
#include "LogFloatGUI.h"
#include "GuideSystem.h"
#include "HeightMap.h"
#include "TransitionOverlayGUI.h"
#include "EnvironmentRenderer.h"
#include "CommonQuestTable.h"
#include "ForcedMovement.h"

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
	if (m_playerRenderer->GetCurrentState() == PlayerRenderer::AnimationState::Attack || m_playerRenderer->GetCurrentState() == PlayerRenderer::AnimationState::AttackEnd)
		return;
	// 죽거나 사망이면 이동못함
	if (PlayerRenderer::AnimationState::Hit == m_playerRenderer->GetCurrentState())
		return;
	if (PlayerRenderer::AnimationState::Death == m_playerRenderer->GetCurrentState())
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
		m_entityMovement->AddVelocity(vWorldDelta * 0.1f);
	else
		m_entityMovement->AddAcceleration(vWorldDelta);
	
	m_rendererBodyAngleY = udsdx::LerpAngle(m_rendererBodyAngleY, m_cameraAngleAxis.y, deltaTime * 8.0f);
}

void AuthenticPlayer::FixCameraAnchor()
{
	m_cameraAnchor->GetTransform()->SetLocalPosition(Vector3::Up * 1.5f);
	m_cameraAnchorLastPosition = m_cameraAnchor->GetTransform()->GetWorldPosition();
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

void AuthenticPlayer::SetPlayerEquipmentGUI(PlayerEquipmentGUI* playerEquipmentGUI) noexcept
{
	m_playerEquipmentGUI = playerEquipmentGUI;
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
	m_playerRenderer->Hit();
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

void AuthenticPlayer::ToggleLogFloatGUI()
{
	INSTANCE(GameGUIFacade)->LogFloat->SetAlwaysVisible(!INSTANCE(GameGUIFacade)->LogFloat->GetAlwaysVisible());
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
	Send(Create_c2s_REQUEST_QUICK_SLOT(itemID, (uint8_t)index));
	// TODO: 클라이언트가 서버에게 해당 퀵슬롯에 대한 아이템 설정을 요청한다.
}

void AuthenticPlayer::SetPlayerWeapon(int weaponID, bool sendPacket)
{
	// TODO: 서버에 플레이어의 무기를 설정하는 패킷을 전송한다.
	// 이때 플레이어의 무기 정보는 ID(int) 로 전송되어야하며
	// 인자로써 주어진 weaponKey를 ID로 매핑하는 수단이 필요하다.

	if (sendPacket)
	{
		Send(Create_c2s_CHANGE_EQUIPMENT(Nagox::Enum::EQUIPMENT_TYPE::EQUIPMENT_TYPE_WEAPON, weaponID));
	}

	if (weaponID >= 0)
	{
		auto itemID = DATA_TABLE->GetItemID(GET_DATA(std::string, "Weapon", DATA_TABLE->GetWeaponIDStr(weaponID), "ItemKey"));

		// 자신의 플레이어 렌더러의 무기 모델을 설정하는 코드
		m_playerRenderer->SetPlayerWeapon(weaponID);
		m_playerEquipmentGUI->UpdateSlotContents(this, 0, itemID); // 0번 인덱스는 무기 슬롯으로 가정
	}
}

void AuthenticPlayer::SetPlayerArmor(int armorID, bool sendPacket)
{
	if (sendPacket)
	{
		Send(Create_c2s_CHANGE_EQUIPMENT(Nagox::Enum::EQUIPMENT_TYPE::EQUIPMENT_TYPE_ARMOR, armorID));
	}

	if (armorID >= 0)
	{
		auto itemID = DATA_TABLE->GetItemID(GET_DATA(std::string, "Armor", DATA_TABLE->GetArmorIDStr(armorID), "ItemKey"));

		m_playerRenderer->SetPlayerArmor(armorID);
		m_playerEquipmentGUI->UpdateSlotContents(this, 1, itemID); // 1번 인덱스는 갑옷 슬롯으로 가정
	}
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

void AuthenticPlayer::SetDialogueViewTarget(const std::shared_ptr<SceneObject>& target)
{
	m_dialogueViewTarget = target;
	m_dialogueViewTimer = target != nullptr ? 0.5f : 0.0f;
}

void AuthenticPlayer::SetCinematicViewTarget(const std::shared_ptr<SceneObject>& target)
{
	m_cinematicViewTarget = target;
	m_cinematicViewTimer = 3.0f;
}

void AuthenticPlayer::UpdateCameraTransform(Transform* pCameraTransfrom, float deltaTime)
{
	// Region: Camera Rotation Control
	float t = std::min(deltaTime * 16.0f, 1.0f);
	m_cameraAngleAxisSmooth.x = udsdx::LerpAngle(m_cameraAngleAxisSmooth.x, m_cameraAngleAxis.x, t);
	m_cameraAngleAxisSmooth.y = udsdx::LerpAngle(m_cameraAngleAxisSmooth.y, m_cameraAngleAxis.y, t);
	Quaternion cameraRotation = Quaternion::CreateFromYawPitchRoll(m_cameraAngleAxisSmooth * DEG2RAD);

	// If a dialogue view target is set, adjust the camera rotation
	Quaternion targetRotation = Quaternion::CreateFromYawPitchRoll(m_dialogueViewAngleAxis + Vector3::Up * PIDIV4);
	cameraRotation = Quaternion::Slerp(cameraRotation, targetRotation, m_dialogueCameraFactor);

	m_cameraAnchor->GetTransform()->SetLocalRotation(cameraRotation);

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
	Vector3 targetPosition = playerPosition + Vector3::Up * 1.5f;
	if (m_dialogueViewTarget != nullptr)
	{
		// If a dialogue view target is set, adjust the camera anchor position to focus on it
		targetPosition = Vector3::Lerp(targetPosition, m_dialogueViewTarget->GetTransform()->GetWorldPosition() + Vector3::Up * 1.5f, 0.5f);
	}
	if (m_cinematicViewTarget != nullptr)
	{
		targetPosition = Vector3::Lerp(targetPosition, m_cinematicViewTarget->GetTransform()->GetWorldPosition(), std::clamp(m_cinematicViewTimer, 0.0f, 1.0f));
	}
	Vector3 interpolatedAnchorPosition = Vector3::Lerp(m_cameraAnchorLastPosition, targetPosition, deltaTime * 8.0f);
	m_cameraAnchor->GetTransform()->SetLocalPosition(interpolatedAnchorPosition - playerPosition);
	m_cameraAnchorLastPosition = m_cameraAnchor->GetTransform()->GetWorldPosition();

	// Region: Camera Z Distance / Screen Offset Control
	bool focused = false; // INSTANCE(Input)->GetMouseRightButton();
	float distanceTarget = focused ? 1.0f : m_cameraDistance;

	m_cameraDistanceSmooth = std::lerp(m_cameraDistanceSmooth, -distanceTarget, deltaTime * 8.0f);
	float tParam = m_fMoveTime * 0.5f;
	float mParam = 0.04f;
	m_cameraXOffsetSmooth = std::lerp(m_cameraXOffsetSmooth, focused ? 0.25f : 0.0f, deltaTime * 8.0f);
	pCameraTransfrom->SetLocalPosition(Vector3(sin(tParam) * mParam + m_cameraXOffsetSmooth, sin(tParam * 2.0f) * mParam, m_cameraDistanceSmooth));

	// Region: Camera Position Postprocess
	if (m_environment)
	{
		HeightMap* heightMap = m_environment->HeightMap;
		const float TerrainSize = m_environment->TerrainSize;
		const float TerrainHeight = m_environment->TerrainHeight;
		const float TerrainOffset = m_environment->TerrainOffset;

		Transform* pAnchorTransform = m_cameraAnchor->GetTransform();
		Matrix4x4 cameraWorldMatrix = pAnchorTransform->GetWorldSRTMatrix();
		Matrix4x4 terrainWorldMatrix = Matrix4x4::CreateScale(TerrainSize, TerrainHeight, TerrainSize) * Matrix4x4::CreateTranslation(Vector3(TerrainOffset, 0.0f, TerrainOffset));
		Matrix4x4 cameraToTerrain = cameraWorldMatrix * terrainWorldMatrix.Invert();

		Vector3 terrainPos = Vector3::Transform(pCameraTransfrom->GetLocalPosition(), cameraToTerrain);
		float height = heightMap->GetHeight(terrainPos.x * heightMap->GetPixelWidth(), terrainPos.z * heightMap->GetPixelHeight());
		terrainPos.y = std::max(terrainPos.y, height + 0.25f / TerrainHeight);
		pCameraTransfrom->SetLocalPosition(Vector3::Transform(terrainPos, cameraToTerrain.Invert()));
	}

	// Region: Camera FOV Control
	m_fovAdd = std::lerp(m_fovAdd, 0.0f, deltaTime * 4.0f);
	m_pCamera->SetFov(m_fovBase + m_fovAdd);
}

void AuthenticPlayer::UpdateCameraTransformDebug(Transform* pCameraTransfrom, float deltaTime)
{
	m_cameraAnchor->GetTransform()->SetLocalPosition(Vector3::Zero);
	m_cameraAnchor->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(0.0f, PIDIV2, 0.0f));
	m_cameraObj->GetTransform()->SetLocalPosition(Vector3::Forward * 64.0f);
}

void AuthenticPlayer::TryClickScreen(int mouse)
{
	if (!CanAttack())
		return;
	switch (mouse)
	{
	case 0: // 왼쪽 마우스 버튼
		m_playerRenderer->Attack();
		m_playerStatusGUI->UseSkill(0);
		DoAttack(Nagox::Enum::SKILL_TYPE_DEFAULT);
		break;
	case 1: // 오른쪽 마우스 버튼
		m_playerRenderer->Attack();
		m_playerStatusGUI->UseSkill(1);
		DoAttack(Nagox::Enum::SKILL_TYPE_SKILL_1);
	}
	//std::cout << "공격 시도\n";
}

void AuthenticPlayer::OnInitialize()
{
	m_cameraAnchor = SceneObject::MakeShared();
	m_cameraObj = SceneObject::MakeShared();

	m_pCamera = m_cameraObj->AddComponent<CameraPerspective>();
	m_pCamera->SetClearColor(Color(1.0f, 1.0f, 1.0f, 1.0f));

	// TODO: 아이템 Json 레지스트리에서 종류의 개수를 알아내서 가져오면 좋겠다.
	m_inventory = std::vector<int>(16, 0);
	m_quickSlot = std::vector<int>(MAX_QUICK_SLOT, -1);

	m_cameraAnchorLastPosition = Vector3::Up * 120.0f;

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
	input_handler->AddKeyFunc(Keyboard::N, KEY_STATE::KET_TAP, []() { Send( Create_c2s_QUEST_END()); });

	input_handler->AddKeyFunc(Keyboard::CapsLock, KEY_STATE::KET_TAP, &AuthenticPlayer::RequestQuest, this);

	input_handler->AddKeyFunc(Keyboard::Enter, KEY_STATE::KET_TAP, &AuthenticPlayer::ToggleLogFloatGUI, this);

	m_entityMovement = AddComponent<EntityMovement>();
	m_playerRenderer = AddComponent<PlayerRenderer>();
	m_pServerObject = AddComponent<ServerObject>();
	m_pServerObject->AddComp<ForcedMovement>();
	m_entityMovement->SetFriction(40.0f);
}

void AuthenticPlayer::DoAttack(const Nagox::Enum::SKILL_TYPE skill_type)
{
	if constexpr (g_bUseNetWork)
	{
		const auto rad = CommonMath::GetYawFromQuaternion(m_cameraAnchor->GetTransform()->GetLocalRotation());
		m_bSendFlag = true;
		Send(
			Create_c2s_PLAYER_ATTACK(rad, ToFlatVec3(GetSceneObject()->GetTransform()->GetLocalPosition())
			, skill_type)
		);
	}

	switch (m_playerType)
	{
	case 0:
		attackSFXInstance = INSTANCE(Resource)->Load<AudioClip>(RESOURCE_PATH(L"audio\\attack_warrior.wav"))->CreateInstance();
		break;
	case 1:
		attackSFXInstance = INSTANCE(Resource)->Load<AudioClip>(RESOURCE_PATH(L"audio\\attack_priest.wav"))->CreateInstance();
		break;
	case 2:
		attackSFXInstance = INSTANCE(Resource)->Load<AudioClip>(RESOURCE_PATH(L"audio\\attack_archer.wav"))->CreateInstance();
		break;
	}
	if (attackSFXInstance)
	{
		attackSFXInstance->SetVolume(0.5f);
		attackSFXInstance->Play();
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

const bool AuthenticPlayer::CanAttack() const noexcept
{
	// 플레이어가 공격 중인 상태 (쿨타임을 기다려야한다)
	if (PlayerRenderer::AnimationState::Attack == m_playerRenderer->GetCurrentState())
		return false;
	// 경직상태에선 공격 못함
	if (PlayerRenderer::AnimationState::Hit == m_playerRenderer->GetCurrentState())
		return false;
	// 죽는모션도 마찬가지임
	if (PlayerRenderer::AnimationState::Death == m_playerRenderer->GetCurrentState())
		return false;
	if (PlayerRenderer::AnimationState::Dash == m_playerRenderer->GetCurrentState())
		return false;
	return true;
}

void AuthenticPlayer::Update(const Time& time, Scene& scene)
{
	Transform* transform = GetSceneObject()->GetTransform();

	if (m_dialogueViewTarget != nullptr)
	{
		Vector3 delta = m_dialogueViewTarget->GetTransform()->GetWorldPosition() - GetSceneObject()->GetTransform()->GetLocalPosition();
		delta.Normalize();
		m_dialogueViewAngleAxis = Vector3(-std::asin(delta.y), std::atan2(delta.x, delta.z), 0.0f);
	}

	Quaternion playerRotation = Quaternion::CreateFromYawPitchRoll(m_rendererBodyAngleY * DEG2RAD + PI, 0.0f, 0.0f);
	playerRotation = Quaternion::Slerp(playerRotation, Quaternion::CreateFromYawPitchRoll(m_dialogueViewAngleAxis.y + PI, 0.0f, 0.0f), m_dialogueViewFactor);

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
		m_cameraAngleAxis.y = std::fmod(m_cameraAngleAxis.y + 180.0f, 360.0f) - 180.0f;
	}

	if (m_bDebugCamera)
		UpdateCameraTransformDebug(m_cameraObj->GetTransform(), time.deltaTime);
	else
		UpdateCameraTransform(m_cameraObj->GetTransform(), time.deltaTime);

	const bool vec3int_equal = vPrevState.x == m_vCurState.x && vPrevState.y == m_vCurState.y && vPrevState.z == m_vCurState.z;

	float yaw = udsdx::LerpAngle(m_cameraAngleAxis.y, m_dialogueViewAngleAxis.y * RAD2DEG, m_dialogueViewFactor);
	float pitch = udsdx::LerpAngle(m_cameraAngleAxis.x, m_dialogueViewAngleAxis.x * RAD2DEG, m_dialogueViewFactor);
	m_playerRenderer->SetViewDirection(yaw, pitch);

	auto sceneObject = GetSceneObject();
	const auto input_handler = sceneObject->GetComponent<InputHandler>();
	m_bSendFlag = input_handler->IsKeyHit();

	const auto fm = sceneObject->GetComponent<ServerObject>()->GetComp<ForcedMovement>();
	if (fm->CheckDash() && PlayerRenderer::AnimationState::Dash != m_playerRenderer->GetCurrentState())
	{
		m_playerRenderer->Dash();
		m_playerStatusGUI->UseSkill(2);
		soundEffectInstance = INSTANCE(Resource)->Load<AudioClip>(RESOURCE_PATH(L"audio\\dash.wav"))->CreateInstance();
		soundEffectInstance->SetVolume(0.5f);
		soundEffectInstance->Play();
		m_fovAdd = 0.2f;
	}
	if (fm->IsForcedMovement())
	{
		fm->Update();
	}
	else
	{
		const auto navi = GetSceneObject()->GetComponent<ServerObject>()->GetNaviAgent();
		const auto prev_pos = GetSceneObject()->GetComponent<EntityMovement>()->prev_pos;
		Vector3 temp = prev_pos;
		navi->SetCellPos(DT, prev_pos, transform->GetLocalPosition(), temp);
		transform->SetLocalPosition(temp);
		GetSceneObject()->GetComponent<EntityMovement>()->prev_pos = temp;
	}

	m_dialogueViewTimer -= time.deltaTime;
	m_dialogueViewFactor = std::lerp(m_dialogueViewFactor, m_dialogueViewTarget != nullptr ? 1.0f : 0.0f, time.deltaTime * 4.0f);
	if (m_dialogueViewTimer <= 0.0f)
		m_dialogueCameraFactor = std::lerp(m_dialogueCameraFactor, m_dialogueViewTarget != nullptr ? 1.0f : 0.0f, time.deltaTime * 4.0f);
	m_cinematicViewTimer -= time.deltaTime;

	if (INSTANCE(Input)->GetKeyDown(Keyboard::P))
	{
		const auto pos = GetSceneObject()->GetTransform()->GetLocalPosition();
		std::cout << std::format("Vector3( {}F,{}F,{}F ) \n", pos.x, pos.y, pos.z);
		
		//const Vector3 end = { -119.499115f,75,13.64f }; // 마을 중앙
		//GuideSystem::GetInst()->ToggleFlag();
	}
	//if (INSTANCE(Input)->GetKeyDown(Keyboard::K))
	//{
	//	GuideSystem::GetInst()->ToggleFlag();
	//	if (GuideSystem::GetInst()->temp_force_pos == Vector3::Zero)
	//		GuideSystem::GetInst()->temp_force_pos = Vector3(-120.55993F, 76.17661F, 4.4105754F);
	//	else
	//		GuideSystem::GetInst()->temp_force_pos = Vector3::Zero;
	//}

	// TODO: 튜토리얼 호위퀘스트는 특정 위치에서 상호작용으로 하기
	if (INSTANCE(Input)->GetKeyDown(Keyboard::O))
	{
		//Send(Create_c2s_REGISTER_PARTY_QUEST(0));
		Send(Create_c2s_REQUEST_QUEST(Common::CommonQuestTable::GetCommonQuestInfo(L"여우 곰 잡기").quest_id));
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