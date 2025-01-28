#include "ServerCorePch.h"
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

namespace ServerCore
{
	CoreGlobal::CoreGlobal()
		:m_iocpCore{ SocketUtils::Init() }
	{
	}

	CoreGlobal::~CoreGlobal()
	{
		SocketUtils::Clear();
	}

	void CoreGlobal::Init()const noexcept
	{
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

		_CrtMemDumpStatistics(&mem_start);

		Mgr(BenchmarkMgr)->RegisterDestroy();

		std::atexit([]() {Mgr(CoreGlobal)->ExitRoutine(); });

		Mgr(CoreGlobal)->RegisterDestroy();
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

#ifdef _DEBUG
		_CrtMemState mem_check;
		_CrtMemDumpStatistics(&mem_end);

		if (_CrtMemDifference(&mem_check, &mem_start, &mem_end))
			_CrtMemDumpStatistics(&mem_check);
#endif
	}
}