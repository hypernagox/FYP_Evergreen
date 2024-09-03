#include "ServerCorePch.h"
#include "MemoryMgr.h"

/*-------------
	Memory
---------------*/

//namespace ServerCore
//{
//	MemoryMgr::MemoryMgr()
//	{
//		for (int i = 0; i < ServerCore::NUM_OF_THREADS; ++i)
//		{
//			auto& pools = m_pools[i];
//
//			pools.reserve(POOL_COUNT);
//
//			int32 size = 0;
//			int32 tableIndex = 0;
//
//			for (size = 32; size <= 1024; size += 32)
//			{
//				AtomicNonTemplate* const pool = new (m_poolAllocator.allocate())AtomicNonTemplate(size);
//				pools.emplace_back(pool);
//
//				while (tableIndex <= size)
//				{
//					m_poolTable[i][tableIndex] = pool;
//					tableIndex++;
//				}
//			}
//			size -= 32;
//			for (; size <= 2048; size += 128)
//			{
//				AtomicNonTemplate* const pool = new (m_poolAllocator.allocate())AtomicNonTemplate(size);
//				pools.emplace_back(pool);
//
//				while (tableIndex <= size)
//				{
//					m_poolTable[i][tableIndex] = pool;
//					tableIndex++;
//				}
//			}
//			size -= 128;
//			for (; size <= 4096 + 256; size += 256)
//			{
//				AtomicNonTemplate* const pool = new (m_poolAllocator.allocate())AtomicNonTemplate(size);
//				pools.emplace_back(pool);
//
//				while (tableIndex <= size)
//				{
//					m_poolTable[i][tableIndex] = pool;
//					tableIndex++;
//				}
//			}
//		}
//	}
//
//	MemoryMgr::~MemoryMgr()
//	{
//		for (const auto& pools : m_pools)
//		{
//			for(auto& pool:pools)
//			{
//				pool->~AtomicNonTemplate();
//				m_poolAllocator.deallocate(pool);
//			}
//		}
//	}
//
//	void* const MemoryMgr::Allocate(const size_t size)const noexcept
//	{
//		//const size_t allocSize = size + sizeof(MemoryHeader);
//		constexpr const int32 header_size = static_cast<c_int32>(sizeof(MemoryHeader));
//		const int32 allocSize = static_cast<c_int32>(size) + header_size;
//		const thread_local int32 thID = GetCurThreadIdx() % ServerCore::NUM_OF_THREADS;
//
//		
//		//static std::atomic_int g_th_id = 0;
//		//const thread_local int32 thID = g_th_id.fetch_add(1);
//
//#ifdef _STOMP
//		return MemoryHeader::AttachHeader(static_cast<MemoryHeader* const>(StompAllocator::Alloc(allocSize)), allocSize);
//#else
//		if (allocSize > MAX_ALLOC_SIZE)
//		{
//			// 메모리 풀링 최대 크기를 벗어나면 일반 할당
//			return MemoryHeader::AttachHeader(static_cast<MemoryHeader* const>(::_aligned_malloc(allocSize, std::hardware_constructive_interference_size)), allocSize, thID);
//		}
//		else
//		{
//			// 메모리 풀에서 꺼내온다
//			return MemoryHeader::AttachHeader(static_cast<MemoryHeader* const>(m_poolTable[thID][allocSize]->allocate()), allocSize, thID);
//		}
//#endif	
//	}
//
//	void MemoryMgr::Release(void* const ptr)const noexcept
//	{
//		MemoryHeader* const __restrict header = MemoryHeader::DetachHeader(ptr);
//		//const size_t allocSize = header->allocSize;
//
//		const int32 allocSize = header->allocSize;
//		
//		NAGOX_ASSERT(allocSize > 0);
//
//		header->allocSize = 0;
//#ifdef _STOMP
//		StompAllocator::Release(header);
//#else
//		if (allocSize > MAX_ALLOC_SIZE)
//		{
//			// 메모리 풀링 최대 크기를 벗어나면 일반 해제
//			::_aligned_free(header);
//		}
//		else
//		{
//			// 메모리 풀에 반납한다
//			m_poolTable[header->allocThread][allocSize]->deallocate(header);
//		}
//#endif	
//	}
//}