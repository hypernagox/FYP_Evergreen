#pragma once

#include "pch.h"

class InteractiveEntity : public udsdx::Component
{
public:
	InteractiveEntity(const std::shared_ptr<udsdx::SceneObject>& owner);
	
	void OnInteract();
	void OnInteractRange(bool inRange);
	std::wstring GetInteractionText() const { return m_interactionText; }
	void SetInteractionText(const std::wstring& text) { m_interactionText = text; }
	DirectX::Keyboard::Keys GetInteractionKey() const { return m_interactionKey; }
	void SetInteractionKey(DirectX::Keyboard::Keys key) { m_interactionKey = key; }
	udsdx::RendererBase* GetTargetRenderer() const { return m_targetRenderer; }
	void SetTargetRenderer(udsdx::RendererBase* renderer) { m_targetRenderer = renderer; }
	void SetInteractionCallback(std::function<void()> callback) { m_interactionCallback = callback; }

private:
	udsdx::RendererBase* m_targetRenderer = nullptr;
	std::wstring m_interactionText = L"Interact";
	DirectX::Keyboard::Keys m_interactionKey = DirectX::Keyboard::Keys::E;
	std::function<void()> m_interactionCallback;
};