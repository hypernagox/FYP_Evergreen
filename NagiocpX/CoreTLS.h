#pragma once
#include <stack>
#include <random>
#include "RefCountable.h"

namespace NagiocpX
{
	extern thread_local std::stack<int32> LLockStack;

	constinit extern thread_local int8_t LThreadContainerIndex;
	constinit extern thread_local class SendBufferChunk* LSendBufferChunk;
	constinit extern thread_local uint64_t LEndTickCount;
	constinit extern thread_local class TaskQueueable* LCurTaskQueue;
	constinit extern thread_local class Queueabler* LCurQueueableComponent;
	constinit extern thread_local uint32_t LRandSeed;

	extern thread_local std::mt19937 LRandEngine;
}
