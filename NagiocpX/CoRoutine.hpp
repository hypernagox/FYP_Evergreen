#pragma once
#include "NagiocpXPch.h"

namespace NagiocpX
{
	class CoRoutine
	{
	public:
		struct promise_type;
		using handle = std::coroutine_handle<promise_type>;
		struct  promise_type
		{
			static void* operator new(const std::size_t size) { return Memory::Alloc(size); }
			static void operator delete(void* const pointer, const std::size_t size) { return Memory::Free_Sized(pointer, static_cast<const uint32_t>(size)); }
			CoRoutine get_return_object() { return CoRoutine{ handle::from_promise(*this) }; }
			auto initial_suspend() { return std::suspend_always{}; }
			auto final_suspend()noexcept { return std::suspend_always{}; }
			void return_void() {}
			void unhandled_exception() { assert(false); }
		};
		handle m_coHandle = nullptr;
		CoRoutine() = default;
		~CoRoutine()
		{
			if (nullptr != m_coHandle)
				m_coHandle.destroy();
			m_coHandle = nullptr;
		}
		CoRoutine(const CoRoutine&) = delete;
		CoRoutine& operator=(const CoRoutine&) = delete;
		CoRoutine(CoRoutine&& _other) noexcept
		{
			if (nullptr != m_coHandle)
				m_coHandle.destroy();
			m_coHandle = _other.m_coHandle;
			_other.m_coHandle = nullptr;
		}
		CoRoutine& operator = (CoRoutine&& _other)noexcept
		{
			if (nullptr != m_coHandle)
				m_coHandle.destroy();
			m_coHandle = _other.m_coHandle;
			_other.m_coHandle = nullptr;
			return *this;
		}
		CoRoutine(handle _h) :m_coHandle{ _h } {}
		operator bool()
		{
			if (nullptr != m_coHandle)
				return true;
			else
				return false;
		}
		bool done()const { return m_coHandle.done(); }
	private:
		void reset()
		{
			if (nullptr != m_coHandle)
				m_coHandle.destroy();
			m_coHandle = nullptr;
		}
	public:
		bool resume()
		{
			if (nullptr != m_coHandle && !m_coHandle.done())
			{
				m_coHandle.resume();
				return true;
			}
			else
			{
				reset();
				return false;
			}
		}
	};
}