#include "pch.h"
#include "PlayerTagGUI.h"
#include "GameScene.h"

using namespace udsdx;

PlayerTagGUI::PlayerTagGUI(const std::shared_ptr<udsdx::SceneObject>& object) : Component(object)
{
	m_nameObject = std::make_shared<SceneObject>();
	auto nameRenderer = m_nameObject->AddComponent<GUIText>();
	nameRenderer->SetText(L"Player Name");
	nameRenderer->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));

	object->AddChild(m_nameObject);
}

void PlayerTagGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	auto gameScene = dynamic_cast<GameScene*>(&scene);
	if (gameScene != nullptr)
	{
		auto camera = gameScene->GetMainCamera();
		Vector3 viewPos = Vector3::Transform(m_targetPos, camera->GetViewMatrix());
		if (viewPos.z > 0.0f)
		{
			m_nameObject->SetActive(true);
			float aspectRatio = INSTANCE(Core)->GetAspectRatio();
			Vector3 screenPos = Vector3::Transform(viewPos, camera->GetProjMatrix(aspectRatio));
			screenPos.x *= GUIElement::RefScreenSize.y * aspectRatio * 0.5f;
			screenPos.y *= GUIElement::RefScreenSize.y * 0.5f;
			GetTransform()->SetLocalPosition(Vector3(screenPos.x, screenPos.y, 0.0f));
		}
		else
			m_nameObject->SetActive(false);
	}
}
