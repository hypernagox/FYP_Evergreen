#include "NagiocpXPch.h"
#include "CoreTLS.h"

namespace NagiocpX
{
	thread_local std::stack<int32> LLockStack = {};

	constinit thread_local int8_t LThreadContainerIndex = 0;
	constinit thread_local class SendBufferChunk* LSendBufferChunk = nullptr;
	constinit thread_local uint64_t LEndTickCount = 0;
	constinit thread_local class TaskQueueable* LCurTaskQueue = nullptr;
	constinit thread_local class Queueabler* LCurQueueableComponent = nullptr;
	constinit thread_local uint32_t LRandSeed = {};

	thread_local std::mt19937 LRandEngine{ std::random_device{}() };
}