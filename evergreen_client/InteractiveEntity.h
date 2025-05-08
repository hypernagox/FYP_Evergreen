#pragma once

#include "pch.h"

class InteractiveEntity : public udsdx::Component
{
public:
	InteractiveEntity(const std::shared_ptr<udsdx::SceneObject>& owner);
	
	void OnInteract();
	std::wstring GetInteractionText() const { return m_interactionText; }
	void SetInteractionText(const std::wstring& text) { m_interactionText = text; }
	DirectX::Keyboard::Keys GetInteractionKey() const { return m_interactionKey; }
	void SetInteractionKey(DirectX::Keyboard::Keys key) { m_interactionKey = key; }
	void SetInteractionCallback(std::function<void()> callback) { m_interactionCallback = callback; }

private:
	std::wstring m_interactionText = L"Interact";
	DirectX::Keyboard::Keys m_interactionKey = DirectX::Keyboard::Keys::E;
	std::function<void()> m_interactionCallback;
};