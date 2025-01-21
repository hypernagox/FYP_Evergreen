#pragma once
#include "ServerCorePch.h"

namespace ServerCore
{
    template<typename Key, typename Value>
    class LinkedHashMap
    {
    public:
        constexpr LinkedHashMap(const std::size_t size_ = DEFAULT_ATOMIC_ALLOCATOR_SIZE)noexcept :m_mapForGetItem(size_) { m_mapForFindItem.reserve(size_); }
    public:
        template<typename V>
        Value* const AddItem(const Key& key, V&& value)noexcept
        {
            if (HasItem(key))
                return nullptr;
            const auto temp_ptr = value.get();
            m_mapForFindItem.try_emplace(key, m_listItem.insert(m_listItem.cend(), temp_ptr));
            m_mapForGetItem.emplace_unsafe(key, std::forward<V>(value));
            return static_cast<Value* const>(temp_ptr);
        }
        template<typename V> requires std::same_as<std::remove_cvref_t<V>, S_ptr<Value>>
        Value* const AddItem_endLock(const Key& key, V&& value)noexcept
        {
            if (HasItem(key))
                return nullptr;
            const auto temp_ptr = value.get();
            decltype(m_listItem.begin()) insert_iter;
            m_srwLock.lock();
            insert_iter = m_listItem.insert(m_listItem.cend(), temp_ptr);
            m_srwLock.unlock();
            m_mapForFindItem.emplace(key, insert_iter);
            m_mapForGetItem.emplace_unsafe(key, std::forward<V>(value));
            return static_cast<Value* const>(temp_ptr);
        }
        S_ptr<Value> FindItem(const Key& key)const noexcept
        {
            auto item = m_mapForGetItem.find(key);
            return item ? std::move((*item)) : nullptr;
        }
        Value** FindItemUnsafe(const Key& key)const noexcept
        {
            if (const auto iter = m_mapForFindItem.find(key); m_mapForFindItem.cend() != iter)
            {
                return &(*iter->second);
            }
            else
            {
                return nullptr;
            }
        }
        const auto FindListIter(const Key& key)const noexcept
        {
            if (const auto iter = m_mapForFindItem.find(key); m_mapForFindItem.cend() != iter)
            {
                return iter->second;
            }
            else
            {
                return m_listItem.cend();
            }
        }
        const auto FindListIter(const Key& key)noexcept
        {
            if (const auto iter = m_mapForFindItem.find(key); m_mapForFindItem.end() != iter)
            {
                return iter->second;
            }
            else
            {
                return m_listItem.end();
            }
        }
        const bool EraseItem(const Key& key)noexcept
        {
            if (const auto iter = m_mapForFindItem.extract(key))
            {
                m_mapForGetItem.erase_unsafe(key);
                m_listItem.erase(iter.mapped());
                return true;
            }
            else
            {
                return false;
            }
        }
        const bool EraseItemSafe(const Key& key)noexcept
        {
            if (const auto item = m_mapForGetItem.extract(key))
            {
                m_srwLock.lock();
                m_listItem.erase(m_mapForFindItem.extract(key).mapped());
                m_srwLock.unlock();
                return true;
            }
            else
            {
                return false;
            }
        }
        const auto EraseItemAndGetIter(const Key& key)noexcept
        {
            if (const auto iter = m_mapForFindItem.extract(key))
            {
                m_mapForGetItem.erase_unsafe(key);
                return m_listItem.erase(iter.mapped());
            }
            else
            {
                return m_listItem.end();
            }
        }
        S_ptr<Value> ExtractItem(const Key& key)noexcept
        {
            if (auto item = m_mapForGetItem.extract_unsafe(key))
            {
                m_listItem.erase(m_mapForFindItem.extract(key).mapped());
                return std::move(item->second);
            }
            else
            {
                return nullptr;
            }
        }
        S_ptr<Value> ExtractItemSafe(const Key& key)noexcept
        {
            if (auto item = m_mapForGetItem.extract(key))
            {
                m_srwLock.lock();
                m_listItem.erase(m_mapForFindItem.extract(key).mapped());
                m_srwLock.unlock();
                return std::move(item->second);
            }
            else
            {
                return nullptr;
            }
        }
        void SwapElement(const Key& a, const Key& b)noexcept
        {
            auto& x = m_mapForFindItem[a];
            auto& y = m_mapForFindItem[b];
            std::iter_swap(x, y);
            std::swap(x, y);
        }
        template <typename Func, typename... Args> requires std::invocable<Func, Value&, Args...> || std::invocable<Func, Value*, Args...>
        void IterateItem(Func&& fp, Args&&... args)const noexcept
        {
            for (const auto items : m_listItem)
            {
                std::invoke(fp, items, args...);
            }
        }
        void clear_unsafe()noexcept
        {
            m_mapForGetItem.clear_unsafe();
            m_mapForFindItem.clear();
            m_listItem.clear();
        }
        const bool HasItem(const Key& key)const noexcept { return m_mapForFindItem.contains(key); }

        constexpr const auto begin()const noexcept { return m_listItem.begin(); }
        constexpr const auto end()const noexcept { return m_listItem.end(); }
        constexpr const auto begin()noexcept { return m_listItem.begin(); }
        constexpr const auto end()noexcept { return m_listItem.end(); }

        constexpr const auto cbegin()const noexcept { return m_listItem.cbegin(); }
        constexpr const auto cend()const noexcept { return m_listItem.cend(); }
        constexpr const auto cbegin()noexcept { return m_listItem.cbegin(); }
        constexpr const auto cend()noexcept { return m_listItem.cend(); }

        constexpr const auto size()const noexcept { return m_listItem.size(); }

        const auto& GetItemListRef()const noexcept { return m_listItem; }
        auto& GetItemListRef()noexcept { return m_listItem; }

        constexpr const auto end_safe()noexcept {
            m_srwLock.lock_shared();
            const auto end_iter = m_listItem.end();
            m_srwLock.unlock_shared();
            return end_iter;
        }
        constexpr const auto end_safe()const noexcept {
            m_srwLock.lock_shared();
            const auto end_iter = m_listItem.end();
            m_srwLock.unlock_shared();
            return end_iter;
        }
        constexpr const auto cend_safe()noexcept {
            m_srwLock.lock_shared();
            const auto end_iter = m_listItem.cend();
            m_srwLock.unlock_shared();
            return end_iter;
        }
        constexpr const auto cend_safe()const noexcept {
            m_srwLock.lock_shared();
            const auto end_iter = m_listItem.cend();
            m_srwLock.unlock_shared();
            return end_iter;
        }
        constexpr inline auto& GetSRWLock()noexcept { return m_srwLock; }
    private:
        std::list<Value*, AtomicAllocator<Value*>> m_listItem;
        alignas(64) XHashMap<Key, decltype(m_listItem.begin())> m_mapForFindItem;
        ConcurrentHashMap<Key, S_ptr<Value>> m_mapForGetItem;
        mutable SRWLock m_srwLock;
    };
}