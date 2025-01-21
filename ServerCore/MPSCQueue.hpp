#pragma once
#include "ServerCorePch.h"
#include "EBR.hpp"
#include "BackOff.h"
#include "NagoxAtomic.h"
#include "VectorHash.hpp"

namespace ServerCore
{
	extern thread_local VectorSetUnsafe<std::pair<uint32_t, const ContentsEntity*>> new_view_list_session;

	template <typename T>
	class MPSCQueue
	{
	private:
		struct Node
		{
			T data;
			NagoxAtomic::Atomic<Node*> next{ nullptr };
			template<typename... Args>
			constexpr Node(Args&&... args)noexcept :data{ std::forward<Args>(args)... }, next{ nullptr } {}
		};
		Node* volatile tail;
		int64_t pad[7];
		std::atomic<Node*> head;
	private:
		void reset()noexcept {
			Node* curHead = head.load(std::memory_order_seq_cst);
			const Node* const curTail = tail;
			while (curTail != curHead)
			{
				Node* const delHead = curHead;
				curHead = curHead->next.load();
				xdelete<Node>(delHead);
			}
			curHead->next.store_relaxed(nullptr);
			tail = curHead;
			head.store(curHead, std::memory_order_release);
		}
	public:
		MPSCQueue()noexcept
			: tail{ xnew<Node>() }
			, head{ tail }
		{
		}
		~MPSCQueue()noexcept {
			reset();
			xdelete<Node>(head.load(std::memory_order_relaxed));
		}
		template <typename... Args>
		void emplace(Args&&... args) noexcept {
			//const BackOff bo{ NUM_OF_THREADS / 2 };
			Node* const value = xnew<Node>(std::forward<Args>(args)...);
			static_cast<Node* const>(
				InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&tail), value)
				)->next.store(value);
			//for (;;)
			//{
			//	Node* const oldTail = tail;
			//	if (oldTail != tail)continue;
			//	if (oldTail == InterlockedCompareExchangePointer(reinterpret_cast<volatile PVOID*>(&tail), value, oldTail))
			//	{
			//		oldTail->next.store(value);
			//		return;
			//	}
			//	bo.delay();
			//}
		}
		const bool try_pop_single(T& _target)noexcept {
			Node* const head_temp = head.load(std::memory_order_relaxed);
			if (Node* const __restrict newHead = head_temp->next.load())
			{
				Node* const oldHead = head_temp;
				head.store(newHead, std::memory_order_release);
				if constexpr (std::same_as<T, Task> || !std::swappable<T>)
					_target = std::move(newHead->data);
				else if constexpr (std::is_trivial_v<T>)
					_target = newHead->data;
				else
					_target.swap(newHead->data);
				xdelete<Node>(oldHead);
				return true;
			}
			return false;
		}
		const bool try_pop_single(XVector<T>& _targetForPushBack)noexcept {
			Node* const head_temp = head.load(std::memory_order_relaxed);
			if (Node* const __restrict newHead = head_temp->next.load())
			{
				Node* const oldHead = head_temp;
				head.store(newHead, std::memory_order_release);
				_targetForPushBack.emplace_back(std::move(newHead->data));
				xdelete<Node>(oldHead);
				return true;
			}
			return false;
		}
		const bool try_pop_single(T& _target, Node*& head_temp)noexcept {
			if (Node* const __restrict newHead = head_temp->next.load())
			{
				Node* const oldHead = head_temp;
				head_temp = newHead;
				if constexpr (std::same_as<T, Task> || !std::swappable<T>)
					_target = std::move(newHead->data);
				else
					_target.swap(newHead->data);
				xdelete<Node>(oldHead);
				return true;
			}
			return false;
		}
		const bool try_pop_single(XVector<T>& _targetForPushBack, Node*& head_temp)noexcept {
			if (Node* const __restrict newHead = head_temp->next.load())
			{
				Node* const oldHead = head_temp;
				head_temp = newHead;
				_targetForPushBack.emplace_back(std::move(newHead->data));
				xdelete<Node>(oldHead);
				return true;
			}
			return false;
		}
		const bool try_pop_single(std::vector<T>& _targetForPushBack, Node*& head_temp)noexcept {
			if (Node* const __restrict newHead = head_temp->next.load())
			{
				Node* const oldHead = head_temp;
				head_temp = newHead;
				_targetForPushBack.emplace_back(std::move(newHead->data));
				xdelete<Node>(oldHead);
				return true;
			}
			return false;
		}
		XVector<T> try_flush_single()noexcept {
			Node* head_temp = head.load(std::memory_order_seq_cst);
			XVector<T> vec; vec.reserve(32); while (try_pop_single(vec, head_temp));
			head.store(head_temp, std::memory_order_release);
			return vec;
		}
		void try_flush_single(XVector<T>& vec_)noexcept {
			Node* head_temp = head.load(std::memory_order_seq_cst);
			if constexpr (std::same_as<std::decay_t<T>, S_ptr<SendBuffer>>){
				extern thread_local VectorSetUnsafe<std::pair<uint32_t, const ContentsEntity*>> new_view_list_session;
				auto& wsa_bufs_vec = new_view_list_session.GetItemListRef();
				wsa_bufs_vec.clear();
				constexpr const size_t PKT_LIMIT_COUNT = 3072;
				while (try_pop_single(vec_, head_temp)){
					const auto& sb = vec_.back();
					wsa_bufs_vec.push_back(reinterpret_cast<std::pair<uint32_t, const ContentsEntity*>&&>
						(WSABUF{ sb->WriteSize(),reinterpret_cast<char* const>(sb->Buffer() )}));
					if (PKT_LIMIT_COUNT == vec_.size()) [[unlikely]] {
						//std::cout << "Send Limit\n";
						break;
					}
				}
			}
			else {
				while (try_pop_single(vec_, head_temp));
			}
			head.store(head_temp, std::memory_order_release);
		}
		//void try_flush_single(std::vector<T>& vec_)noexcept {
		//	Node* head_temp = head.load(std::memory_order_seq_cst);
		//	if constexpr (std::same_as<std::decay_t<T>, S_ptr<SendBuffer>>) {
		//		extern thread_local XVector<WSABUF> wsaBufs;
		//		wsaBufs.clear();
		//		while (try_pop_single(vec_, head_temp)) {
		//			const auto& sb = vec_.back();
		//			wsaBufs.emplace_back(static_cast<const ULONG>(sb->WriteSize()), reinterpret_cast<char* const>(sb->Buffer()));
		//		}
		//	}
		//	else {
		//		while (try_pop_single(vec_, head_temp));
		//	}
		//	head.store(head_temp, std::memory_order_release);
		//}
		const bool empty_single()const noexcept {
			return tail == head.load(std::memory_order_relaxed);
		}
		void clear_single()noexcept { reset(); }

		auto& head_for_single_pop()noexcept { return head; }
	};
}