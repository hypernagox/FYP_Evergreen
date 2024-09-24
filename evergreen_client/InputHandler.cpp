#include "pch.h"
#include "InputHandler.h"
#include "input.h"

bool(* const InputHandler::g_keyCheckFunc[KEY_STATE::END])(const Keyboard::Keys&)noexcept
{
	[](const Keyboard::Keys& k)noexcept {return INSTANCE(udsdx::Input)->GetKeyDown(k); },
	[](const Keyboard::Keys& k)noexcept {return INSTANCE(udsdx::Input)->GetKey(k); },
	[](const Keyboard::Keys& k)noexcept {return INSTANCE(udsdx::Input)->GetKeyUp(k); },
};

void InputHandler::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	const auto scene_object = GetSceneObject();

	for (int i = 0; i < KEY_STATE::END; ++i)
	{
		for (const auto& [key, func] : m_keyTable[i])
		{
			if (!func)continue;
			if (g_keyCheckFunc[i](key))
			{
				func();
			}
		}
	}
}

bool InputHandler::IsKeyHit() const noexcept
{
	for (const auto& key : m_keyTable[KEY_STATE::KET_TAP] | std::views::keys) 
	{
		if (g_keyCheckFunc[KEY_STATE::KET_TAP](key))
			return true;
	}
	for (const auto& key : m_keyTable[KEY_STATE::KEY_AWAY] | std::views::keys)
	{
		if (g_keyCheckFunc[KEY_STATE::KEY_AWAY](key))
			return true;
	}
	return false;
}

