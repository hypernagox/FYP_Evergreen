#pragma once

#include "pch.h"

class DialogueGUI : public udsdx::Component
{
public:
	void OnInitialize() override;
	void OnInactive() override;
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void ShowDialogue(const std::shared_ptr<udsdx::SceneObject>& target, std::string_view dialogueKey);
	void SetOnDialogueEndCallback(std::function<void()> callback) { m_onDialogueEndCallback = callback; }

private:
	static constexpr float CharPerSecond = 30.0f;

	std::shared_ptr<udsdx::SceneObject> m_panel;
	std::shared_ptr<udsdx::SceneObject> m_nameText;
	std::shared_ptr<udsdx::SceneObject> m_dialogueText;
	std::shared_ptr<udsdx::SceneObject> m_nextButton;
	std::wstring m_currentDialogue;
	std::queue<std::wstring> m_dialogueCache;
	std::unique_ptr<SoundEffectInstance> m_dialogueSound;

	float m_charTimer = 0.0f;

	void OnDialogueNext();

	std::function<void()> m_onDialogueEndCallback = nullptr;
};