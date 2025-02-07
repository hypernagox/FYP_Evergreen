#include "NagiocpXPch.h"
#include "CoreGlobal.h"
#include "ThreadMgr.h"
#include "MemoryMgr.h"
#include "DeadLockProfiler.h"
#include "SocketUtils.h"
#include "SendBufferMgr.h"
#include "TaskTimerMgr.h"
#include "Logger.h"
#include "DBMgr.h"
#include "FieldMgr.h"
#include "TimeMgr.h"
#include "Benchmarker.h"
#include "Service.h"
#include "ClusterUpdateQueue.h"

namespace NagiocpX
{
	CoreGlobal::CoreGlobal()
		:m_iocpCore{ SocketUtils::Init() }
	{
	}

	CoreGlobal::~CoreGlobal()
	{
		SocketUtils::Clear();
	}

	void CoreGlobal::Init()noexcept
	{
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		
		GlobalEventQueue::Init();
		ClusterUpdateQueue::Init();

		std::atexit(CoreGlobal::ExitRoutine);
		Mgr(CoreGlobal)->RegisterDestroy();

		Mgr(BenchmarkMgr)->RegisterDestroy();

		
		Mgr(TimeMgr)->RegisterDestroy();
		Mgr(Logger)->RegisterDestroy();
		Mgr(ThreadMgr)->RegisterDestroy();
		Mgr(DeadLockProfiler)->RegisterDestroy();
		Mgr(TaskTimerMgr)->RegisterDestroy();
		//Mgr(DBMgr)->RegisterDestroy();
		Mgr(FieldMgr)->RegisterDestroy();


		Mgr(BenchmarkMgr)->Init();
		Mgr(TimeMgr)->Init();
		Mgr(Logger)->Init();
		//Mgr(MemoryMgr)->Init();
		Mgr(ThreadMgr)->Init();
		Mgr(DeadLockProfiler)->Init();
		Mgr(TaskTimerMgr)->Init();
		//Mgr(SendBufferMgr)->Init();
		//Mgr(DBMgr)->Init();
		Mgr(FieldMgr)->Init();
	}

	void CoreGlobal::ExitRoutine() noexcept
	{
		delete Service::GetMainService();
		SendBufferMgr::DestroyTLSChunkPool();
		ClusterUpdateQueue::Free();
		GlobalEventQueue::Free();
#ifdef _DEBUG
		if (!_CrtCheckMemory()) 
		{
			PrintLogEndl("\n\n\nHeap corruption detected!\n");
		}
#endif
	}
}