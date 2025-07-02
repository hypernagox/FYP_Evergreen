#pragma once

#include "pch.h"

class PopupGUIManager : public udsdx::Component
{
private:
	struct PopupElement
	{
		std::shared_ptr<udsdx::SceneObject> PopupObject;
		std::vector<std::shared_ptr<udsdx::SceneObject>> PopupPages;
	};

public:
	PopupGUIManager(const std::shared_ptr<udsdx::SceneObject>& object);

	void Append(const std::shared_ptr<udsdx::SceneObject>& popup, const std::shared_ptr<udsdx::SceneObject>& page = nullptr);
	void Pop(const std::shared_ptr<udsdx::SceneObject>& popup, bool ignorePages = false);
	void Pop(bool ignorePages = false);
	void PopAll();

	bool IsEmpty() const { return m_popupContainer.empty(); }
	void SetOnPopEmptyCallback(const std::function<void()>& callback) { m_onPopEmptyCallback = callback; }
	void SetOnFocusChangedCallback(const std::function<void(bool)>& callback) { m_onFocusChangedCallback = callback; }

private:
	std::vector<PopupElement>::iterator Find(const std::shared_ptr<udsdx::SceneObject>& popup);
	void PlaySound(bool mode);

private:
	std::vector<PopupElement> m_popupContainer;
	std::function<void()> m_onPopEmptyCallback;
	std::function<void(bool)> m_onFocusChangedCallback;
	std::unique_ptr<SoundEffectInstance> m_menuSound;
};