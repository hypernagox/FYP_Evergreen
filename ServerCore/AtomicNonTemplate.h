#pragma once

namespace ServerCore
{
    class alignas(64) AtomicNonTemplate
    {
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
        static const uint64_t packPointerAndTag(const Block* const ptr, const uint32_t tag) noexcept;

        static const Block* const unpackPointer(const uint64_t combined) noexcept;

        static constexpr const uint32_t unpackTag(const uint64_t combined) noexcept;

        void initialize() noexcept;


        void* const allocateNewBlock()noexcept;


    public:
        AtomicNonTemplate(const size_t size_, const size_t count = DEFAULT_MEM_POOL_SIZE);

        ~AtomicNonTemplate();


        void* const allocate() noexcept;


        void deallocate(void* const object) noexcept;


        void checkAndResetIfNeeded()noexcept;


        const bool isNeedReset()const noexcept;

    };
}