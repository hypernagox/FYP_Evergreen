#pragma once
#include "AtomicMemoryPool.hpp"
#include "MemoryHeader.hpp"
#include "func.h"

namespace NagiocpX
{
	//template<typename T>
	//class ObjectPoolAllocator
	//{
	//public:
	//	using value_type = T;
	//
	//	constexpr ObjectPoolAllocator()noexcept {}
	//
	//	template<typename Other>
	//	ObjectPoolAllocator(const ObjectPoolAllocator<Other>&) { }
	//
	//	constexpr static T* const allocate(const size_t size)noexcept
	//	{
	//		const thread_local int32 thID = GetCurThreadIdx() % NagiocpX::NUM_OF_THREADS;
	//		return reinterpret_cast<T* const>(new (reinterpret_cast<void* const>(allocator_once[thID].allocate()))MemoryHeader{ thID } + 1);
	//	}
	//
	//	constexpr static void deallocate(T* const ptr, const size_t count)noexcept
	//	{
	//		MemoryHeader* const __restrict header = reinterpret_cast<MemoryHeader* const>(ptr) - 1;
	//		allocator_once[header->allocThread].deallocate(reinterpret_cast<T* const>(header));
	//	}
	//
	//private:
	//	static inline AtomicMemoryPool<T> allocator_once[NagiocpX::NUM_OF_THREADS];
	//};
	//
	//template<typename T, typename... Args>
	//constexpr T* const PoolNew(Args&&... args)noexcept
	//{
	//	return std::construct_at<T>(static_cast<T* const>(ObjectPoolAllocator<T>::allocate(sizeof(T))), std::forward<Args>(args)...);
	//}
	//
	//template <typename T>
	//constexpr void PoolDelete(T* const poolObjPtr)noexcept
	//{
	//	std::destroy_at<T>(poolObjPtr);
	//	ObjectPoolAllocator<T>::deallocate(poolObjPtr, 1);
	//}
	//
	//template <typename T>
	//struct UPDeleter {
	//	constexpr inline void operator()(T* const ptr) const noexcept { PoolDelete<T>(ptr); }
	//};
	//
	//template<typename T>
	//using U_Pptr = std::unique_ptr<T, UPDeleter<T>>;
	//
	//template<typename T, typename... Args>
	//constexpr U_Pptr<T> MakePoolUnique(Args&&... args)noexcept
	//{
	//	return std::unique_ptr<T, UPDeleter<T>>{PoolNew<T>(std::forward<Args>(args)...), UPDeleter<T>{}};
	//}
}
