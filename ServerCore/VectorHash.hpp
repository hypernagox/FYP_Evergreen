#pragma once
#include "ServerCorePch.h"

namespace ServerCore
{
    template<typename Key, typename Value>
    class VectorHashMap
    {
    public:
        constexpr VectorHashMap(const std::size_t size_ = DEFAULT_ATOMIC_ALLOCATOR_SIZE)noexcept
        {
            m_ID2idx.reserve(size_);
            m_idx2ID.reserve(size_);
            m_listItem.reserve(size_);
        }
    public:
        template <typename V>
        const bool AddItem(const Key& key,V&& value)noexcept
        {
            const int32_t cur_idx = static_cast<const int32_t>(m_listItem.size());
            if (false == m_ID2idx.try_emplace(key, cur_idx).second)
                return false;
            m_idx2ID.try_emplace(cur_idx, key);
            m_srwLock.lock();
            m_listItem.emplace_back(std::forward<V>(value));
            m_srwLock.unlock();
            return true;
        }
   
        const auto EraseItemAndGetIter(const Key& key)noexcept
        {
            if (const auto iter = m_ID2idx.extract(key))
            {
                const int32_t last_idx = GetLastIndex();
                const int32_t target_idx = iter.mapped();
                m_idx2ID.erase(target_idx);
                if (const auto iter2 = m_idx2ID.extract(last_idx))
                {
                    const auto& k = iter2.mapped();
                    m_ID2idx[k] = target_idx;
                    m_idx2ID.try_emplace(target_idx, k);
                    std::swap(m_listItem[last_idx], m_listItem[target_idx]);
                    m_listItem.pop_back();
                    return m_listItem.begin() + target_idx;
                }
                else
                {
                    m_listItem.pop_back();
                    return m_listItem.end();
                }
            }
            else
            {
                return m_listItem.end();
            }
        }
   
        void EraseItemSafe(const Key& key)noexcept
        {
            if (const auto iter = m_ID2idx.extract(key))
            {
                const int32_t last_idx = GetLastIndex();
                const int32_t target_idx = iter.mapped();
                m_idx2ID.erase(target_idx);
                if (const auto iter2 = m_idx2ID.extract(last_idx))
                {
                    const auto& k = iter2.mapped();
                    m_ID2idx[k] = target_idx;
                    m_idx2ID.try_emplace(target_idx, k);
                    m_srwLock.lock();
                    std::swap(m_listItem[last_idx], m_listItem[target_idx]);
                    m_listItem.pop_back();
                    m_srwLock.unlock();
                }
                else
                {
                    m_srwLock.lock();
                    m_listItem.pop_back();
                    m_srwLock.unlock();
                }
            }
        }
   
        Value ExtractItemSafe(const Key& key)noexcept
        {
            if (const auto iter = m_ID2idx.extract(key))
            {
                const int32_t last_idx = GetLastIndex();
                const int32_t target_idx = iter.mapped();
                m_idx2ID.erase(target_idx);
                const auto item_cache = m_listItem.data();
                auto temp = item_cache[target_idx];
                if (const auto iter2 = m_idx2ID.extract(last_idx))
                {
                    const auto& k = iter2.mapped();
                    m_ID2idx[k] = target_idx;
                    m_idx2ID.try_emplace(target_idx, k);
                    m_srwLock.lock();
                    std::swap(item_cache[last_idx], item_cache[target_idx]);
                    m_listItem.pop_back();
                    m_srwLock.unlock();
                }
                else
                {
                    m_srwLock.lock();
                    m_listItem.pop_back();
                    m_srwLock.unlock();
                }
                return temp;
            }
            else
            {
                return {};
            }
        }
       
        void SwapElement(const Key& a, const Key& b)noexcept
        {
            auto& x = m_ID2idx[a];
            auto& y = m_ID2idx[b];
            std::swap(x, y);
            std::swap(m_idx2ID[x], m_idx2ID[y]);
            std::swap(m_listItem[x], m_listItem[y]);
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
            m_ID2idx.clear();
            m_idx2ID.clear();
            m_listItem.clear();
        }
   
        const bool HasItem(const Key& key)const noexcept { return m_ID2idx.contains(key); }
   
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
       inline const int32_t GetLastIndex()const noexcept { return static_cast<const int32_t>(m_listItem.size() - 1); }
    private:
        Vector<Value> m_listItem;
        alignas(64) HashMap<Key, int32_t> m_ID2idx;
        HashMap<int32_t, Key> m_idx2ID;
        mutable SRWLock m_srwLock;
    };

    template<typename Key, typename Value>
    class VectorHashMap4ID
    {
    public:
        constexpr  VectorHashMap4ID(const std::size_t size_ = DEFAULT_ATOMIC_ALLOCATOR_SIZE)noexcept
        {
            m_ID2idx.reserve(size_);
            m_listItem.reserve(size_);
        }
    public:
        template <typename V>
        const bool AddItem(const Key& key, V&& value)noexcept
        {
            if (false == m_ID2idx.try_emplace(key, static_cast<const int32_t>(m_listItem.size())).second)
                return false;
            m_srwLock.lock();
            m_listItem.emplace_back(std::forward<V>(value));
            m_srwLock.unlock();
            return true;
        }

        const auto EraseItemAndGetIter(const Key& key)noexcept
        {
            if (const auto iter = m_ID2idx.extract(key))
            {
                const int32_t last_idx = GetLastIndex();
                const int32_t target_idx = iter.mapped();
                const auto item_cache = m_listItem.data();
                if (last_idx != target_idx)
                {
                    m_ID2idx[item_cache[last_idx]->GetObjectID()] = target_idx;
                    std::swap(item_cache[last_idx], item_cache[target_idx]);
                    m_listItem.pop_back();
                    return m_listItem.begin() + target_idx;
                }
                else
                {
                    m_listItem.pop_back();
                    return m_listItem.end();
                }
            }
            else
            {
                return m_listItem.end();
            }
        }

        void EraseItemSafe(const Key& key)noexcept
        {
            if (const auto iter = m_ID2idx.extract(key))
            {
                const int32_t last_idx = GetLastIndex();
                const int32_t target_idx = iter.mapped();
                const auto item_cache = m_listItem.data();
                if (last_idx != target_idx)
                {
                    m_ID2idx[item_cache[last_idx]->GetObjectID()] = target_idx;
                    m_srwLock.lock();
                    std::swap(item_cache[last_idx], item_cache[target_idx]);
                    m_listItem.pop_back();
                    m_srwLock.unlock();
                }
                else
                {
                    m_srwLock.lock();
                    m_listItem.pop_back();
                    m_srwLock.unlock();
                }
            }
        }

        Value FindItemUnsafe(const Key& key)const noexcept
        {
            const auto iter = m_ID2idx.find(key);
            if (m_ID2idx.cend() != iter)
            {
                return m_listItem[iter->second];
            }
            else
            {
                return {};
            }
        }

        Value ExtractItemSafe(const Key& key)noexcept
        {
            if (const auto iter = m_ID2idx.extract(key))
            {
                const int32_t last_idx = GetLastIndex();
                const int32_t target_idx = iter.mapped();
                const auto item_cache = m_listItem.data();
                auto temp = item_cache[target_idx];
                if (last_idx != target_idx)
                {
                    m_ID2idx[item_cache[last_idx]->GetObjectID()] = target_idx;
                    m_srwLock.lock();
                    std::swap(item_cache[last_idx], item_cache[target_idx]);
                    m_listItem.pop_back();
                    m_srwLock.unlock();
                }
                else
                {
                    m_srwLock.lock();
                    m_listItem.pop_back();
                    m_srwLock.unlock();
                }
                return temp;
            }
            else
            {
                return {};
            }
        }

        void SwapElement(const Key& a, const Key& b)noexcept
        {
            auto& x = m_ID2idx[a];
            auto& y = m_ID2idx[b];
            std::swap(x, y);
            std::swap(m_listItem[x], m_listItem[y]);
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
            m_ID2idx.clear();
            m_listItem.clear();
        }

        const bool HasItem(const Key& key)const noexcept { return m_ID2idx.contains(key); }

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
        constexpr inline const auto& GetSRWLock()const noexcept { return m_srwLock; }
        inline void lock()const noexcept { m_srwLock.lock(); }
        inline void unlock()const noexcept { m_srwLock.unlock(); }
        inline void lock_shared()const noexcept { m_srwLock.lock_shared(); }
        inline void unlock_shared()const noexcept { m_srwLock.unlock_shared(); }
    private:
        inline const int32_t GetLastIndex()const noexcept { return static_cast<const int32_t>(m_listItem.size() - 1); }
    private:
        Vector<Value> m_listItem;
        alignas(64) HashMap<Key, int32_t> m_ID2idx;
        mutable SRWLock m_srwLock;
    };

    template<typename Value>
    class VectorHashSetUnsafe
    {
    public:
        constexpr VectorHashSetUnsafe(const std::size_t size_ = DEFAULT_ATOMIC_ALLOCATOR_SIZE)noexcept
        {
            m_ID2idx.reserve(size_);
            m_idx2ID.reserve(size_);
            m_listItem.reserve(size_);
        }
    public:
        template <typename V>
        const bool AddItem(V&& value)noexcept
        {
            const auto key = GetHashVal(value);
            const int32_t cur_idx = static_cast<const int32_t>(m_listItem.size());
            if (false == m_ID2idx.try_emplace(key, cur_idx).second)
                return false;
            m_idx2ID.try_emplace(cur_idx, key);
            m_listItem.emplace_back(std::forward<V>(value));
            return true;
        }

        
        const auto EraseItemAndGetIter(const Value& value)noexcept
        {
            const auto key = GetHashVal(value);
            if (const auto iter = m_ID2idx.extract(key))
            {
                const int32_t last_idx = GetLastIndex();
                const int32_t target_idx = iter.mapped();
                m_idx2ID.erase(target_idx);
                if (const auto iter2 = m_idx2ID.extract(last_idx))
                {
                    const auto& k = iter2.mapped();
                    m_ID2idx[k] = target_idx;
                    m_idx2ID.try_emplace(target_idx, k);
                    std::swap(m_listItem[last_idx], m_listItem[target_idx]);
                    m_listItem.pop_back();
                    return m_listItem.begin() + target_idx;
                }
                else
                {
                    m_listItem.pop_back();
                    return m_listItem.end();
                }
            }
            else
            {
                return m_listItem.end();
            }
        }

        void EraseItem(const Value& value)noexcept
        {
            const auto key = GetHashVal(value);
            if (const auto iter = m_ID2idx.extract(key))
            {
                const int32_t last_idx = GetLastIndex();
                const int32_t target_idx = iter.mapped();
                m_idx2ID.erase(target_idx);
                if (const auto iter2 = m_idx2ID.extract(last_idx))
                {
                    const auto& k = iter2.mapped();
                    m_ID2idx[k] = target_idx;
                    m_idx2ID.try_emplace(target_idx, k);
                    std::swap(m_listItem[last_idx], m_listItem[target_idx]);
                    m_listItem.pop_back();
                }
                else
                {
                    m_listItem.pop_back();
                }
            }
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
            m_ID2idx.clear();
            m_idx2ID.clear();
            m_listItem.clear();
        }

        const bool HasItem(const Value& val)const noexcept { return m_ID2idx.contains(GetHashVal(val)); }

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

    private:
        inline const int32_t GetLastIndex()const noexcept { return static_cast<const int32_t>(m_listItem.size() - 1); }
        inline const std::size_t GetHashVal(const Value& val)const noexcept { return std::hash<Value>{}(val); }
    private:
        Vector<Value> m_listItem;
        alignas(64) HashMap<std::size_t, int32_t> m_ID2idx;
        HashMap<int32_t, std::size_t> m_idx2ID;
    };

    template<typename Key, typename Value>
    class VectorHashMap4IDUnsafe
    {
    public:
        constexpr  VectorHashMap4IDUnsafe(const std::size_t size_ = DEFAULT_ATOMIC_ALLOCATOR_SIZE)noexcept
        {
            m_listItem.reserve(size_);
        }
    public:
        template <typename V>
        const bool AddItem(const Key& key, V&& value)noexcept
        {
            if (false == m_ID2idx.try_emplace(key, static_cast<const int32_t>(m_listItem.size())).second)
                return false;
            m_listItem.emplace_back(std::forward<V>(value));
            return true;
        }

        const auto EraseItemAndGetIter(const Key& key)noexcept
        {
            if (const auto iter = m_ID2idx.extract(key))
            {
                const int32_t last_idx = GetLastIndex();
                const int32_t target_idx = iter.mapped();
                const auto item_cache = m_listItem.data();
                if (last_idx != target_idx)
                {
                    m_ID2idx[item_cache[last_idx]->GetObjectID()] = target_idx;
                    std::swap(item_cache[last_idx], item_cache[target_idx]);
                    m_listItem.pop_back();
                    return m_listItem.begin() + target_idx;
                }
                else
                {
                    m_listItem.pop_back();
                    return m_listItem.end();
                }
            }
            else
            {
                return m_listItem.end();
            }
        }

        void EraseItem(const Key& key)noexcept
        {
            if (const auto iter = m_ID2idx.extract(key))
            {
                const int32_t last_idx = GetLastIndex();
                const int32_t target_idx = iter.mapped();
                const auto item_cache = m_listItem.data();
                if (last_idx != target_idx)
                {
                    m_ID2idx[item_cache[last_idx]->GetObjectID()] = target_idx;
                    std::swap(item_cache[last_idx], item_cache[target_idx]);
                    m_listItem.pop_back();
                }
                else
                {
                    m_listItem.pop_back();
                }
            }
        }

        const bool TryEraseItem(const Key& key)noexcept
        {
            if (const auto iter = m_ID2idx.extract(key))
            {
                const int32_t last_idx = GetLastIndex();
                const int32_t target_idx = iter.mapped();
                const auto item_cache = m_listItem.data();
                if (last_idx != target_idx)
                {
                    m_ID2idx[item_cache[last_idx]->GetObjectID()] = target_idx;
                    std::swap(item_cache[last_idx], item_cache[target_idx]);
                    m_listItem.pop_back();
                }
                else
                {
                    m_listItem.pop_back();
                }
                return true;
            }
            return false;
        }

        Value FindItem(const Key& key)const noexcept
        {
            const auto iter = m_ID2idx.find(key);
            if (m_ID2idx.cend() != iter)
            {
                return m_listItem[iter->second];
            }
            else
            {
                return {};
            }
        }

        Value ExtractItem(const Key& key)noexcept
        {
            if (const auto iter = m_ID2idx.extract(key))
            {
                const int32_t last_idx = GetLastIndex();
                const int32_t target_idx = iter.mapped();
                const auto item_cache = m_listItem.data();
                auto temp = item_cache[target_idx];
                if (last_idx != target_idx)
                {
                    m_ID2idx[item_cache[last_idx]->GetObjectID()] = target_idx;
                    std::swap(item_cache[last_idx], item_cache[target_idx]);
                    m_listItem.pop_back();
                }
                else
                {
                    m_listItem.pop_back();
                }
                return temp;
            }
            else
            {
                return {};
            }
        }

        void SwapElement(const Key& a, const Key& b)noexcept
        {
            auto& x = m_ID2idx[a];
            auto& y = m_ID2idx[b];
            std::swap(x, y);
            std::swap(m_listItem[x], m_listItem[y]);
        }

        template <typename Func, typename... Args> requires std::invocable<Func, Value&, Args...> || std::invocable<Func, Value*, Args...>
        void IterateItem(Func&& fp, Args&&... args)const noexcept
        {
            for (const auto items : m_listItem)
            {
                std::invoke(fp, items, args...);
            }
        }

        void clear()noexcept
        {
            m_ID2idx.clear();
            m_listItem.clear();
        }

        const bool HasItem(const Key& key)const noexcept { return m_ID2idx.contains(key); }

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
    private:
        inline const int32_t GetLastIndex()const noexcept { return static_cast<const int32_t>(m_listItem.size() - 1); }
    private:
        Vector<Value> m_listItem;
        Map<Key, int32_t> m_ID2idx;
    };


    template<typename Value>
    class VectorSetUnsafe
    {
    public:
        constexpr VectorSetUnsafe(const std::size_t size_ = DEFAULT_ATOMIC_ALLOCATOR_SIZE)noexcept
        {
            m_listItem.reserve(size_);
        }
    public:
        template <typename V>
        const bool AddItem(V&& value)noexcept
        {
            const int32_t cur_idx = static_cast<const int32_t>(m_listItem.size());
            if (false == m_ID2idx.try_emplace(value, cur_idx).second)
                return false;
            m_listItem.emplace_back(std::forward<V>(value));
            return true;
        }

        const auto EraseItemAndGetIter(const Value& value)noexcept
        {
            if (const auto iter = m_ID2idx.extract(value))
            {
                const int32_t last_idx = GetLastIndex();
                const int32_t target_idx = iter.mapped();
                const auto item_cache = m_listItem.data();
                if (last_idx != target_idx)
                {
                    m_ID2idx[item_cache[last_idx]] = target_idx;
                    std::swap(item_cache[last_idx], item_cache[target_idx]);
                    m_listItem.pop_back();
                    return m_listItem.begin() + target_idx;
                }
                else
                {
                    m_listItem.pop_back();
                    return m_listItem.end();
                }
            }
            else
            {
                return m_listItem.end();
            }
        }

        void EraseItem(const Value& value)noexcept
        {
            if (const auto iter = m_ID2idx.extract(value))
            {
                const int32_t last_idx = GetLastIndex();
                const int32_t target_idx = iter.mapped();
                const auto item_cache = m_listItem.data();
                if (last_idx != target_idx)
                {
                    m_ID2idx[item_cache[last_idx]] = target_idx;
                    std::swap(item_cache[last_idx], item_cache[target_idx]);
                    m_listItem.pop_back();
                }
                else
                {
                    m_listItem.pop_back();
                }
            }
        }

        const bool TryEraseItem(const Value& value)noexcept
        {
            if (const auto iter = m_ID2idx.extract(value))
            {
                const int32_t last_idx = GetLastIndex();
                const int32_t target_idx = iter.mapped();
                const auto item_cache = m_listItem.data();
                if (last_idx != target_idx)
                {
                    m_ID2idx[item_cache[last_idx]] = target_idx;
                    std::swap(item_cache[last_idx], item_cache[target_idx]);
                    m_listItem.pop_back();
                }
                else
                {
                    m_listItem.pop_back();
                }
                return true;
            }
            return false;
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
            m_ID2idx.clear();
            m_listItem.clear();
        }

        const bool HasItem(const Value& val)const noexcept { return m_ID2idx.contains(val); }

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

    private:
        inline const int32_t GetLastIndex()const noexcept { return static_cast<const int32_t>(m_listItem.size() - 1); }
    private:
        Vector<Value> m_listItem;
        Map<Value, int32_t> m_ID2idx;
    };
}