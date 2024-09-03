#pragma once
#include <stack>
#include <random>
#include "RefCountable.h"

namespace ServerCore
{
	extern thread_local std::stack<int32> LLockStack;
	extern thread_local S_ptr<class SendBufferChunk> LSendBufferChunk;

	constinit extern thread_local uint32_t LThreadId;
	constinit extern thread_local uint64_t LEndTickCount;
	constinit extern thread_local class TaskQueueable* LCurTaskQueue;
	constinit extern thread_local uint64_t LCurHandleSessionID;
	constinit extern thread_local uint32_t LRandSeed;
}

extern std::mt19937 g_RandEngine;