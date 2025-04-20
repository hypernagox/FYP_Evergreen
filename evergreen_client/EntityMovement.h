#pragma once

#include <updown_studio.h>

using namespace udsdx;

class EntityMovement : public udsdx::Component
{
public:
	EntityMovement(const std::shared_ptr<udsdx::SceneObject>& object);
	~EntityMovement();

	void Update(const Time& time, Scene& scene) override;

	void AddVelocity(const Vector3& velocity) { m_velocity += velocity; }
	void AddAcceleration(const Vector3& acceleration) { m_acceleration += acceleration; }

	void SetVelocity(const Vector3& velocity) { m_velocity = velocity; }
	void SetAcceleration(const Vector3& acceleration) { m_acceleration = acceleration; }

	void SetFriction(float friction) { m_friction = friction; }

	void SetPosition(const Vector3& position) { GetSceneObject()->GetTransform()->SetLocalPosition(position); }
	void SetRotation(const Quaternion& rotation) { GetSceneObject()->GetTransform()->SetLocalRotation(rotation); }

	Vector3 GetVelocity() const { return m_velocity; }
	Vector3 GetAcceleration() const { return m_acceleration; }
	float GetFriction() const { return m_friction; }

	Vector3 GetPosition() const { return GetSceneObject()->GetTransform()->GetLocalPosition(); }
	Quaternion GetRotation() const { return GetSceneObject()->GetTransform()->GetLocalRotation(); }

	// TODO: юс╫ц
	Vector3 prev_pos;
private:
	Vector3 m_velocity = Vector3::Zero;
	Vector3 m_acceleration = Vector3::Zero;

	float m_friction = 20.0f;

	// Max values must be positive
	float m_velocityHMax = 7.5f;
	float m_velocityVMax = 20.0f;
};