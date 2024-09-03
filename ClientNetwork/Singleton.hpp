#pragma once

namespace NetHelper
{
	template <typename T>
	class Singleton
		:public std::enable_shared_from_this<Singleton<T>>
	{
	private:
		Singleton(const Singleton&) = delete;
		Singleton& operator=(const Singleton&) = delete;
		Singleton(Singleton&&)noexcept = delete;
		Singleton& operator=(Singleton&&)noexcept = delete;
	private:
		constexpr static inline T* const InitInstance()noexcept {
			static const std::shared_ptr<T>* const mgr{ new (std::nothrow) std::shared_ptr<T> { new (std::nothrow) T{},[](T* const ptr) noexcept { delete ptr; } } };
			std::atexit([]()noexcept { delete mgr; });
			return mgr->get();
		}
	protected:
		Singleton()noexcept = default;
		virtual ~Singleton()noexcept = default;
	public:
		constexpr static inline T* const GetInst() noexcept
		{
			static const auto mgrPtr = InitInstance();
			return mgrPtr;
		}
		std::shared_ptr<T> shared_from_this()noexcept { return std::shared_ptr<T>{this->std::enable_shared_from_this<Singleton<T>>::shared_from_this(), static_cast<T* const>(this)}; }
		std::shared_ptr<const T> shared_from_this()const noexcept { return std::shared_ptr<const T>{this->std::enable_shared_from_this<Singleton<T>>::shared_from_this(), static_cast<const T* const>(this)}; }
		virtual void Init()noexcept {}
	};
}