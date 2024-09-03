#pragma once

namespace ServerCore
{
	class Memory;

	/*-------------------
		StompAllocator
	-------------------*/

	class StompAllocator
	{
		enum { PAGE_SIZE = 0x1000 };

	public:
		static void* Alloc(const size_t size);
		static void		Release(void* ptr);
	};

	/*-------------------
		STL Allocator
	-------------------*/

	template<typename T>
	class StlAllocator
	{
	public:
		using value_type = T;

		constexpr StlAllocator()noexcept {}

		template<typename Other>
		StlAllocator(const StlAllocator<Other>&) {}

		constexpr static T* const allocate(const size_t count)noexcept { return Memory::Alloc<T>(sizeof(T) * count); }

		constexpr static void deallocate(T* const ptr, const size_t count)noexcept { return Memory::Free_Sized(ptr, static_cast<const uint32_t>(sizeof(T) * count)); }

		friend constexpr const bool operator==(const StlAllocator<T>&, const StlAllocator<T>&)noexcept { return true; }

		friend constexpr const bool operator!=(const StlAllocator<T>& a, const StlAllocator<T>& b)noexcept { return !(a == b); }
	};

	template<typename T>
	class StlAllocator64
	{
	public:
		using value_type = T;

		constexpr StlAllocator64()noexcept {}

		template<typename Other>
		StlAllocator64(const StlAllocator64<Other>&) {}

		constexpr static T* const allocate(const size_t count)noexcept
		{
			return static_cast<T* const>(::tc_new_aligned_nothrow(sizeof(T) * count, std::align_val_t{ 64 }, std::nothrow_t{}));
		}

		constexpr static void deallocate(T* const ptr, const size_t count)noexcept
		{
			return ::tc_delete_sized_aligned(ptr, sizeof(T) * count, std::align_val_t{ 64 });
		}

		friend constexpr const bool operator==(const StlAllocator64<T>&, const StlAllocator64<T>&)noexcept { return true; }

		friend constexpr const bool operator!=(const StlAllocator64<T>& a, const StlAllocator64<T>& b)noexcept { return !(a == b); }
	};

	template <typename T>
	struct UDeleter { constexpr inline void operator()(T* const ptr) const noexcept { xdelete<T>(ptr); } };

	template <typename T>
	struct USDeleter { constexpr inline void operator()(T* const ptr) const noexcept { xdelete_sized<T>(ptr, sizeof(T)); } };
}