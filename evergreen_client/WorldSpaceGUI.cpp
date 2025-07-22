#include "pch.h"
#include "WorldSpaceGUI.h"
#include "GameScene.h"

using namespace udsdx;

void WorldSpaceGUI::PostUpdate(const Time& time, Scene& scene)
{
	auto gameScene = dynamic_cast<GameScene*>(&scene);
	if (gameScene != nullptr)
	{
		auto camera = gameScene->GetMainCamera();
		Vector3 viewPos = Vector3::Transform((m_sourceTransform ? m_sourceTransform->GetWorldPosition() : Vector3::Zero) + m_worldOffset, camera->GetViewMatrix());
		if (viewPos.z > 0.0f && viewPos.z < m_viewFar && (viewPos / viewPos.Length()).Dot(Vector3::Backward) > m_angleRange)
		{
			m_targetObject->SetActive(true);
			float aspectRatio = INSTANCE(Core)->GetAspectRatio();
			Vector3 screenPos = Vector3::Transform(viewPos, camera->GetProjMatrix(aspectRatio));
			screenPos.x *= GUIElement::RefScreenSize.y * aspectRatio * 0.5f;
			screenPos.y *= GUIElement::RefScreenSize.y * 0.5f;
			m_targetObject->GetTransform()->SetLocalPosition(Vector3(screenPos.x, screenPos.y, 0.0f) + m_screenOffset);
		}
		else
			m_targetObject->SetActive(false);
	}
}

void WorldSpaceGUI::SetTargetObject(const std::shared_ptr<udsdx::SceneObject>& targetObject)
{
	m_targetObject = targetObject;
}

void WorldSpaceGUI::SetSourceTransform(Transform* sourceTransform)
{
	m_sourceTransform = sourceTransform;
}

void WorldSpaceGUI::SetWorldOffset(const Vector3& offset)
{
	m_worldOffset = offset;
}

void WorldSpaceGUI::SetScreenOffset(const udsdx::Vector3& offset)
{
	m_screenOffset = offset;
}

void WorldSpaceGUI::SetViewFar(float viewFar)
{
	m_viewFar = viewFar;
}

void WorldSpaceGUI::SetAngleRange(float degrees)
{
	m_angleRange = std::cosf(degrees * DEG2RAD);
}
