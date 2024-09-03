#pragma once
#include "ServerCorePch.h"

class IDGenerator
{
public:
	IDGenerator() = delete;
	~IDGenerator() = delete;
public:
	static c_uint64 GenerateID()noexcept { return g_objectID.fetch_add(1, std::memory_order_relaxed); }
private:
	constinit static inline std::atomic<uint64_t> g_objectID = 1;
};