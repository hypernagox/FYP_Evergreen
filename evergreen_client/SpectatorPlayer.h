#pragma once

#include "pch.h"

using namespace udsdx;

// SpectatorPlayer는 디버깅을 위해 자유자재로 카메라를 조종할 수 있는 객체를 나타내는 클래스
//   - 인풋 핸들러를 이용해 카메라의 움직임을 직접 조종
class SpectatorPlayer : public Component
{
public:
	SpectatorPlayer(const std::shared_ptr<SceneObject>& object);
	void Update(const Time& time, Scene& scene) override;

	void MoveByView(const Vector3& acceleration);
	void UpdateMovement(const Time& time);

	Camera* GetCameraComponent() const noexcept { return GetComponent<Camera>(); }

private:
	Vector3 m_velocity = Vector3::Zero;
	float m_maxSpeed = 40.0f;
	bool m_isMoving = false;

	Vector3 m_cameraAxis = Vector3::Zero;
	Vector3 m_cameraAxisSmooth = Vector3::Zero;
};