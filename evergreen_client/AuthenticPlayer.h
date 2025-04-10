#pragma once

#include "pch.h"

using namespace udsdx;

class ServerObject;
class EntityMovement;
class PlayerRenderer;
class PlayerStatusGUI;
class PlayerQuickSlotGUI;
class PlayerInventoryGUI;
class PlayerCraftGUI;

// AuthenticPlayer는 우리가 조종할 수 있는 진짜의 플레이어를 나타내는 클래스
//   - 인풋 핸들러를 이용해 플레이어의 움직임을 직접 조종
//   - 플레이어의 현재 위치를 계속해서 서버로 전송
//   - 카메라를 생성하고 조종
class AuthenticPlayer : public Component
{
private:
	std::shared_ptr<SceneObject> m_cameraAnchor;
	std::shared_ptr<SceneObject> m_cameraObj;
	std::shared_ptr<SceneObject> m_particlePrefab;

	bool m_bGround = false;
	float m_rendererBodyAngleY = 0.0f;

	std::unique_ptr<SoundEffectInstance> soundEffectInstance;

	float m_fMoveSpeed = 100.f;
	float m_fMoveTime = 0.0f;
	float m_fCamSensivity = 0.1f;

	float m_fovBase = 75.0f * DEG2RAD;

	int m_iMaxHP = 5; // TODO: Magic Number
	int m_iCurHP = 5;

	// 플레이어의 아이템 ID별 소지 개수를 나타내는 자료구조
	// ex) m_inventory[x] = y: id가 x인 아이템을 y개 소지하고 있다.
	std::vector<int> m_inventory;

	// 플레이어의 슬롯별로 등록된 아이템을 나타내는 자료구조
	// ex) m_quickSlot[x] = y: x번 슬롯에 id가 y인 아이템이 등록되어 있다.
	// y가 -1일 경우 아무 아이템도 등록되지 않았음을 의미
	std::vector<int> m_quickSlot;

	Vector3 m_cameraAngleAxis = Vector3::Zero;
	Vector3 m_cameraAngleAxisSmooth = Vector3::Zero;

	CameraPerspective* m_pCamera;

	bool m_bSendFlag = false;
	Vector3Int m_vCurState = {};

	PlayerRenderer* m_playerRenderer = nullptr;
	EntityMovement* m_entityMovement = nullptr;
	ServerObject* m_pServerObject = nullptr;

	PlayerStatusGUI* m_playerStatusGUI = nullptr;
	PlayerQuickSlotGUI* m_playerQuickSlotGUI = nullptr;
	PlayerInventoryGUI* m_playerInventoryGUI = nullptr;
	PlayerCraftGUI* m_playerCraftGUI = nullptr;

private:
	void UpdatePlayerCamFpsMode(float deltaTime);
	void MoveByView(const Vector3& vDelta);
	void UpdateCameraTransform(Transform* pCameraTransfrom, float deltaTime);
	void RequestQuest();
public:
	AuthenticPlayer(const std::shared_ptr<SceneObject>& object);
	~AuthenticPlayer();

	void FireProj();
	void DoAttack();
	void Start();
	void Update(const Time& time, Scene& scene) override;
	void TryClickScreen();

	Vector3 GetPlayerLook() const noexcept;
	void InitCamDirection();
	bool& GetSendFlag()noexcept { return m_bSendFlag; }
	const float GetYAngle()const noexcept { return m_rendererBodyAngleY; }
	void SetPlayerStatusGUI(PlayerStatusGUI* playerStatusGUI) noexcept;
	void SetPlayerQuickSlotGUI(PlayerQuickSlotGUI* playerQuickSlotGUI) noexcept;
	void SetPlayerInventoryGUI(PlayerInventoryGUI* playerInventoryGUI) noexcept;
	void SetPlayerCraftGUI(PlayerCraftGUI* playerCraftGUI) noexcept;
	void OnHit(int damage);
	void OnModifyInventory(uint8_t itemID, int delta);

	void SetQuickSlotItemOnBlank(uint8_t itemID);
	void SetQuickSlotItem(int index, uint8_t itemID);
	void UseQuickSlotItem(int index);
	void CraftItem(int recipeIndex);
public:
	const auto& GetStatusGUI()const noexcept { return m_playerStatusGUI; }
};

