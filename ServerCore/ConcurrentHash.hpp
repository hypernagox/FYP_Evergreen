#pragma once
#include "ServerCorePch.h"
#include "SRWLock.hpp"

namespace ServerCore
{
	template <typename Key, typename Value>
	class ConcurrentListForMap
	{
	public:
		struct Node
		{
			std::pair<Key, Value> data;
			Node* next;
			constexpr Node()noexcept :next{ nullptr } {}
			constexpr Node(std::pair<Key, Value>&& data_)noexcept :data{ std::move(data_) } {}
			constexpr Node(std::pair<Key, Value>&& data_, Node* const next_)noexcept :data{ std::move(data_) }, next{ next_ } {}
		};
	private:
		Node head;
		//mutable SRWLock m_sharedMutex;
		mutable SRWLock m_sharedMutexForErase;
		mutable std::mutex m_eraseLock;
	private:
		void reset()noexcept
		{
			while (Node* const oldHead = head.next)
			{
				head.next = oldHead->next;
				xdelete<Node>(oldHead);
			}
		}
	public:
		ConcurrentListForMap() noexcept = default;
		~ConcurrentListForMap() noexcept
		{
			reset();
		}
		ConcurrentListForMap(ConcurrentListForMap&& other) noexcept
			: head(std::move(other.head))
		{
		}
		template<typename ...Args>
		void emplace_front_no_return(const Key& key_, Args&&... args)noexcept
		{
			//Node* const newNode = ::xnew<Node>(std::make_pair(key_, std::forward<Args>(args)...));
			//{
			//	std::lock_guard<SRWLock> s_lock{ m_sharedMutex };
			//	newNode->next = head.next;
			//	head.next = newNode;
			//}
			Node* const newNode = xnew<Node>(std::make_pair(key_, std::forward<Args>(args)...));
			std::shared_lock<SRWLock> shared_erase{ m_sharedMutexForErase };
			do {
				newNode->next = head.next;
			} while (InterlockedCompareExchangePointer(
				reinterpret_cast<PVOID volatile*>(&head.next),
				newNode,
				newNode->next) != newNode->next);
		}
		template<typename ...Args>
		std::optional<Value> emplace_front(const Key& key_, Args&&... args)noexcept
		{
			//Node* const newNode = ::xnew<Node>(std::make_pair(key_, std::forward<Args>(args)...));
			//{
			//	std::lock_guard<SRWLock> s_lock{ m_sharedMutex };
			//	newNode->next = head.next;
			//	head.next = newNode;
			//}
			Node* const newNode = xnew<Node>(std::make_pair(key_, std::forward<Args>(args)...));
			std::shared_lock<SRWLock> shared_erase{ m_sharedMutexForErase };
			do {
				newNode->next = head.next;
			} while (InterlockedCompareExchangePointer(
				reinterpret_cast<PVOID volatile*>(&head.next),
				newNode,
				newNode->next) != newNode->next);
			return newNode->data.second;
		}
		template<typename ...Args>
		void emplace_unsafe(const Key& key_, Args&&... args)noexcept
		{
			Node* const newNode = xnew<Node>(std::make_pair(key_, std::forward<Args>(args)...), head.next);
			head.next = newNode;
			std::atomic_thread_fence(std::memory_order_release);
		}
		template<typename ...Args>
		const bool find_and_emplace(const Key& key_, Args&&... args) noexcept
		{
			const Node* curNode = &head;
			{
				std::lock_guard<SRWLock> find_lock{ m_sharedMutexForErase };
				curNode = head.next;
				while (curNode)
				{
					if (key_ == curNode->data.first)
					{
						break;
					}
					curNode = curNode->next;
				}
				if (!curNode)
				{
					Node* const newNode = xnew<Node>(std::make_pair(key_, std::forward<Args>(args)...), head.next);
					head.next = newNode;
				}
			}
			return !curNode;
		}
		std::optional<Value> find(const Key& key_) noexcept
		{
			const Node* curNode = &head;
			std::optional<Value> targetData = std::nullopt;
			{
				//std::shared_lock<SRWLock> s_lock{ m_sharedMutex };
				std::shared_lock<SRWLock> shared_erase{ m_sharedMutexForErase };
				curNode = head.next;
				while (curNode)
				{
					if (key_ == curNode->data.first)
					{
						targetData.emplace(curNode->data.second);
						break;
					}
					curNode = curNode->next;
				}
			}
			return targetData;
		}
		std::optional<Value> find(const Key& key_)const noexcept
		{
			const Node* curNode = &head;
			std::optional<Value> targetData = std::nullopt;
			{
				//std::shared_lock<SRWLock> s_lock{ m_sharedMutex };
				std::shared_lock<SRWLock> shared_erase{ m_sharedMutexForErase };
				curNode = head.next;
				while (curNode)
				{
					if (key_ == curNode->data.first)
					{
						targetData.emplace(curNode->data.second);
						break;
					}
					curNode = curNode->next;
				}
			}
			return targetData;
		}
		void erase(const Key& key_)noexcept
		{
			Node* prevNode = &head;
			Node* curNode;
			bool flag = false;
			{
				std::lock_guard<std::mutex> erase_lock{ m_eraseLock };
				//std::shared_lock<SRWLock> s_lock{ m_sharedMutex };
				curNode = prevNode->next;
				while (curNode)
				{
					if (key_ == curNode->data.first)
					{
						flag = true;
						std::lock_guard<SRWLock> lock{ m_sharedMutexForErase };
						prevNode->next = curNode->next;
						break;
					}
					prevNode = curNode;
					curNode = curNode->next;
				}
			}
			if (flag)
			{
				xdelete<Node>(curNode);
			}
		}
		std::optional<std::pair<Key, Value>> extract(const Key& key_)noexcept
		{
			Node* prevNode = &head;
			Node* curNode;
			bool flag = false;
			std::optional<std::pair<Key, Value>> target = std::nullopt;
			{
				std::lock_guard<std::mutex> erase_lock{ m_eraseLock };
				//std::shared_lock<SRWLock> s_lock{ m_sharedMutex };
				curNode = prevNode->next;
				while (curNode)
				{
					if (key_ == curNode->data.first)
					{
						flag = true;
						std::lock_guard<SRWLock> lock{ m_sharedMutexForErase };
						prevNode->next = curNode->next;
						target.emplace(std::move(curNode->data));
						break;
					}
					prevNode = curNode;
					curNode = curNode->next;
				}
			}
			if (flag)
			{
				xdelete<Node>(curNode);
			}
			return target;
		}
		void erase_unsafe(const Key& key_)noexcept
		{
			bool flag = false;
			Node* prevNode = &head;
			Node* curNode = prevNode->next;
			while (curNode)
			{
				if (key_ == curNode->data.first)
				{
					flag = true;
					std::lock_guard<SRWLock> lock{ m_sharedMutexForErase };
					prevNode->next = curNode->next;
					break;
				}
				prevNode = curNode;
				curNode = curNode->next;
			}
			if (flag)
			{
				xdelete<Node>(curNode);
			}
		}
		std::optional<std::pair<Key, Value>> extract_unsafe(const Key& key_)noexcept
		{
			bool flag = false;
			std::optional<std::pair<Key, Value>> target = std::nullopt;
			Node* prevNode = &head;
			Node* curNode = prevNode->next;
			while (curNode)
			{
				if (key_ == curNode->data.first)
				{
					flag = true;
					std::lock_guard<SRWLock> lock{ m_sharedMutexForErase };
					prevNode->next = curNode->next;
					target.emplace(std::move(curNode->data));
					break;
				}
				prevNode = curNode;
				curNode = curNode->next;
			}
			if (flag)
			{
				xdelete<Node>(curNode);
			}
			return target;
		}
		void clear()noexcept
		{
			{
				std::scoped_lock lock{ m_eraseLock,m_sharedMutexForErase };
				reset();
			}
		}
		void clear_unsafe()noexcept
		{
			reset();
		}
	};

	template <typename Key, typename Value>
	class ConcurrentHashMap
	{
	private:
		Vector<ConcurrentListForMap<const Key, Value>> buckets;
		std::hash<Key> hasher;
	public:
		ConcurrentHashMap(const std::size_t size_ = DEFAULT_MEM_POOL_SIZE) noexcept :buckets(size_) {}

		template <typename ...Args>
		void emplace_no_return(const Key& key, Args&&... args) noexcept
		{
			const size_t index = hasher(key) % buckets.size();
			if (!buckets[index].find(key))
				buckets[index].emplace_front_no_return(key, std::forward<Args>(args)...);
		}

		template <typename ...Args>
		std::optional<Value> emplace(const Key& key, Args&&... args) noexcept
		{
			const size_t index = hasher(key) % buckets.size();
			std::optional<Value> iter = buckets[index].find(key);
			if (!iter)
				iter = buckets[index].emplace_front(key, std::forward<Args>(args)...);
			return iter;
		}

		template <typename ...Args>
		void emplace_unsafe(const Key& key, Args&&... args) noexcept
		{
			const size_t index = hasher(key) % buckets.size();
			if (!buckets[index].find(key))
				buckets[index].emplace_unsafe(key, std::forward<Args>(args)...);
		}

		template <typename ...Args>
		const bool find_and_emplace(const Key& key, Args&&... args) noexcept
		{
			const size_t index = hasher(key) % buckets.size();
			return buckets[index].find_and_emplace(key, std::forward<Args>(args)...);
		}

		template <typename ...Args>
		std::pair<std::optional<Value>, bool> try_emplace(const Key& key, Args&&... args) noexcept
		{
			const size_t index = hasher(key) % buckets.size();
			std::optional<Value> iter = buckets[index].find(key);
			const bool bNotExist = !iter.has_value();
			if (bNotExist)
				iter = buckets[index].emplace_front(key, std::forward<Args>(args)...);
			return std::make_pair(std::move(iter), bNotExist);
		}

		std::optional<Value> find(const Key& key)const noexcept
		{
			const size_t index = hasher(key) % buckets.size();
			return buckets[index].find(key);
		}

		std::optional<Value> find(const Key& key) noexcept
		{
			const size_t index = hasher(key) % buckets.size();
			return buckets[index].find(key);
		}

		std::optional<std::pair<Key, Value>> extract(const Key& key) noexcept
		{
			const size_t index = hasher(key) % buckets.size();
			return buckets[index].extract(key);
		}

		void erase(const Key& key) noexcept
		{
			const size_t index = hasher(key) % buckets.size();
			buckets[index].erase(key);
		}

		std::optional<std::pair<Key, Value>> extract_unsafe(const Key& key) noexcept
		{
			const size_t index = hasher(key) % buckets.size();
			return buckets[index].extract_unsafe(key);
		}

		void erase_unsafe(const Key& key) noexcept
		{
			const size_t index = hasher(key) % buckets.size();
			buckets[index].erase_unsafe(key);
		}

		void reserve(const size_t newCapacity) noexcept
		{
			buckets.resize(newCapacity);
		}

		const bool contains(const Key& key)const noexcept
		{
			return find(key).has_value();
		}

		void clear() noexcept
		{
			for (auto& bucket : buckets)
			{
				bucket.clear();
			}
		}

		void clear_unsafe() noexcept
		{
			for (auto& bucket : buckets)
			{
				bucket.clear_unsafe();
			}
		}
	};

	template <typename Value>
	class ConcurrentList
	{
	public:
		struct Node
		{
			Value data;
			Node* next;
			constexpr Node()noexcept :next{ nullptr } {}
			template<typename... Args>
			constexpr Node(Args&&... args)noexcept :data{ std::forward<Args>(args)... } {}
			template<typename... Args>
			constexpr Node(Node* const next_, Args&&... args)noexcept :data{ std::forward<Args>(args)... }, next{ next_ } {}
		};
	private:
		Node head;
		//mutable SRWLock m_sharedMutex;
		mutable SRWLock m_sharedMutexForErase;
		mutable std::mutex m_eraseLock;
	private:
		void reset()noexcept
		{
			while (Node* const oldHead = head.next)
			{
				head.next = oldHead->next;
				xdelete<Node>(oldHead);
			}
		}
	public:
		ConcurrentList() noexcept = default;
		~ConcurrentList() noexcept
		{
			reset();
		}
		ConcurrentList(ConcurrentList&& other) noexcept
			: head(std::move(other.head))
		{
		}
		template<typename ...Args>
		void emplace_front_no_return(Args&&... args)noexcept
		{
			//Node* const newNode = ::xnew<Node>(std::forward<Args>(args)...);
			//{
			//	std::lock_guard<SRWLock> s_lock{ m_sharedMutex };
			//	newNode->next = head.next;
			//	head.next = newNode;
			//}
			Node* const newNode = xnew<Node>(std::forward<Args>(args)...);
			std::shared_lock<SRWLock> shared_erase{ m_sharedMutexForErase };
			do {
				newNode->next = head.next;
			} while (InterlockedCompareExchangePointer(
				reinterpret_cast<PVOID volatile*>(&head.next),
				newNode,
				newNode->next) != newNode->next);
		}
		template<typename ...Args>
		std::optional<Value> emplace_front(Args&&... args)noexcept
		{
			//Node* const newNode = ::xnew<Node>(std::forward<Args>(args)...);
			//{
			//	std::lock_guard<SRWLock> s_lock{ m_sharedMutex };
			//	newNode->next = head.next;
			//	head.next = newNode;
			//}
			//return newNode->data;
			Node* const newNode = xnew<Node>(std::forward<Args>(args)...);
			std::shared_lock<SRWLock> shared_erase{ m_sharedMutexForErase };
			do {
				newNode->next = head.next;
			} while (InterlockedCompareExchangePointer(
				reinterpret_cast<PVOID volatile*>(&head.next),
				newNode,
				newNode->next) != newNode->next);
			return newNode->data;
		}
		template<typename ...Args>
		void emplace_unsafe(Args&&... args)noexcept
		{
			Node* const newNode = xnew<Node>(head.next, std::forward<Args>(args)...);
			head.next = newNode;
			std::atomic_thread_fence(std::memory_order_release);
		}
		const bool find_and_emplace(Value&& val_) noexcept
		{
			const Node* curNode = &head;
			{
				std::lock_guard<SRWLock> find_lock{ m_sharedMutexForErase };
				curNode = head.next;
				while (curNode)
				{
					if (val_ == curNode->data)
					{
						break;
					}
					curNode = curNode->next;
				}
				if (!curNode)
				{
					Node* const newNode = xnew<Node>(head.next, std::move(val_));
					head.next = newNode;
				}
			}
			return !curNode;
		}
		std::optional<Value> find(const Value& val_) noexcept
		{
			const Node* curNode = &head;
			std::optional<Value> targetData = std::nullopt;
			{
				//std::shared_lock<SRWLock> s_lock{ m_sharedMutex };
				std::shared_lock<SRWLock> shared_erase{ m_sharedMutexForErase };
				curNode = head.next;
				while (curNode)
				{
					if (val_ == curNode->data)
					{
						targetData.emplace(curNode->data);
						break;
					}
					curNode = curNode->next;
				}
			}
			return targetData;
		}
		std::optional<Value> find(const Value& val_)const noexcept
		{
			const Node* curNode = &head;
			std::optional<Value> targetData = std::nullopt;
			{
				//std::shared_lock<SRWLock> s_lock{ m_sharedMutex };
				std::shared_lock<SRWLock> shared_erase{ m_sharedMutexForErase };
				curNode = head.next;
				while (curNode)
				{
					if (val_ == curNode->data)
					{
						targetData.emplace(curNode->data);
						break;
					}
					curNode = curNode->next;
				}
			}
			return targetData;
		}
		void erase(const Value& val_)noexcept
		{
			Node* prevNode = &head;
			Node* curNode;
			bool flag = false;
			{
				std::lock_guard<std::mutex> erase_lock{ m_eraseLock };
				//std::shared_lock<SRWLock> s_lock{ m_sharedMutex };
				curNode = prevNode->next;
				while (curNode)
				{
					if (val_ == curNode->data)
					{
						flag = true;
						std::lock_guard<SRWLock> lock{ m_sharedMutexForErase };
						prevNode->next = curNode->next;
						break;
					}
					prevNode = curNode;
					curNode = curNode->next;
				}
			}
			if (flag)
			{
				xdelete<Node>(curNode);
			}
		}
		std::optional<Value> extract(const Value& val_)noexcept
		{
			Node* prevNode = &head;
			Node* curNode;
			bool flag = false;
			std::optional<Value> target = std::nullopt;
			{
				std::lock_guard<std::mutex> erase_lock{ m_eraseLock };
				//std::shared_lock<SRWLock> s_lock{ m_sharedMutex };
				curNode = prevNode->next;
				while (curNode)
				{
					if (val_ == curNode->data)
					{
						flag = true;
						std::lock_guard<SRWLock> lock{ m_sharedMutexForErase };
						prevNode->next = curNode->next;
						target.emplace(std::move(curNode->data));
						break;
					}
					prevNode = curNode;
					curNode = curNode->next;
				}
			}
			if (flag)
			{
				xdelete<Node>(curNode);
			}
			return target;
		}
		void erase_unsafe(const Value& val_)noexcept
		{
			bool flag = false;
			Node* prevNode = &head;
			Node* curNode = prevNode->next;
			while (curNode)
			{
				if (val_ == curNode->data)
				{
					flag = true;
					std::lock_guard<SRWLock> lock{ m_sharedMutexForErase };
					prevNode->next = curNode->next;
					break;
				}
				prevNode = curNode;
				curNode = curNode->next;
			}
			if (flag)
			{
				xdelete<Node>(curNode);
			}
		}
		std::optional<Value> extract_unsafe(const Value& val_)noexcept
		{
			bool flag = false;
			std::optional<Value> target = std::nullopt;
			Node* prevNode = &head;
			Node* curNode = prevNode->next;
			while (curNode)
			{
				if (val_ == curNode->data)
				{
					flag = true;
					std::lock_guard<SRWLock> lock{ m_sharedMutexForErase };
					prevNode->next = curNode->next;
					target.emplace(std::move(curNode->data));
					break;
				}
				prevNode = curNode;
				curNode = curNode->next;
			}
			if (flag)
			{
				xdelete<Node>(curNode);
			}
			return target;
		}
		void clear()noexcept
		{
			{
				std::scoped_lock lock{ m_eraseLock,m_sharedMutexForErase };
				reset();
			}
		}
		void clear_unsafe()noexcept
		{
			reset();
		}
		std::vector<Value*> GetAllElements()const noexcept
		{
			std::vector<Value*> temp;
			Node* curNode = head.next;
			while (curNode)
			{
				temp.emplace_back(&curNode->data);
				curNode = curNode->next;
			}
			return temp;
		}
	};

	template <typename Key>
	class ConcurrentHashSet
	{
	private:
		Vector<ConcurrentList<Key>> buckets;
		std::hash<Key> hasher;
	public:
		ConcurrentHashSet(const std::size_t size_ = DEFAULT_MEM_POOL_SIZE) noexcept :buckets(size_) {}
		~ConcurrentHashSet()noexcept
		{
			clear();
		}

		template <typename ...Args>
		void emplace_no_return(Args&&... args) noexcept
		{
			Key key{ std::forward<Args>(args)... };
			const size_t index = hasher(key) % buckets.size();
			if (!buckets[index].find(key))
				buckets[index].emplace_front_no_return(std::move(key));
		}

		template <typename ...Args>
		std::optional<Key> emplace(Args&&... args) noexcept
		{
			Key key{ std::forward<Args>(args)... };
			const size_t index = hasher(key) % buckets.size();
			std::optional<Key> iter = buckets[index].find(key);
			if (!iter)
				iter = buckets[index].emplace_front(std::move(key));
			return iter;
		}

		template <typename ...Args>
		void emplace_unsafe(Args&&... args) noexcept
		{
			Key key{ std::forward<Args>(args)... };
			const size_t index = hasher(key) % buckets.size();
			if (!buckets[index].find(key))
				buckets[index].emplace_unsafe(std::move(key));
		}

		template <typename ...Args>
		const bool find_and_emplace(Args&&... args) noexcept
		{
			Key key{ std::forward<Args>(args)... };
			const size_t index = hasher(key) % buckets.size();
			return buckets[index].find_and_emplace(std::move(key));
		}

		template <typename ...Args>
		std::pair<std::optional<Key>, bool> try_emplace(Args&&... args) noexcept
		{
			Key key{ std::forward<Args>(args)... };
			const size_t index = hasher(key) % buckets.size();
			std::optional<Key> iter = buckets[index].find(key);
			const bool bNotExist = !iter.has_value();
			if (bNotExist)
				iter = buckets[index].emplace_front(std::move(key));
			return std::make_pair(std::move(iter), bNotExist);
		}

		std::optional<Key> find(const Key& key)const noexcept
		{
			const size_t index = hasher(key) % buckets.size();
			return buckets[index].find(key);
		}

		std::optional<Key> find(const Key& key) noexcept
		{
			const size_t index = hasher(key) % buckets.size();
			return buckets[index].find(key);
		}

		const bool contains(const Key& key)const noexcept
		{
			return find(key).has_value();
		}

		void erase(const Key& key) noexcept
		{
			const size_t index = hasher(key) % buckets.size();
			buckets[index].erase(key);
		}

		void erase_unsafe(const Key& key) noexcept
		{
			const size_t index = hasher(key) % buckets.size();
			buckets[index].erase_unsafe(key);
		}

		std::optional<Key> extract(const Key& key) noexcept
		{
			const size_t index = hasher(key) % buckets.size();
			return buckets[index].extract(key);
		}

		std::optional<Key> extract_unsafe(const Key& key) noexcept
		{
			const size_t index = hasher(key) % buckets.size();
			return buckets[index].extract_unsafe(key);
		}
		void reserve(const size_t newCapacity) noexcept
		{
			buckets.resize(newCapacity);
		}

		void clear() noexcept
		{
			for (auto& bucket : buckets)
			{
				bucket.clear();
			}
		}

		void clear_unsafe() noexcept
		{
			for (auto& bucket : buckets)
			{
				bucket.clear_unsafe();
			}
		}

		std::vector<Key*> GetAllElements()noexcept
		{
			std::vector<Key*> temp;
			temp.reserve(100);
			for (const auto& bucket : buckets)
			{
				for (const auto& ele : bucket.GetAllElements())
				{
					temp.emplace_back(ele);
				}
			}
			return temp;
		}
	};
}