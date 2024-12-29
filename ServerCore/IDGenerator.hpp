#pragma once
#include "ServerCorePch.h"

class IDGenerator
{
public:
	IDGenerator() = delete;
	~IDGenerator() = delete;
public:
	static c_uint64 GenerateID()noexcept { return InterlockedIncrement64((LONG64*)&g_objectID); }
private:
	__declspec(align(8)) constinit static inline volatile ULONG64 g_objectID = 0;
};