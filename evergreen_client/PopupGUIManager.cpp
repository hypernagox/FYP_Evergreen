#include "pch.h"
#include "PopupGUIManager.h"

using namespace udsdx;

PopupGUIManager::PopupGUIManager(const std::shared_ptr<udsdx::SceneObject>& object) : Component(object)
{
}

std::vector<PopupGUIManager::PopupElement>::iterator PopupGUIManager::Find(const std::shared_ptr<udsdx::SceneObject>& popup)
{
	return std::find_if(m_popupContainer.begin(), m_popupContainer.end(), [&popup](const PopupElement& element) { return element.PopupObject == popup; });
}

void PopupGUIManager::Append(const std::shared_ptr<udsdx::SceneObject>& popup, const std::shared_ptr<udsdx::SceneObject>& page)
{
	auto it = Find(popup);
	if (it != m_popupContainer.end())
	{
		// Popup already exists, and no new page is provided.
		if (page == nullptr)
		{
			return;
		}
		// The popup tries to add a new page. Deactivate the last page and append the new page.
		else
		{
			if (!it->PopupPages.empty())
			{
				it->PopupPages.back()->SetActive(false);
			}
			it->PopupPages.emplace_back(page);
			page->SetActive(true);
		}
	}
	else
	{
		bool isFirstPopup = m_popupContainer.empty();

		popup->SetActive(true);
		auto& element = m_popupContainer.emplace_back();

		element.PopupObject = popup;
		if (nullptr != page)
		{
			page->SetActive(true);
			element.PopupPages.emplace_back(page);
		}
		else
		{
			element.PopupPages.emplace_back(popup);
		}

		if (isFirstPopup && nullptr != m_onFocusChangedCallback)
		{
			m_onFocusChangedCallback(false);
		}
	}

	PlaySound(true);
}

void PopupGUIManager::Pop(const std::shared_ptr<udsdx::SceneObject>& popup, bool ignorePages)
{
	auto it = Find(popup);
	if (it == m_popupContainer.end())
	{
		return;
	}

	if (ignorePages)
	{
		while (!it->PopupPages.empty())
		{
			it->PopupPages.back()->SetActive(false);
			it->PopupPages.pop_back();
		}
	}
	else
	{
		it->PopupPages.back()->SetActive(false);
		it->PopupPages.pop_back();

		if (!it->PopupPages.empty())
		{
			it->PopupPages.back()->SetActive(true);
		}
	}

	if (it->PopupPages.empty())
	{
		it->PopupObject->SetActive(false);
		m_popupContainer.erase(it);
	}

	if (m_popupContainer.empty() && nullptr != m_onFocusChangedCallback)
	{
		m_onFocusChangedCallback(true);
	}

	PlaySound(false);
}

void PopupGUIManager::Pop(bool ignorePages)
{
	if (m_popupContainer.empty())
	{
		if (nullptr != m_onPopEmptyCallback)
		{
			m_onPopEmptyCallback();
		}
		return;
	}

	auto& target = m_popupContainer.back();

	if (ignorePages)
	{
		while (!target.PopupPages.empty())
		{
			target.PopupPages.back()->SetActive(false);
			target.PopupPages.pop_back();
		}
	}
	else
	{
		target.PopupPages.back()->SetActive(false);
		target.PopupPages.pop_back();

		if (!target.PopupPages.empty())
		{
			target.PopupPages.back()->SetActive(true);
		}
	}

	if (target.PopupPages.empty())
	{
		target.PopupObject->SetActive(false);
		m_popupContainer.pop_back();
	}

	PlaySound(false);

	if (m_popupContainer.empty())
	{
		if (nullptr != m_onFocusChangedCallback)
		{
			m_onFocusChangedCallback(true);
		}
	}
}

void PopupGUIManager::PopAll()
{
	if (m_popupContainer.empty())
	{
		return;
	}

	while (!m_popupContainer.empty())
	{
		auto& popupElement = m_popupContainer.back();
		while (!popupElement.PopupPages.empty())
		{
			popupElement.PopupPages.back()->SetActive(false);
			popupElement.PopupPages.pop_back();
		}

		m_popupContainer.pop_back();
	}

	if (nullptr != m_onFocusChangedCallback)
	{
		m_onFocusChangedCallback(true);
	}

	PlaySound(false);
}

void PopupGUIManager::PlaySound(bool mode)
{
	if (mode)
		m_menuSound = INSTANCE(Resource)->Load<udsdx::AudioClip>(RESOURCE_PATH(L"audio\\uiopen.wav"))->CreateInstance();
	else
		m_menuSound = INSTANCE(Resource)->Load<udsdx::AudioClip>(RESOURCE_PATH(L"audio\\uiclose.wav"))->CreateInstance();
	m_menuSound->SetVolume(0.5f);
	m_menuSound->Play();
}
