#pragma once

#include "pch.h"

using namespace udsdx;

class ServerObject;
class EntityMovement;
class PlayerRenderer;

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

	std::function<void(void)> m_fpChangeCamMode[3];

	float m_fMoveSpeed = 100.f;
	float m_fMoveTime = 0.0f;
	float m_fCamSensivity = 0.1f;

	float m_fovBase = PIDIV4;

	Vector3 m_cameraAngleAxis = Vector3::Zero;
	Vector3 m_cameraAngleAxisSmooth = Vector3::Zero;

	CameraPerspective* m_pCamera;
	unsigned short m_curCamMode = 1;

	bool m_bSendFlag = false;
	Vector3Int m_vCurState = {};

	PlayerRenderer* m_playerRenderer = nullptr;
	EntityMovement* m_entityMovement = nullptr;
	ServerObject* m_pServerObject = nullptr;

private:
	void UpdatePlayerCamFpsMode(float deltaTime);
	void MoveByView(const Vector3& vDelta);
	void UpdateCameraTransform(Transform* pCameraTransfrom, float deltaTime);
	void DoAttack();
	void RequestQuest();
public:
	AuthenticPlayer(const std::shared_ptr<SceneObject>& object);
	~AuthenticPlayer();

	void Start();
	void Update(const Time& time, Scene& scene) override;

	Vector3 GetPlayerLook() const noexcept;
	void InitCamDirection();
	bool& GetSendFlag()noexcept { return m_bSendFlag; }
	const float GetYAngle()const noexcept { return m_rendererBodyAngleY; }
};

