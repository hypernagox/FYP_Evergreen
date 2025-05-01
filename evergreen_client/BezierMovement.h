#pragma once

#include "pch.h"

class BezierMovement : public udsdx::Component
{
private:
	static constexpr int NumSteps = 128;

	struct ControlPoint
	{
		udsdx::Vector3 Position;
		udsdx::Vector3 TangentIn;
		udsdx::Vector3 TangentOut;
		udsdx::Quaternion Rotation;
	};

	struct Spline
	{
		std::vector<ControlPoint> ControlPoints;
		std::vector<float> LengthToStep;
	};

private:
	static udsdx::Vector3 GetPositionByTime(const ControlPoint& p1, const ControlPoint& p2, float t);
	static udsdx::Vector3 GetTangentByTime(const ControlPoint& p1, const ControlPoint& p2, float t);

public:
	BezierMovement(const std::shared_ptr<udsdx::SceneObject>& owner);

	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void LoadSpline(const std::wstring& filePath);

	void SetSpeed(float speed) { m_speed = speed; }
	float GetSpeed() const { return m_speed; }

private:
	float m_currentTime = 0.0f;
	float m_speed = 1.0f;

	Spline m_spline;
};