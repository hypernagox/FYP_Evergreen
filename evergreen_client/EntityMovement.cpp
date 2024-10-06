#include "pch.h"
#include "EntityMovement.h"
#include "NaviAgent.h"
#include "ServerObject.h"

EntityMovement::EntityMovement(const std::shared_ptr<udsdx::SceneObject>& object) : Component(object)
{
}

EntityMovement::~EntityMovement()
{
}

void EntityMovement::Update(const Time& time, Scene& scene)
{
	Transform* transform = GetSceneObject()->GetTransform();
	const Vector3 position = transform->GetLocalPosition();
	const Quaternion rotation = transform->GetLocalRotation();

	Vector3 velocityH = Vector3(m_velocity.x, 0.0f, m_velocity.z);
	velocityH.Normalize();
	Vector3 frictionVector = -velocityH * m_friction;
	m_velocity += frictionVector * time.deltaTime;

	if (velocityH.Dot(m_velocity) < 0.0f)
	{
		m_velocity.x = 0.0f;
		m_velocity.z = 0.0f;
	}

	const Vector3 newVelocity = m_velocity + m_acceleration * time.deltaTime;

	const float newVelocityHLength = Vector2(newVelocity.x, newVelocity.z).Length();
	const float newVelocityVLength = newVelocity.y;

	m_velocity = newVelocity;

	// Assuming newVelocityHLength is always positive
	if (newVelocityHLength > m_velocityHMax)
	{
		const float ratio = m_velocityHMax / newVelocityHLength;
		m_velocity.x *= ratio;
		m_velocity.z *= ratio;
	}

	// Assuming newVelocityVLength is always positive
	if (newVelocityVLength > m_velocityVMax)
	{
		const float ratio = m_velocityVMax / newVelocityVLength;
		m_velocity.y *= ratio;
	}
	//m_velocity.y = 0;
	transform->SetLocalPosition(position + m_velocity * time.deltaTime);

	const auto navi = GetSceneObject()->GetComponent<ServerObject>()->m_pNaviAgent;
	transform->SetLocalPosition(navi->GetNaviPos(transform->GetLocalPosition()));

}