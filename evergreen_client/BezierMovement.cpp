#include "pch.h"
#include "BezierMovement.h"

using namespace udsdx;

Vector3 BezierMovement::GetPositionByTime(const ControlPoint& p1, const ControlPoint& p2, float t)
{
	// Cubic Bezier curve formula
	const float it = 1.0f - t;
	const float cf1 = it * it * it;
	const float cf2 = 3.0f * it * it * t;
	const float cf3 = 3.0f * it * t * t;
	const float cf4 = t * t * t;

	return cf1 * p1.Position + cf2 * p1.TangentOut + cf3 * p2.TangentIn + cf4 * p2.Position;
}

Vector3 BezierMovement::GetTangentByTime(const ControlPoint& p1, const ControlPoint& p2, float t)
{
	const float it = 1.0f - t;
	const float cf1d = 3.0f * it * it;
	const float cf2d = 6.0f * it * t;
	const float cf3d = 3.0f * t * t;
	
	return cf1d * (p1.TangentOut - p1.Position) + cf2d * (p2.TangentIn - p1.TangentOut) + cf3d * (p2.Position - p2.TangentIn);
}

BezierMovement::BezierMovement(const std::shared_ptr<SceneObject>& owner) : Component(owner)
{
}

void BezierMovement::Update(const Time& time, Scene& scene)
{
	m_currentTime += time.deltaTime * m_speed;

	Transform* transform = GetSceneObject()->GetTransform();
	if (m_spline.ControlPoints.size() < 2)
		return;

	float length = fmodf(m_currentTime, m_spline.LengthToStep.back());
	auto targetStep = std::prev(std::upper_bound(m_spline.LengthToStep.begin(), m_spline.LengthToStep.end(), length));
	int steps = static_cast<int>(std::distance(m_spline.LengthToStep.begin(), targetStep));

	const int numSegments = static_cast<int>(m_spline.ControlPoints.size());
	const int targetIndex = steps / NumSteps;
	const int nextIndex = (targetIndex + 1) % numSegments;

	const float t = (steps % NumSteps + (*targetStep - length) / (*targetStep - *std::next(targetStep))) / NumSteps;

	Vector3 position = GetPositionByTime(m_spline.ControlPoints[targetIndex], m_spline.ControlPoints[nextIndex], t);
	Vector3 tangent = GetTangentByTime(m_spline.ControlPoints[targetIndex], m_spline.ControlPoints[nextIndex], t);

	// Normalize the tangent vector
	tangent.Normalize();

	// Calculate the rotation quaternion
	const float smoothT = t * t * 3.0f - t * t * t * 2.0f;
	const Matrix4x4 cpTransform = Matrix4x4::CreateFromQuaternion(Quaternion::Slerp(m_spline.ControlPoints[targetIndex].Rotation, m_spline.ControlPoints[nextIndex].Rotation, smoothT));
	Vector3 up = Vector3::TransformNormal(Vector3::Up, cpTransform);
	Vector3 right = up.Cross(tangent);
	Quaternion rotation = Quaternion::CreateFromRotationMatrix(Matrix4x4(right, up, tangent));

	// Set the position and rotation of the object
	transform->SetLocalPosition(position);
	transform->SetLocalRotation(rotation);
}

void BezierMovement::LoadSpline(const std::wstring& filePath)
{
	std::ifstream file(filePath.data());
	nlohmann::json j;
	file >> j;

	m_spline.ControlPoints.clear();
	for (auto& controlPoint : j)
	{
		ControlPoint cp;
		cp.Position = Vector3(controlPoint["Position"]["x"], controlPoint["Position"]["y"], controlPoint["Position"]["z"]);
		cp.TangentIn = Vector3(controlPoint["TangentIn"]["x"], controlPoint["TangentIn"]["y"], controlPoint["TangentIn"]["z"]);
		cp.TangentOut = Vector3(controlPoint["TangentOut"]["x"], controlPoint["TangentOut"]["y"], controlPoint["TangentOut"]["z"]);
		cp.Rotation = Quaternion(controlPoint["Rotation"]["value"]["x"], controlPoint["Rotation"]["value"]["y"], controlPoint["Rotation"]["value"]["z"], controlPoint["Rotation"]["value"]["w"]);

		// Tangent In/Out Point is in local space. Convert to world space
		Matrix4x4 worldMatrix = Matrix4x4::CreateFromQuaternion(cp.Rotation) * Matrix4x4::CreateTranslation(cp.Position);
		cp.TangentIn = Vector3::Transform(cp.TangentIn, worldMatrix);
		cp.TangentOut = Vector3::Transform(cp.TangentOut, worldMatrix);

		m_spline.ControlPoints.emplace_back(std::move(cp));
	}

	const int numSegments = static_cast<int>(m_spline.ControlPoints.size());
	m_spline.LengthToStep.resize(numSegments * NumSteps + 1);
	m_spline.LengthToStep[0] = 0.0f;
	Vector3 lastPosition = m_spline.ControlPoints[0].Position;
	for (size_t i = 0; i < numSegments; ++i)
	{
		for (int j = 0; j < NumSteps; ++j)
		{
			float t = static_cast<float>(j + 1) / NumSteps;
			Vector3 position = GetPositionByTime(m_spline.ControlPoints[i], m_spline.ControlPoints[(i + 1) % numSegments], t);
			float distance = (position - lastPosition).Length();
			m_spline.LengthToStep[i * NumSteps + j + 1] = m_spline.LengthToStep[i * NumSteps + j] + distance;
			lastPosition = position;
		}
	}
}