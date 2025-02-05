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
			const BackOff bo;
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
}