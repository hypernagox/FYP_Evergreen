#pragma once
#include "pch.h"
#include "component.h"


class DelegaterVoid
{
public:
	template <typename U>
	void operator +=(U&& add_func)noexcept { m_vecKeyFunc.emplace_back(std::forward<U>(add_func)); }
	void Reset()noexcept { m_vecKeyFunc.clear(); }
	void operator()()const noexcept { for (const auto& fp : m_vecKeyFunc)fp(); }
	operator bool()const noexcept { return !m_vecKeyFunc.empty(); }
private:
	std::vector<std::function<void()>> m_vecKeyFunc;
};

enum KEY_STATE
{
	KET_TAP,
	KEY_HOLD,
	KEY_AWAY,

	END,
};

class InputHandler
	:public udsdx::Component
{
public:
	InputHandler(const std::shared_ptr<udsdx::SceneObject>& object)noexcept :udsdx::Component{ object } {}
public:
	using KeyFunc = std::function<void(void)>;
public:
	virtual void Update(const udsdx::Time& time, udsdx::Scene& scene)override;
public:
	void Reset()noexcept { for (auto& table : m_keyTable)table.clear(); }
	void SwapKey(Keyboard::Keys a_key, Keyboard::Keys b_key)noexcept 
	{ 
		for (auto& key_table : m_keyTable)
		{
			const auto e_iter = key_table.cend();
			const auto iter_a = key_table.find(a_key);
			const auto iter_b = key_table.find(b_key);
			std::swap(key_table[a_key], key_table[b_key]);
			if (e_iter == iter_a)key_table.erase(b_key);
			if (e_iter == iter_b)key_table.erase(a_key);
		}
	}
	void AddKeyFunc(Keyboard::Keys key, KEY_STATE key_state, KeyFunc key_func)noexcept { m_keyTable[key_state][key] += std::move(key_func); }
	template<typename Fp,typename... Args>
	void AddKeyFunc(Keyboard::Keys key, KEY_STATE key_state, Fp&& fp, Args&&... args)noexcept { m_keyTable[key_state][key] += [fp, args...]()noexcept {std::invoke(fp, (args)...); }; }
	bool IsKeyHit()const noexcept;
private:
	using KeyTable = std::unordered_map<Keyboard::Keys, DelegaterVoid>;

	KeyTable m_keyTable[KEY_STATE::END];

	static bool(* const g_keyCheckFunc[KEY_STATE::END])(const Keyboard::Keys&)noexcept;
	
};

