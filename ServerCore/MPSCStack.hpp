#pragma once
#include "ServerCorePch.h"
#include "NagoxAtomic.h"
#include "BackOff.h"

namespace ServerCore
{
	template<typename T>
	class MPSCStack
	{
		struct Node
		{
			T data;
			NagoxAtomic::Atomic<Node*> prev;
			template<typename... Args>
			constexpr Node(Args&&... args)noexcept :data{ std::forward<Args>(args)... } {}
		};
	public:
		void clear()noexcept {
			Node* node = m_top;
			m_top = nullptr;
			while (node) {
				Node* const next = node->prev;
				xdelete<Node>(node);
				node = next;
			}
		}
		~MPSCStack()noexcept { clear(); }
	public:
		template <typename... Args>
		void emplace(Args&&... args) noexcept {
			const BackOff bo{ NUM_OF_THREADS / 2 };
			Node* const newTop = xnew<Node>(std::forward<Args>(args)...);
			for (;;)
			{
				Node* const oldTop = m_top;
				newTop->prev.store_relaxed(oldTop);
				if (oldTop != m_top)continue;
				if (oldTop == InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&m_top), newTop, oldTop))
				{
					return;
				}
				else
				{
					bo.delay();
				}
			}
		}
		const bool try_pop_single(T& target)noexcept {
			for (;;)
			{
				Node* const oldTop = m_top;
				if (nullptr == oldTop)return false;
				Node* const newTop = oldTop->prev.load_relaxed();
				if (oldTop != m_top)continue;
				if (oldTop == InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&m_top), newTop, oldTop))
				{
					target = std::move(oldTop->data);
					xdelete<Node>(oldTop);
					return true;
				}
			}
		}
		const bool empty_single()const noexcept { return nullptr == m_top; }
	private:
		Node* volatile m_top = nullptr;
	};

	template<typename T>
	class MPSCBoundedStack
	{

	public:
		MPSCBoundedStack(const LONG size_)noexcept
			: m_arrStack{ (T*)Memory::AlignedAlloc(sizeof(T) * size_,64) }
			, m_maxSize{ size_ }
			, m_curTop{ -1 }
		{
			std::ranges::fill(m_arrStack, m_arrStack + m_maxSize, T{});
		}
		~MPSCBoundedStack()noexcept {
			std::ranges::destroy(m_arrStack, m_arrStack + m_maxSize);
			Memory::AlignedFree(m_arrStack, 64);
		}
	public:
		template <typename... Args>
		void emplace(Args&&... args) noexcept {
			T value{ std::forward<Args>(args)... };
			m_arrStack[InterlockedIncrement(&m_curTop)] = std::move(value);
		}
		const bool try_pop_single(T& target)noexcept {
			for (;;)
			{
				const auto curTop = m_curTop;
				if (-1 == curTop)return false;
				auto temp = m_arrStack[curTop];
				if (curTop != m_curTop)continue;
				if (curTop == InterlockedCompareExchange(&m_curTop, curTop - 1, curTop))
				{
					target = std::move(temp);
					return true;
				}
			}
		}
		const bool empty_single()const noexcept { return -1 == m_curTop; }
	private:
		alignas(8) const LONG m_maxSize;
		alignas(8) volatile LONG m_curTop = -1;
		alignas(8) T* const m_arrStack;
	};
}