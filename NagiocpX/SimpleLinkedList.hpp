#pragma once

namespace NagiocpX
{
	template<typename T>
	class SimpleLinkedList
	{
	private:
		struct Node {
			T data;
			Node* const next;
		};
		struct HeadNode { Node* volatile next = nullptr; };
		class SLLIter {
		public:
			SLLIter()noexcept = default;
			~SLLIter()noexcept = default;
			SLLIter(Node* const node)noexcept :curNode{ node } {}
		public:
			T& operator*()const noexcept { return curNode->data; }
			T* operator->()const noexcept { return &curNode->data; }
			SLLIter& operator++()noexcept { curNode = curNode->next; return *this; }
			SLLIter operator++(int)noexcept { SLLIter temp{ *this }; curNode = curNode->next; return temp; }
			bool operator==(const SLLIter& other)const noexcept { return curNode == other.curNode; }
		private:
			Node* curNode = nullptr;
		};
	public:
		SimpleLinkedList()noexcept = default;
		~SimpleLinkedList()noexcept { GetTLList().merge_list(head); }
		SimpleLinkedList(const SimpleLinkedList&) = delete;
		SimpleLinkedList& operator=(const SimpleLinkedList&) = delete;
	public:
		template<typename... Args>
		void emplace(Args&&... args)noexcept { 
			auto& tl_pool = GetTLList();
			head.next = tl_pool.pop_front(std::forward<Args>(args)..., head.next);
		}
	
		void swap(SimpleLinkedList& other)noexcept {
			auto& other_head = other.head.next;
			const auto temp = other_head;
			other_head = head.next;
			head.next = temp;
		}
		bool empty()const noexcept { return nullptr == head.next; }
	public:
		auto begin()const noexcept { return SLLIter{ head.next }; }
		auto end()const noexcept { return SLLIter{ nullptr }; }
	private:
		void clear()noexcept {
			while (Node* const curNode = head.next) {
				head.next = curNode->next;
				ReleaseNode(curNode);
			}
		}
		struct SLLPool {
			~SLLPool()noexcept { tl_list.clear(); }
			template<typename... Args>
			Node* const pop_front(Args&&... args)noexcept {
				if (auto& head_next = tl_list.head.next)
				{
					const auto curNode = head_next;
					head_next = curNode->next;
					return std::construct_at<Node>(curNode, std::forward<Args>(args)...);
				}
				else
					return AllocNode(std::forward<Args>(args)...);
			}
			void merge_list(HeadNode& head_node)noexcept { tl_list.head.next = head_node.next; }
			SimpleLinkedList tl_list;
		};
		static auto& GetTLList()noexcept { thread_local SLLPool tl_list; return tl_list; }
	private:
		template<typename... Args>
		static Node* AllocNode(Args&&... args)noexcept { return xnew<Node>(std::forward<Args>(args)...); }
		static void ReleaseNode(Node* const target_node)noexcept { xdelete<Node>(target_node); }
	private:
		HeadNode head;
	};
}