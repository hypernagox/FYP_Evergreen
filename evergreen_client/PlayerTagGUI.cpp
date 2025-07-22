#include "pch.h"
#include "PlayerTagGUI.h"
#include "GameScene.h"
#include "WorldSpaceGUI.h"

using namespace udsdx;

void PlayerTagGUI::OnInitialize()
{
	m_nameObject = SceneObject::MakeShared();
	auto nameRenderer = m_nameObject->AddComponent<GUIText>();
	nameRenderer->SetText(L"Player Name");
	nameRenderer->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));

	auto panelWorldTransform = AddComponent<WorldSpaceGUI>();
	panelWorldTransform->SetSourceTransform(GetTransform());
	panelWorldTransform->SetTargetObject(m_nameObject);
	panelWorldTransform->SetViewFar(100.0f);
	panelWorldTransform->SetWorldOffset(Vector3::Up * 1.7f);

	m_nameObject->SetActive(false);
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
		}
	}
}

void PlayerTagGUI::OnDetach()
{
	m_nameObject->RemoveFromParent();
}

void PlayerTagGUI::SetText(std::wstring_view text)
{
	m_nameObject->GetComponent<GUIText>()->SetText(text.data());
}
