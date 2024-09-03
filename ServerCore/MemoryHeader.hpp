#pragma once

/*-----------------
	MemoryHeader
------------------*/

namespace ServerCore
{
	struct alignas(8) MemoryHeader
	{
		// [MemoryHeader][Data]
		constexpr MemoryHeader(const int32 size, const int32 thID)noexcept : allocSize{ size }, allocThread{ thID } {}
		constexpr MemoryHeader(const int32 thID)noexcept : allocThread{ thID } {}

		constexpr static void* const AttachHeader(MemoryHeader* const header, const int32 size, const int32 thID)noexcept
		{
			return new(header)MemoryHeader{ size, thID } + 1;
		}

		constexpr static MemoryHeader* const DetachHeader(void* const ptr)noexcept
		{
			return static_cast<MemoryHeader* const>(ptr) - 1;
		}
		//alignas(8) size_t allocSize;
		int32 allocSize;
		c_int32 allocThread;
	};
}