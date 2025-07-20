#include "pch.h"
#include "PlayerTagGUI.h"
#include "GameScene.h"

using namespace udsdx;

void PlayerTagGUI::OnInitialize()
{
	m_nameObject = SceneObject::MakeShared();
	auto nameRenderer = m_nameObject->AddComponent<GUIText>();
	nameRenderer->SetText(L"Player Name");
	nameRenderer->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
}

void PlayerTagGUI::OnAttach()
{
	auto scene = GetSceneObject()->GetScene();
	if (scene != nullptr)
	{
		auto gameScene = dynamic_cast<GameScene*>(scene);
		if (gameScene != nullptr)
		{
			gameScene->AddInterfaceObject(m_nameObject);
			UpdateTransform(*scene);
		}
	}
}

void PlayerTagGUI::OnActive()
{
	m_nameObject->SetActive(true);
}

void PlayerTagGUI::OnInactive()
{
	m_nameObject->SetActive(false);
}

void PlayerTagGUI::OnDetach()
{
	m_nameObject->RemoveFromParent();
}

void PlayerTagGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	UpdateTransform(scene);
}

void PlayerTagGUI::UpdateTransform(udsdx::Scene& scene)
{
	auto gameScene = dynamic_cast<GameScene*>(&scene);
	if (gameScene != nullptr)
	{
		auto camera = gameScene->GetMainCamera();
		Vector3 viewPos = Vector3::Transform(GetTransform()->GetWorldPosition() + Vector3::Up * 1.7f, camera->GetViewMatrix());
		if (viewPos.z > 0.0f)
		{
			m_nameObject->SetActive(true);
			float aspectRatio = INSTANCE(Core)->GetAspectRatio();
			Vector3 screenPos = Vector3::Transform(viewPos, camera->GetProjMatrix(aspectRatio));
			screenPos.x *= GUIElement::RefScreenSize.y * aspectRatio * 0.5f;
			screenPos.y *= GUIElement::RefScreenSize.y * 0.5f;
			m_nameObject->GetTransform()->SetLocalPosition(Vector3(screenPos.x, screenPos.y, 0.0f));
		}
		else
			m_nameObject->SetActive(false);
	}
}

void PlayerTagGUI::SetText(std::wstring_view text)
{
	m_nameObject->GetComponent<GUIText>()->SetText(text.data());
}
