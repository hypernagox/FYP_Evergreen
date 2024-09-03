#include "ServerCorePch.h"
#include "AtomicNonTemplate.h"

namespace ServerCore
{
    //constinit static inline const uint8 CACHE_LINE_MINUS_ONE = static_cast<c_uint8>(std::hardware_constructive_interference_size - 1);
    constinit static inline const uint8 CACHE_LINE_MINUS_ONE = static_cast<c_uint8>(8 - 1);
    const uint64_t AtomicNonTemplate::packPointerAndTag(const Block* const ptr, const uint32_t tag) noexcept {
        const uintptr_t ptrVal = reinterpret_cast<uintptr_t>(ptr);
        return (static_cast<const uint64_t>(ptrVal) & 0x7FFFFFFFFFF) | (static_cast<const uint64_t>(tag) << 45);
    }


    const AtomicNonTemplate::Block* const AtomicNonTemplate::unpackPointer(const uint64_t combined) noexcept {
        return reinterpret_cast<const Block* const>(combined & 0x7FFFFFFFFFF);
    }

    constexpr const uint32_t AtomicNonTemplate::unpackTag(const uint64_t combined) noexcept {
        return static_cast<const uint32_t>(combined >> 45);
    }

    void AtomicNonTemplate::initialize() noexcept
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

    void* const AtomicNonTemplate::allocateNewBlock()noexcept
    {
        //Block* const newBlock = std::construct_at<Block>(static_cast<Block* const>(::_aligned_malloc(blockSize, std::hardware_constructive_interference_size)));
        Block* const newBlock = std::construct_at<Block>(static_cast<Block* const>(::_aligned_malloc(blockSize, 8)));
        BlockChaser* const newTop = std::construct_at<BlockChaser>(static_cast<BlockChaser* const>(::HeapAlloc(g_handle, NULL, sizeof(BlockChaser))), newBlock, poolTop.load(std::memory_order_relaxed));
        while (!poolTop.compare_exchange_weak(newTop->next, newTop,
            std::memory_order_relaxed, std::memory_order_relaxed))
        {
        }
        std::atomic_thread_fence(std::memory_order_acquire);
        return reinterpret_cast<void* const>(newBlock + 1);
    }


    AtomicNonTemplate::AtomicNonTemplate(const size_t size_, const size_t count)
        : blockSize{ (sizeof(Block) + size_ + CACHE_LINE_MINUS_ONE) & ~CACHE_LINE_MINUS_ONE }
        , maxBlockCount{ count }
        , poolTop{ nullptr }
        , blockStart{ static_cast<std::byte* const>(::_aligned_malloc(blockSize * maxBlockCount, std::hardware_constructive_interference_size)) }
    {
        initialize();
    }

    AtomicNonTemplate:: ~AtomicNonTemplate()
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

    void* const AtomicNonTemplate::allocate() noexcept
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
        return reinterpret_cast<void* const>(const_cast<Block* const>(currentBlock) + 1);
    }

    void AtomicNonTemplate::deallocate(void* const object) noexcept
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

    void AtomicNonTemplate::checkAndResetIfNeeded()noexcept
    {
        const uint32_t currentTag = unpackTag(head.load(std::memory_order_relaxed));
        if (currentTag >= maxTagValue)
        {
            initialize();
        }
    }

    const bool AtomicNonTemplate::isNeedReset()const noexcept {
        return maxTagValue - 10000 <= unpackTag(head.load(std::memory_order_relaxed));
    }
}