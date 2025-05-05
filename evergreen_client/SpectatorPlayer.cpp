#include "pch.h"
#include "SpectatorPlayer.h"
#include "InputHandler.h"

SpectatorPlayer::SpectatorPlayer(const std::shared_ptr<SceneObject>& object) : Component(object)
{
	auto camera = AddComponent<CameraPerspective>();
	camera->SetFov(45.0f);

	auto input_handler = AddComponent<InputHandler>();
	const float acceleration = 100.0f;

	input_handler->AddKeyFunc(Keyboard::A, KEY_STATE::KEY_HOLD, &SpectatorPlayer::MoveByView, this, Vector3::Left * acceleration);
	input_handler->AddKeyFunc(Keyboard::W, KEY_STATE::KEY_HOLD, &SpectatorPlayer::MoveByView, this, Vector3::Backward * acceleration);
	input_handler->AddKeyFunc(Keyboard::S, KEY_STATE::KEY_HOLD, &SpectatorPlayer::MoveByView, this, Vector3::Forward * acceleration);
	input_handler->AddKeyFunc(Keyboard::D, KEY_STATE::KEY_HOLD, &SpectatorPlayer::MoveByView, this, Vector3::Right * acceleration);
	input_handler->AddKeyFunc(Keyboard::LeftShift, KEY_STATE::KEY_HOLD, &SpectatorPlayer::MoveByView, this, Vector3::Down * acceleration);
	input_handler->AddKeyFunc(Keyboard::Space, KEY_STATE::KEY_HOLD, &SpectatorPlayer::MoveByView, this, Vector3::Up * acceleration);
}

void SpectatorPlayer::Update(const Time& time, Scene& scene)
{
	UpdateMovement(time);
}

void SpectatorPlayer::MoveByView(const Vector3& acceleration)
{
	const float deltaTime = DT;

	Vector3 accelerationLocal = Vector3::TransformNormal(acceleration, Matrix4x4::CreateFromQuaternion(GetTransform()->GetLocalRotation()));
	m_velocity += accelerationLocal * deltaTime;
	if (m_velocity.LengthSquared() > m_maxSpeed * m_maxSpeed)
		m_velocity *= m_maxSpeed / m_velocity.Length();

	m_isMoving = true;
}

void SpectatorPlayer::UpdateMovement(const Time& time)
{
	Vector3 localPosition = GetTransform()->GetLocalPosition();
	localPosition += m_velocity * time.deltaTime;
	GetTransform()->SetLocalPosition(localPosition);

	if (!m_isMoving)
	{
		const float deceleration = 100.0f;
		const float delta = deceleration * time.deltaTime;
		if (m_velocity.LengthSquared() > delta * delta)
		{
			Vector3 normalized = -m_velocity;
			normalized.Normalize();
			m_velocity += normalized * delta;
		}
		else
			m_velocity = Vector3::Zero;
	}
	m_isMoving = false;

	if (INSTANCE(Input)->GetMouseMode() == Mouse::Mode::MODE_RELATIVE)
	{
		float mouse_dx = static_cast<float>(INSTANCE(Input)->GetMouseX());
		float mouse_dy = static_cast<float>(INSTANCE(Input)->GetMouseY());
		Vector3 delta = Vector3(mouse_dy, mouse_dx, 0.0f);
		const float sensitivity = 0.1f;
		m_cameraAxis += delta * sensitivity;
		m_cameraAxis.x = std::clamp(m_cameraAxis.x, -80.0f, 80.0f);
	}

	m_cameraAxisSmooth = Vector3::Lerp(m_cameraAxisSmooth, m_cameraAxis, std::min(time.deltaTime * 16.0f, 1.0f));
	m_cameraAxisSmooth.z = (m_cameraAxisSmooth.y - m_cameraAxis.y) * 0.25f;
	GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(m_cameraAxisSmooth * DEG2RAD));
}
