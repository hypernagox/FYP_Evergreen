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
	void CoreGlobal::Init() noexcept
	{
		Mgr(BenchmarkMgr)->RegisterDestroy();


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
}