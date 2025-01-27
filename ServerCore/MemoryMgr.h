#pragma once
#include "ServerCorePch.h"
#include "AtomicMemoryPool.hpp"
#include "AtomicNonTemplate.h"
#include "Container.h"

class AtomicNonTemplate;

/*-------------
	Memory
---------------*/

namespace ServerCore
{
	//class MemoryMgr
	//	:public Singleton<MemoryMgr>
	//{
	//	friend class Singleton;
	//	enum
	//	{
	//		// ~1024까지 32단위, ~2048까지 128단위, ~4096까지 256단위
	//		POOL_COUNT = (1024 / 32) + 1 + ((2048 - 1024) / 128) + 1 + ((4096 - 2048) / 256) + 1,
	//		MAX_ALLOC_SIZE = 4096 + 256
	//	};
	//	MemoryMgr();
	//	~MemoryMgr();
	//public:
	//	void* const	Allocate(const size_t size)const noexcept;
	//	void Release(void* const ptr)const noexcept;
	//private:
	//	std::vector<AtomicNonTemplate*> m_pools[ServerCore::NUM_OF_THREADS];
	//	AtomicNonTemplate* m_poolTable[ServerCore::NUM_OF_THREADS][MAX_ALLOC_SIZE + 1];
	//	AtomicMemoryPool<AtomicNonTemplate> m_poolAllocator{ POOL_COUNT * ServerCore::NUM_OF_THREADS };
	//};


//#define USE_JE_MALLOC

	class Memory
	{
	public:
		template<typename T>
		static inline T* const Alloc(const size_t size) noexcept {
#ifdef USE_JE_MALLOC
			return static_cast<T* const>(::je_malloc(size));
#else
			return static_cast<T* const>(::malloc(size));
#endif
		}

		static inline void* const Alloc(const size_t size) noexcept {
#ifdef USE_JE_MALLOC
			return ::je_malloc(size);
#else
			return ::malloc(size);
#endif
		}

		static inline void Free(void* const ptr) noexcept {
#ifdef USE_JE_MALLOC
			return ::je_free(ptr);
#else
			return ::free(ptr);
#endif
		}

		static inline void Free_Sized(void* const ptr, const uint32_t obj_size) noexcept {
#ifdef USE_JE_MALLOC
			return ::je_free(ptr);
#else
			return ::free(ptr);
#endif
		}

		template<typename T>
		static inline T* const AlignedAlloc(const size_t size, const size_t align_val) noexcept {
#ifdef USE_JE_MALLOC
			return static_cast<T* const>(::je_aligned_alloc(align_val, size));
#else
			return static_cast<T* const>(_aligned_malloc(size, align_val));
#endif
		}

		static inline void* const AlignedAlloc(const size_t size, const size_t align_val) noexcept {
#ifdef USE_JE_MALLOC
			return ::je_aligned_alloc(align_val, size);
#else
			return _aligned_malloc(size, align_val);
#endif
		}

		static inline void AlignedFree(void* const ptr, const size_t align_val) noexcept {
#ifdef USE_JE_MALLOC
			return ::je_free(ptr);
#else
			return _aligned_free(ptr);
#endif
		}

		static inline void AlignedFree_Sized(void* const ptr, const size_t size, const size_t align_val) noexcept {
#ifdef USE_JE_MALLOC
			return ::je_free(ptr);
#else
			return _aligned_free(ptr);
#endif
		}
	};

	template<typename T, typename... Args> requires (alignof(T) <= 8)
	constexpr inline T* const xnew(Args&&... args)noexcept{
		static_assert(alignof(T) <= 8);
		return new (Memory::Alloc(sizeof(T))) T(std::forward<Args>(args)...);
	}

	template<typename T, typename... Args> requires (alignof(T) > 8)
	constexpr inline T* const aligned_xnew(Args&&... args)noexcept {
		static_assert(alignof(T) > 8);
		return new (Memory::AlignedAlloc(sizeof(T), alignof(T))) T(std::forward<Args>(args)...);
	}

	template<typename T>
	constexpr inline void xdelete(T* const obj_ptr)noexcept{
		static_assert(std::same_as<std::decay_t<T>, SendBufferChunk> || alignof(T) <= 8);
		static_assert(alignof(SendBufferChunk) > 8);
		if constexpr (std::same_as<std::decay_t<T>, ContentsEntity>)
			return obj_ptr->ProcessCleanUp();
		else if constexpr (std::derived_from<std::decay_t<T>, Session>)
			return ReturnSession(obj_ptr);
		else if constexpr (std::same_as<std::decay_t<T>, SendBufferChunk>)
			return SendBufferMgr::ReturnChunk(obj_ptr);
		else if constexpr (!std::is_trivially_destructible_v<T>)
			obj_ptr->~T(); 
		Memory::Free(obj_ptr);
	}

	template<typename T> requires (alignof(T) > 8)
	constexpr inline void aligned_xdelete(T* const obj_ptr, const size_t align_val)noexcept {
		static_assert(alignof(T) > 8);
		if constexpr (!std::is_trivially_destructible_v<T>)
			obj_ptr->~T();
		Memory::AlignedFree(obj_ptr, align_val);
	}

	template<typename T> requires (alignof(T) <= 8)
	constexpr inline void xdelete_sized(T* const obj_ptr, const uint32_t obj_size)noexcept {
		static_assert(alignof(T) <= 8);
		if constexpr (std::derived_from<std::decay_t<T>, Session>)
			return ReturnSession(obj_ptr);
		else if constexpr (!std::is_trivially_destructible_v<T>)
			obj_ptr->~T(); 
		Memory::Free_Sized(obj_ptr, obj_size);
	}

	template<typename T> requires (alignof(T) > 8)
	constexpr inline void aligned_xdelete_sized(T* const obj_ptr, const uint32_t obj_size, const size_t align_val)noexcept {
		static_assert(alignof(T) > 8);
		if constexpr (!std::is_trivially_destructible_v<T>)
			obj_ptr->~T(); 
		Memory::AlignedFree_Sized(obj_ptr, obj_size, align_val);
	}

	template<typename T, typename... Args> requires (false == std::derived_from<std::decay_t<T>,RefCountable>)
	constexpr inline std::shared_ptr<T> MakeSharedSTD(Args&&... args)noexcept{
		return std::allocate_shared<T>(StlAllocator<T>{}, std::forward<Args>(args)...);
	}

	template <typename T, typename... Args> requires (false == std::derived_from<std::decay_t<T>, RefCountable>)
	constexpr inline U_ptr<T> MakeUnique(Args&&... args)noexcept
	{
		return std::unique_ptr<T, UDeleter<T>>{xnew<T>(std::forward<Args>(args)...), UDeleter<T>{}};
	}

	template <typename T, typename... Args> requires (false == std::derived_from<std::decay_t<T>, RefCountable>)
	constexpr inline Us_ptr<T> MakeUniqueSized(Args&&... args)noexcept
	{
		return std::unique_ptr<T, USDeleter<T>>{xnew<T>(std::forward<Args>(args)...), USDeleter<T>{}};
	}

	template <typename T>
	inline auto& GetTLXVectorForCopy()noexcept { thread_local XVector<T> th_vec(1024); th_vec.clear(); return th_vec; }

	template <typename T>
	inline auto& GetTLSetForUnique()noexcept { thread_local XHashSet<T> th_set(1024); th_set.clear(); return th_set; }

	template<typename T, typename... Args>
	static constexpr inline const std::span<T> CreateJEMallocArray(const size_t num_of_elements, Args&&... args)noexcept
	{
		const auto arr_ptr = static_cast<T* const>(Memory::AlignedAlloc(sizeof(T) * num_of_elements, alignof(T)));
		const auto e = arr_ptr + num_of_elements;
		for (auto b = arr_ptr; e != b;)std::construct_at<T>(b++, args...);
		return { arr_ptr,e };
	}

	template<typename T>
	static constexpr inline void DeleteJEMallocArray(const std::span<T> target_arr)noexcept
	{
		const auto arr_ptr = target_arr.data();
		const size_t num_of_elements = target_arr.size();
		const auto e = arr_ptr + num_of_elements;
		for (auto b = arr_ptr; e != b;)std::destroy_at<T>(b++);
		Memory::AlignedFree_Sized(arr_ptr, sizeof(T) * num_of_elements, alignof(T));
	}

	//template<typename T>
	//class ObjectPool
	//{
	//public:
	//	template<typename... Args>
	//	static T* const PopObject(Args&&... args)noexcept {
	//		auto& pool_vec = ObjectPool<T>::GetPoolXVector();
	//		if (pool_vec.empty())return xnew<T>(std::forward<Args>(args)...);
	//		const auto node = pool_vec.back().release();
	//		pool_vec.pop_back();
	//		return std::construct_at<T>(node, std::forward<Args>(args)...);
	//	}
	//	static void PushObject(T* const ptr)noexcept {
	//		std::destroy_at<T>(ptr);
	//		GetPoolXVector().emplace_back(ptr);
	//	}
	//public:
	//	ObjectPool() = delete;
	//	ObjectPool(const ObjectPool&) = delete;
	//	ObjectPool(ObjectPool&&)noexcept = delete;
	//	ObjectPool& operator=(const ObjectPool&) = delete;
	//	ObjectPool& operator=(ObjectPool&&)noexcept = delete;
	//	~ObjectPool() = delete;
	//private:
	//	static auto& GetPoolXVector()noexcept { thread_local XVector<U_ptr<T>> pool_vec; return pool_vec; }
	//};
}