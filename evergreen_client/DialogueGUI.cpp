#include "pch.h"
#include "DialogueGUI.h"
#include "GUISimpleButton.h"

using namespace udsdx;

void DialogueGUI::OnInitialize()
{
	m_panel = SceneObject::MakeShared();
	auto panelRenderer = m_panel->AddComponent<GUIImage>();
	panelRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\dialogue\\dialogue_background.png")));
	panelRenderer->SetSize(Vector2(8192.0f, 1440.0f));
	GetSceneObject()->AddChild(m_panel);

	m_nameText = SceneObject::MakeShared();
	m_nameText->GetTransform()->SetLocalPosition(Vector3(0.0f, -395.0f, 0.0f));
	m_nameText->GetTransform()->SetLocalScale(1.5f);
	auto nameTextRenderer = m_nameText->AddComponent<GUIText>();
	nameTextRenderer->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
	nameTextRenderer->SetRaycastTarget(false);
	nameTextRenderer->SetText(L"####");
	m_panel->AddChild(m_nameText);

	m_dialogueText = SceneObject::MakeShared();
	m_dialogueText->GetTransform()->SetLocalPosition(Vector3(-430.0f, -473.0f, 0.0f));
	auto textRenderer = m_dialogueText->AddComponent<GUIText>();
	textRenderer->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
	textRenderer->SetRaycastTarget(false);
	textRenderer->SetAlignment(GUIText::Alignment::UpperLeft);
	textRenderer->SetText(L"Insert Dialogue Caption Here.");
	m_panel->AddChild(m_dialogueText);

	auto nameTagImageObject = SceneObject::MakeShared();
	auto nameTagImageRenderer = nameTagImageObject->AddComponent<GUIImage>();
	nameTagImageRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\dialogue\\nametag.png")));
	nameTagImageRenderer->SetSize(Vector2(1550.0f, 100.0f));
	nameTagImageObject->GetTransform()->SetLocalPosition(Vector3(0.0f, -395.0f, 0.0f));
	m_panel->AddChild(nameTagImageObject);

	m_nextButton = SceneObject::MakeShared();
	m_nextButton->GetTransform()->SetLocalPosition(Vector3(1051.0f, -623.0f, 0.0f));
	auto nextButtonRenderer = m_nextButton->AddComponent<GUISimpleButton>();
	nextButtonRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\dialogue\\choice.png")), true);
	nextButtonRenderer->SetClickCallback([this]() { OnDialogueNext(); });
	nextButtonRenderer->SetSize(Vector2(390.0f, 78.0f));
	m_panel->AddChild(m_nextButton);

	auto nextButtonTextObject = SceneObject::MakeShared();
	auto nextButtonTextRenderer = nextButtonTextObject->AddComponent<GUIText>();
	nextButtonTextRenderer->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
	nextButtonTextRenderer->SetRaycastTarget(false);
	nextButtonTextRenderer->SetText(L"´ÙÀ½");
	nextButtonTextObject->GetTransform()->SetLocalScale(1.5f);
	m_nextButton->AddChild(nextButtonTextObject);

	m_dialogueSound = INSTANCE(Resource)->Load<AudioClip>(RESOURCE_PATH(L"audio\\dialogue.wav"))->CreateInstance();
	m_dialogueSound->SetVolume(0.5f);
	m_dialogueSound->Play(true);
	m_dialogueSound->Pause();

	m_panel->SetActive(false);
}

void DialogueGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	m_charTimer += time.deltaTime;

	int charCount = static_cast<int>(std::ceil(CharPerSecond * m_charTimer));
	if (charCount >= m_currentDialogue.size())
	{
		m_dialogueSound->Pause();
		m_dialogueText->GetComponent<GUIText>()->SetText(m_currentDialogue);
	}
	else
	{
		std::wstring currentText = m_currentDialogue.substr(0, charCount);
		m_dialogueText->GetComponent<GUIText>()->SetText(currentText);
	}

	m_panel->GetTransform()->SetLocalPositionY(std::lerp(m_panel->GetTransform()->GetLocalPosition().y, 0.0f, time.deltaTime * 4.0f));
}

void DialogueGUI::ShowDialogue(const std::shared_ptr<udsdx::SceneObject>& target, std::string_view dialogueKey)
{
	m_panel->GetTransform()->SetLocalPosition(Vector3(0.0f, -1440.0f, 0.0f)); // Start off-screen

	std::wstring name = GET_DATA(std::wstring, "Dialogue", dialogueKey, "Name");
	nlohmann::ordered_json dialogueData = GET_DATA(nlohmann::ordered_json, "Dialogue", dialogueKey, "Dialogues");

	while (!m_dialogueCache.empty()) {
		m_dialogueCache.pop();
	}
	for (const auto& dialogue : dialogueData["Texts"]) {
		m_dialogueCache.emplace(Common::DataRegistry::Str2Wstr(dialogue.get<std::string>()));
	}

	m_nameText->GetComponent<GUIText>()->SetText(name);
	m_panel->SetActive(true);

	OnDialogueNext();
}

void DialogueGUI::OnDialogueNext()
{
	if (m_dialogueCache.empty())
	{
		m_panel->SetActive(false);
		m_dialogueSound->Pause();
		if (m_onDialogueEndCallback)
		{
			m_onDialogueEndCallback();
		}
		return;
	}

	m_dialogueSound->Resume();
	m_currentDialogue = m_dialogueCache.front();
	m_dialogueCache.pop();

	m_charTimer = 0.0f;
}