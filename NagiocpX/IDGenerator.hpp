#pragma once
#include "NagiocpXPch.h"

class IDGenerator
{
public:
	IDGenerator() = delete;
	~IDGenerator() = delete;
public:
	static inline c_uint64 GenerateID()noexcept { return InterlockedIncrement64((LONG64*)&g_objectID); }
private:
	__declspec(align(8)) constinit static inline volatile ULONG64 g_objectID = 0;
};