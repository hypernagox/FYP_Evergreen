#pragma once

#include "pch.h"

using namespace udsdx;

// SpectatorPlayer�� ������� ���� ��������� ī�޶� ������ �� �ִ� ��ü�� ��Ÿ���� Ŭ����
//   - ��ǲ �ڵ鷯�� �̿��� ī�޶��� �������� ���� ����
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