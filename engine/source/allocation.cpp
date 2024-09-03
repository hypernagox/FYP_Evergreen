#include "pch.h"

#ifdef TRACY_ENABLE

constexpr int STACKTRACE_SIZE = 16;

void* operator new(std::size_t count) throw(std::bad_alloc)
{
	auto ptr = malloc(count);
	if (!ptr)
		throw std::bad_alloc();
	TracyAllocS(ptr, count, STACKTRACE_SIZE);
	return ptr;
}

void operator delete(void* ptr) noexcept
{
	TracyFreeS(ptr, STACKTRACE_SIZE);
	free(ptr);
}

#endif