#pragma once

#include "pch.h"

#define INSTANCE(T) udsdx::Singleton<T>::GetInstance()

namespace udsdx
{
	template <typename T> requires std::default_initializable<T>
	class Singleton
	{
	private:
		static std::unique_ptr<T> instance;

	public:
		static T* GetInstance()
		{
			if (!HasInstance())
				CreateInstance<T>();

			return instance.get();
		}

		template <typename Derived_T = T> requires std::derived_from<Derived_T, T>
		static T* CreateInstance()
		{
			instance = std::make_unique<Derived_T>();
			return instance.get();
		}

		static void ReleaseInstance()
		{
			instance.release();
		}

		static bool HasInstance()
		{
			return static_cast<bool>(instance);
		}
	};

	template <typename T> requires std::default_initializable<T>
	std::unique_ptr<T> Singleton<T>::instance;
}