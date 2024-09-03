#pragma once
#include "ServerCorePch.h"
#include <Windows.h>
#include "MemoryHeader.hpp"

namespace ServerCore
{
    template<typename T>
    class alignas(64) AtomicMemoryPool
    {
        //constinit static inline const uint8 CACHE_LINE_MINUS_ONE = static_cast<c_uint8>(std::hardware_constructive_interference_size - 1);
        constinit static inline const uint8 CACHE_LINE_MINUS_ONE = static_cast<c_uint8>(8 - 1);
        static const inline HANDLE g_handle = GetProcessHeap();
    private:
        struct alignas(8) Block
        {
            alignas(8) std::atomic<uint64_t> combined = 0;
        };

        struct BlockChaser
        {
            Block* const target = nullptr;
            BlockChaser* next = nullptr;
            BlockChaser() = default;
            BlockChaser(Block* const pBlock_, BlockChaser* const pNext_)noexcept :target{ pBlock_ }, next{ pNext_ } {}
        };
        std::atomic<uint64_t> head;
        std::atomic<BlockChaser*> poolTop = nullptr;
        const size_t blockSize;
        const size_t maxBlockCount;
        std::byte* const blockStart;
        static inline constinit const uint32_t maxTagValue = (1 << 19) - 1;
    private:
        static constexpr const uint64_t packPointerAndTag(const Block* const ptr, const uint32_t tag) noexcept {
            const uintptr_t ptrVal = reinterpret_cast<uintptr_t>(ptr);
            return (static_cast<const uint64_t>(ptrVal) & 0x7FFFFFFFFFF) | (static_cast<const uint64_t>(tag) << 45);
        }

        static constexpr const Block* const unpackPointer(const uint64_t combined) noexcept {
            return reinterpret_cast<const Block* const>(combined & 0x7FFFFFFFFFF);
        }

        static constexpr const uint32_t unpackTag(const uint64_t combined) noexcept {
            return static_cast<const uint32_t>(combined >> 45);
        }

        void initialize() noexcept
        {
            for (size_t i = 0; i < maxBlockCount - 1; ++i)
            {
                Block* const block = new (blockStart + i * blockSize) Block();
                const uint64_t nextCombined = packPointerAndTag(reinterpret_cast<Block*>(blockStart + (i + 1) * blockSize), 0);
                block->combined.store(nextCombined, std::memory_order_relaxed);
            }

            Block* const lastBlock = new (blockStart + (maxBlockCount - 1) * blockSize) Block();
            lastBlock->combined.store(packPointerAndTag(nullptr, 0), std::memory_order_relaxed);

            head.store(packPointerAndTag(reinterpret_cast<Block* const>(blockStart), 0), std::memory_order_relaxed);
        }

        constexpr __forceinline T* const allocateNewBlock()noexcept
        {
            //Block* const newBlock = std::construct_at<Block>(static_cast<Block* const>(::_aligned_malloc(blockSize, std::hardware_constructive_interference_size)));
            Block* const newBlock = std::construct_at<Block>(static_cast<Block* const>(::_aligned_malloc(blockSize, 8)));
            BlockChaser* const newTop = std::construct_at<BlockChaser>(static_cast<BlockChaser* const>(::HeapAlloc(g_handle, NULL, sizeof(BlockChaser))), newBlock, poolTop.load(std::memory_order_relaxed));
            while (!poolTop.compare_exchange_weak(newTop->next, newTop,
                std::memory_order_relaxed, std::memory_order_relaxed))
            {
            }
            std::atomic_thread_fence(std::memory_order_acquire);
            return reinterpret_cast<T* const>(newBlock + 1);
        }

        template <typename T>
        struct alignas(8) AlignedStorage { alignas(8) Block pad; alignas(8)std::byte data[sizeof(T)]; };
    public:
        AtomicMemoryPool(const size_t count = DEFAULT_MEM_POOL_SIZE)
            : blockSize{ (sizeof(MemoryHeader) + sizeof(T) + sizeof(Block) + CACHE_LINE_MINUS_ONE) & ~CACHE_LINE_MINUS_ONE }
            , maxBlockCount{ count }
            , poolTop{ nullptr }
            , blockStart{ static_cast<std::byte* const>(::_aligned_malloc(blockSize * maxBlockCount, std::hardware_constructive_interference_size)) }
        {
            initialize();
        }

        ~AtomicMemoryPool()
        {
            BlockChaser* curBlock = poolTop.load(std::memory_order_relaxed);
            while (BlockChaser* const delBlock = curBlock)
            {
                ::_aligned_free(curBlock->target);
                curBlock = curBlock->next;
                HeapFree(g_handle, NULL, delBlock);
            }
            ::_aligned_free(blockStart);
        }

        constexpr T* const allocate() noexcept
        {
            uint64_t oldCombined = head.load(std::memory_order_relaxed);
            uint64_t newCombined;
            const Block* __restrict currentBlock;
            do {
                currentBlock = unpackPointer(oldCombined);
                if (!currentBlock)
                {
                    return allocateNewBlock();
                }
                const uint32_t newTag = unpackTag(oldCombined) + 1;
                const Block* const nextBlock = unpackPointer(currentBlock->combined.load(std::memory_order_relaxed));
                newCombined = packPointerAndTag(nextBlock, newTag);
            } while (!head.compare_exchange_weak(oldCombined, newCombined,
                std::memory_order_acquire,
                std::memory_order_relaxed));
            return reinterpret_cast<T* const>(const_cast<Block* const>(currentBlock) + 1);
        }

        constexpr void deallocate(void* const object) noexcept
        {
            if (!object)
            {
                return;
            }
            Block* const blockPtr = reinterpret_cast<Block* const>(object) - 1;
            auto& __restrict block_combined = blockPtr->combined;
            uint64_t oldHead = head.load(std::memory_order_relaxed);
            uint64_t newCombined;
            do {
                const uint32_t newTag = unpackTag(oldHead) + 1;
                newCombined = packPointerAndTag(blockPtr, newTag);
                block_combined.store(oldHead, std::memory_order_relaxed);
            } while (!head.compare_exchange_weak(oldHead, newCombined,
                std::memory_order_release,
                std::memory_order_relaxed));
        }

        void checkAndResetIfNeeded()noexcept
        {
            const uint32_t currentTag = unpackTag(head.load(std::memory_order_relaxed));
            if (currentTag >= maxTagValue)
            {
                initialize();
            }
        }

        const bool isNeedReset()const noexcept {
            return maxTagValue - 10000 <= unpackTag(head.load(std::memory_order_relaxed));
        }
    };

    static constexpr const size_t DEFAULT_ATOMIC_ALLOCATOR_SIZE = 128;

    template <typename T>
    class AtomicAllocator
    {
    public:
        using value_type = T;

        AtomicAllocator()noexcept :memPool{ Init() } {}

        template<typename U> requires !std::is_same_v<T, U>
        AtomicAllocator(const AtomicAllocator<U>&)noexcept :memPool{ Init() } {}

        template<typename U> requires !std::is_same_v<T, U>
        AtomicAllocator(AtomicAllocator<U>&&)noexcept :memPool{ Init() } {}

        AtomicAllocator(const AtomicAllocator& rhs)noexcept :memPool{ Init() } {}

        AtomicAllocator(AtomicAllocator&& rhs)noexcept :memPool{ std::move(rhs.memPool) } {}

        constexpr T* const allocate(const size_t size_)const noexcept
        {
            if constexpr (std::is_same_v<T, std::_Container_proxy>)
                return static_cast<T* const>(::malloc(size_ * sizeof(T)));
            else
                return memPool->allocate();
        }
        constexpr void deallocate(void* const ptr, const size_t)const noexcept
        {
            if constexpr (std::is_same_v<T, std::_Container_proxy>)
                ::free(ptr);
            else
                memPool->deallocate(ptr);
        }
        constexpr friend const bool operator==(const AtomicAllocator<T>&, const AtomicAllocator<T>&)noexcept { return true; }

        constexpr friend const bool operator!=(const AtomicAllocator<T>& a, const AtomicAllocator<T>& b)noexcept { return !(a == b); }
    private:
        std::unique_ptr<AtomicMemoryPool<T>> memPool;


        static std::unique_ptr<AtomicMemoryPool<T>> Init() noexcept;
    };

    template<typename T>
    inline std::unique_ptr<AtomicMemoryPool<T>> AtomicAllocator<T>::Init() noexcept
    {
        if constexpr (std::is_same_v<T, std::_Container_proxy>)
        {
            return nullptr;
        }
        else
        {
            return std::make_unique<AtomicMemoryPool<T>>(DEFAULT_ATOMIC_ALLOCATOR_SIZE);
        }
    }
}


