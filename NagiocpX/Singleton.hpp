#pragma once

namespace NagiocpX
{
	template <typename T>
	class Singleton
	{
	private:
		Singleton(const Singleton&) = delete;
		Singleton& operator=(const Singleton&) = delete;
		Singleton(Singleton&&)noexcept = delete;
		Singleton& operator=(Singleton&&)noexcept = delete;
	private:
		struct alignas(std::hardware_constructive_interference_size) SingletonBlock { 
			SingletonBlock()noexcept;
			alignas(std::hardware_constructive_interference_size) std::byte singleton_block[sizeof(T)];
		};
		__declspec(align(std::hardware_constructive_interference_size)) static SingletonBlock g_singleton_block;
	protected:
		Singleton()noexcept = default;
		~Singleton()noexcept = default;
	public:
		void Init()noexcept{}
		static void RegisterDestroy()noexcept { std::atexit([]()noexcept { std::launder(reinterpret_cast<T* const>(&g_singleton_block))->~T(); }); }
		constexpr static inline T* const GetInst() noexcept { return reinterpret_cast<T* const>(&g_singleton_block); }
	};
	template<typename T>
	Singleton<T>::SingletonBlock Singleton<T>::g_singleton_block;
	template<typename T>
	inline Singleton<T>::SingletonBlock::SingletonBlock() noexcept { new (&singleton_block) T; }
}
