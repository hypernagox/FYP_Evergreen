#include "pch.h"
#include "EntityMovement.h"
#include "NaviAgent.h"
#include "ServerObject.h"

void EntityMovement::Update(const Time& time, Scene& scene)
{
	if (INSTANCE(Input)->GetKeyDown(Keyboard::OemPlus))
	{
		m_factor = std::min(m_factor + 1.f, 5.f);
	}
	if (INSTANCE(Input)->GetKeyDown(Keyboard::OemMinus))
	{
		m_factor = std::max(m_factor - 1.f, 1.f);
	}
	m_velocityHForwardMax = 7.5f * m_factor;
	m_velocityHBackMax = 3.0f * m_factor;
	m_velocityVMax = 20.0f * m_factor;

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

	Vector3 velocityHN = Vector3(m_velocity.x, 0.0f, m_velocity.z);
	velocityHN.Normalize();
	float velocityHMax = std::lerp(m_velocityHBackMax, m_velocityHForwardMax, velocityHN.Dot(m_forward) * 0.5f + 0.5f);

	// Assuming newVelocityHLength is always positive
	if (newVelocityHLength > velocityHMax)
	{
		const float ratio = velocityHMax / newVelocityHLength;
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

	//const auto navi = GetSceneObject()->GetComponent<ServerObject>()->m_pNaviAgent;
	//transform->SetLocalPosition(navi->GetNaviPos(transform->GetLocalPosition()));

}