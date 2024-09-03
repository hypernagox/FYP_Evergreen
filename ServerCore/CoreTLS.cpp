#include "ServerCorePch.h"
#include "CoreTLS.h"

namespace ServerCore
{
	thread_local std::stack<int32> LLockStack = {};
	thread_local S_ptr<class SendBufferChunk> LSendBufferChunk = {};

	constinit thread_local uint32_t LThreadId = 1;
	constinit thread_local uint64_t LEndTickCount = 0;
	constinit thread_local class TaskQueueable* LCurTaskQueue = nullptr;
	constinit thread_local class Queueabler* LCurQueueableComponent = nullptr;
	constinit thread_local uint64_t LCurHandleSessionID = 0;
	constinit thread_local uint32_t LRandSeed = {};
}

std::mt19937 g_RandEngine{ std::random_device{}() };