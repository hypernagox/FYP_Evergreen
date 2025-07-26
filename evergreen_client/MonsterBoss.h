#pragma once

#include "pch.h"
#include "Monster.h"
#include "EventTimer.h"

using namespace udsdx;

class ServerObject;

class MonsterBoss : public Monster
{
protected:
	EventTimer m_eventTimer;
	AnimationClip* m_animation;
	AnimationClip* m_flightAnimation;
	ServerObject* m_serverObject;
	std::shared_ptr<SceneObject> m_bossStatusGUI;

	bool m_isFlyMovement = false;
	bool m_isTakeoff = false;
	Vector3 m_flightStartPosition = Vector3::Zero;
	Vector3 m_flightEndPosition = Vector3::Zero;
	float m_flightTime = 0.0f;
	float m_flightTimeTotal = 0.0f;

public:
	void OnInitialize() override;
	void OnAttach() override;
	void OnDetach() override;
	void Update(const Time& time, Scene& scene) override;
	void OnHit(int afterHealth) override;
	void OnDeath() override;
	void UpdateFlightMovement(float deltaTime);
	void OnTakeoffAtPosition(const Vector3& pos);
	void OnLandingAtPosition(const Vector3& pos);
	virtual void OnAnimationStateChange(AnimationState from, AnimationState to) override;
};