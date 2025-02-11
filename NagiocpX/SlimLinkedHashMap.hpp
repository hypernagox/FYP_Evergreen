#pragma once
#include "NagiocpXPch.h"

namespace NagiocpX
{
    template<typename Key, typename Value>
    class SlimLinkedHashMap
    {
    public:
        constexpr SlimLinkedHashMap(const std::size_t size_ = DEFAULT_ATOMIC_ALLOCATOR_SIZE)noexcept { m_mapForFindItem.reserve(size_); }
    public:
        template <typename V>
        const bool AddItem(const Key& key,V&& value)noexcept
        {
            if (false == m_mapForFindItem.try_emplace(key, m_listItem.insert(m_listItem.cend(), std::forward<V>(value))).second)
                return false;
            return true;
        }
        auto FindItemUnsafe(const Key& key)const noexcept
        {
            if (const auto iter = m_mapForFindItem.find(key); m_mapForFindItem.cend() != iter)
            {
                return &(*iter->second);
            }
            else
            {
                return {};
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
            if (const auto item = m_mapForFindItem.extract(key))
            {
                m_srwLock.lock();
                m_listItem.erase(item.mapped());
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
                return m_listItem.erase(iter.mapped());
            }
            else
            {
                return m_listItem.end();
            }
        }
        Value ExtractItem(const Key& key)noexcept
        {
            if (const auto item = m_mapForFindItem.extract(key))
            {
                const auto& temp = item.mapped();
                auto temp_ptr = *temp;
                m_listItem.erase(temp);
                return temp_ptr;
            }
            else
            {
                return {};
            }
        }
       Value ExtractItemSafe(const Key& key)noexcept
        {
            if (const auto item = m_mapForFindItem.extract(key))
            {
                const auto& temp = item.mapped();
                auto temp_ptr = *temp;
                m_srwLock.lock();
                m_listItem.erase(temp);
                m_srwLock.unlock();
                return temp_ptr;
            }
            else
            {
                return {};
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

        constexpr inline auto& GetSRWLock()noexcept { return m_srwLock; }
        void lock()const noexcept { m_srwLock.lock(); }
        void unlock()const noexcept { m_srwLock.unlock(); }
        void lock_shared()const noexcept { m_srwLock.lock_shared(); }
        void unlock_shared()const noexcept { m_srwLock.unlock_shared(); }
    private:
        std::list<Value, AtomicAllocator<Value>> m_listItem;
        alignas(64) XHashMap<Key, decltype(m_listItem.begin())> m_mapForFindItem;
        mutable SRWLock m_srwLock;
    };
}