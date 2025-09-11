#pragma once
#include "NagiocpXPch.h"
#include "EBR.hpp"
#include "BackOff.h"
#include "NagoxAtomic.h"
#include "VectorHash.hpp"

namespace NagiocpX
{
	extern thread_local VectorSetUnsafe<std::pair<uint32_t, const ContentsEntity*>, XHashMap> new_view_list_session;

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
			// 새로운 노드를 만들고
			// 그 노드를 새 tail로 atomic하게 바꾼다.
			static_cast<Node* const>(
				InterlockedExchangePointer(reinterpret_cast<volatile PVOID*>(&tail), value)
				)->next.store(value);
			// 반환되는 값은 바꾸기 전 tail (oldTail) 이므로 해당 노드의 next에 방금 만든 tail을 연결
			// 
			// 예상되는 문제점
			// tail만 바꾸고, 이전 노드의 next를 아직 바꾸지 못한 채 컨텍스트 스위칭 발생?
			// [oldTail] ->(nullptr)  ... [newTail] (아직 oldTail의 next가 nullptr을 가리키는 상태)
			// 이런 일이 여러 스레드에서 동시 다발적으로 발생한다면?
			// [oldTail 0] [oldTail 1] [oldTail 2] [oldTail 3] .... [newTail] 
			// 새롭게 추가된 노드들의 next가 죄다 nullptr을 가리키는 상태
			// -> 어차피 각 스레드는 반환값으로 자신이 next를 바꿔야할 노드를 정확히 알고있음
			// 컨텍스트 스위칭에서 돌아오면 자연스럽게 next를 연결
			// pop하는 스레드는 노드의 next가 nullptr이면 empty로 간주함
			// 이 상태는 아직 큐에 삽입이 이루어지지 않았다고 생각하면 된다.
			// 
			// 만약 이 상태가 마음에 들지 않아서 tail의 next를 먼저 바꾸고 그 다음 tail을 이동시킨다면?
			// 0. head와 tail이 같은 곳을 가리키는 상태에서 [head/tail]
			// 1. 스레드0이 tail의 next를 새롭게 만든 노드로 연결 시키고 컨텍스트 스위칭 됨
			//  [tail] -> [newTail] // 아직 tail은 바뀌지않음
			// 2. 스레드1이 pop하기 위해 tail(head)의 next가 nullptr이 아님을 확인함
			// 3. 스레드1이 데이터를 꺼내고 head를 head->next로 바꾸고 이전 head를 delete (head와 tail이 같은 곳을 가리키는 상태였음)
			// 4. [tail (delete 됨)] ->(???) [head] 상태가 됨
			// 5. 스레드2가 push하기 위해 tail의 next를 참조함
			// 6. 스레드2가 접근하는 tail이 가리키는 곳은 이미 스레드1이 delete한 메모리이므로 undefined behavior 발생
			// 
			
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
				// pop하는 스레드가 반드시 자기자신 뿐 == 자기 이외엔 절대로 누가 노드를 delete하지 않음
				// == 이 로직에 문제가 없다면 절대로 해제된 메모리를 참조할 일이 없음
				// 
				// ABA문제: 내가 접근하려는 메모리를 누가 delete하고 또 다른 누군가가 똑같은 주소값으로 할당해서 생김
				// -> 내가 접근하려는 메모리를 다른 누군가가 delete 할 일이 없다. == ABA문제가 없다. 
				
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
				extern thread_local VectorSetUnsafe<std::pair<uint32_t, const ContentsEntity*>, XHashMap> new_view_list_session;
				auto& wsa_bufs_vec = new_view_list_session.GetItemListRef();
				static_assert(sizeof(WSABUF) == sizeof(wsa_bufs_vec[0]));
				wsa_bufs_vec.clear();
				while (try_pop_single(vec_, head_temp)){
					const auto& sb = vec_.back();
					wsa_bufs_vec.push_back(reinterpret_cast<std::pair<uint32_t, const ContentsEntity*>&&>
						(WSABUF{ sb->WriteSize(),reinterpret_cast<char* const>(sb->Buffer() )}));
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
			return nullptr == head.load(std::memory_order_relaxed)->next.load();
		}
		void clear_single()noexcept { reset(); }

		auto& head_for_single_pop()noexcept { return head; }
	};
}