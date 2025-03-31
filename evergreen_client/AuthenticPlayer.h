#pragma once

#include "pch.h"

using namespace udsdx;

class ServerObject;
class EntityMovement;
class PlayerRenderer;
class PlayerStatusGUI;
class PlayerQuickSlotGUI;

// AuthenticPlayer�� �츮�� ������ �� �ִ� ��¥�� �÷��̾ ��Ÿ���� Ŭ����
//   - ��ǲ �ڵ鷯�� �̿��� �÷��̾��� �������� ���� ����
//   - �÷��̾��� ���� ��ġ�� ����ؼ� ������ ����
//   - ī�޶� �����ϰ� ����
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

	Vector3 GetPlayerLook() const noexcept;
	void InitCamDirection();
	bool& GetSendFlag()noexcept { return m_bSendFlag; }
	const float GetYAngle()const noexcept { return m_rendererBodyAngleY; }
	void SetPlayerStatusGUI(PlayerStatusGUI* playerStatusGUI) noexcept;
	void SetPlayerQuickSlotGUI(PlayerQuickSlotGUI* playerQuickSlotGUI) noexcept;
	void OnHit(int damage);

	void SetQuickSlotItem(int index, uint8_t itemID);
	void UseQuickSlotItem(int index);
};

