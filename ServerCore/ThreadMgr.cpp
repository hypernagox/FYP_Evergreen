#include "ServerCorePch.h"
#include "ThreadMgr.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"
#include "TaskQueueable.h"
#include "Service.h"
#include "IocpCore.h"
#include "TaskTimerMgr.h"
#include "SendBufferMgr.h"
#include "ClusterUpdateQueue.h"
#include "FieldMgr.h"
#include "SendBufferChunk.h"

/*------------------
	ThreadMgr
-------------------*/

namespace ServerCore
{
	constinit extern thread_local uint64 LEndTickCount;
	constinit extern thread_local class TaskQueueable* LCurTaskQueue;

	//thread_local moodycamel::ProducerToken* LPro_token;
	//thread_local moodycamel::ConsumerToken* LCon_token;
	constinit thread_local moodycamel::ProducerToken* LPro_tokenGlobalTask;
	constinit thread_local moodycamel::ConsumerToken* LCon_tokenGlobalTask;

	ThreadMgr::ThreadMgr()
	{
		// Main Thread
		InitTLS();
		LThreadId = 1;
	}

	ThreadMgr::~ThreadMgr()
	{
		std::atomic_thread_fence(std::memory_order_seq_cst);
		if (!m_bStopRequest)
		{
			Join();
		}
		Task task;
		while (m_globalTask.try_dequeue(*LCon_tokenGlobalTask, task)) { std::destroy_at<Task>(&task); }

		DestroyTLS();

		//xdelete<moodycamel::ProducerToken>(LPro_token);
		//xdelete<moodycamel::ConsumerToken>(LCon_token);

		//xdelete<moodycamel::ProducerToken>(LPro_tokenGlobalTask);
		//xdelete<moodycamel::ConsumerToken>(LCon_tokenGlobalTask);
	}

	void ThreadMgr::Launch(const uint64_t num_of_threads, std::function<void(void)> destroyTLSCallBack, std::function<void(void)> initTLSCallBack)
	{
		m_iocpHandle = IocpCore::GetIocpHandleGlobal();
		g_initTLSCallBack.swap(initTLSCallBack);
		g_destroyTLSCallBack.swap(destroyTLSCallBack);
		m_threads.reserve(num_of_threads);

		for (int i = 0; i < num_of_threads; ++i)
		{
			m_threads.emplace_back(&ThreadMgr::WorkerThreadFunc);
		}

		while (g_threadID.load(std::memory_order_seq_cst) <= num_of_threads);
		std::atomic_thread_fence(std::memory_order_seq_cst);

		//m_timerThread = std::thread{ []()noexcept
		//	{
		//		Mgr(ThreadMgr)->InitTLS();
		//		const bool& bStopRequest = Mgr(ThreadMgr)->m_bStopRequest;
		//		TaskTimerMgr& taskTimer = *Mgr(TaskTimerMgr);
		//		for (;;)
		//		{
		//			if (bStopRequest) [[unlikely]]
		//				break;
		//
		//			taskTimer.DistributeTask();
		//
		//			std::this_thread::yield();
		//		}
		//		Mgr(ThreadMgr)->DestroyTLS();
		//	} };

		std::string strFin(32, 0);
		const auto main_service = const_cast<Service* const>(Service::GetMainService());
		if (SERVICE_TYPE::SERVER == main_service->GetServiceType())
		{
			NAGOX_ASSERT(ThreadMgr::NUM_OF_THREADS == num_of_threads);
			static std::atomic_bool registerFinish = false;
			while (!m_bStopRequest)
			{
				std::cin >> strFin;

				if ("EXIT" == strFin)
				{
					if (false == registerFinish.exchange(true))
					{
						main_service->CloseService();
						Mgr(Logger)->m_bStopRequest = true;
						Mgr(FieldMgr)->ClearField();
						std::this_thread::sleep_for(std::chrono::seconds(5));
						Join();
					}
				}

				std::this_thread::sleep_for(std::chrono::seconds(5));
			}
		}
	}

	void ThreadMgr::Join()
	{
		if (m_bStopRequest)
			return;
		m_bStopRequest = true;
		std::atomic_thread_fence(std::memory_order_seq_cst);
		for (int i = 0; i < NUM_OF_THREADS; ++i)
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		for (auto& t : m_threads)
		{
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
			t.join();
		}
		if (m_timerThread.joinable())
			m_timerThread.join();
	}

	void ThreadMgr::InitTLS()
	{
		LThreadId = g_threadID.fetch_add(1);

		//LPro_token = xnew<moodycamel::ProducerToken>(m_globalTaskQueue);
		thread_local moodycamel::ProducerToken pro_token{ m_globalTask };
		LPro_tokenGlobalTask = &pro_token;

		//LCon_token = xnew <moodycamel::ConsumerToken>(m_globalTaskQueue);
		thread_local moodycamel::ConsumerToken con_token{ m_globalTask };
		LCon_tokenGlobalTask = &con_token;

		if (NUM_OF_THREADS >= LThreadId && 0 < LThreadId) 
		{
			LSendBufferChunk = SendBufferMgr::Pop();
			if (g_initTLSCallBack)
			{
				g_initTLSCallBack();
			}
		}

		LRandSeed = std::uniform_int_distribution<uint32_t>{ 0, UINT32_MAX }(g_RandEngine);
	}

	void ThreadMgr::DestroyTLS()
	{
		if (NUM_OF_THREADS >= LThreadId && 0 < LThreadId)
		{
			if (g_destroyTLSCallBack)
			{
				g_destroyTLSCallBack();
			}
		}
		if (LSendBufferChunk)
			LSendBufferChunk->DecRef();
	}

	void ThreadMgr::TryGlobalQueueTask()noexcept
	{
		Task task;
		while (m_globalTask.try_dequeue(*LCon_tokenGlobalTask, task)) {
			task.ExecuteTask();
			//std::destroy_at<Task>(&task);
		}
	}
	void ThreadMgr::WorkerThreadFunc() noexcept
	{
		constinit extern thread_local uint64_t LEndTickCount;
		constinit extern thread_local uint64_t LCurHandleSessionID;

		Mgr(ThreadMgr)->InitTLS();

		TaskTimerMgr& taskTimer = *Mgr(TaskTimerMgr);
		const bool& bStopRequest = Mgr(ThreadMgr)->GetStopFlagRef();
		const HANDLE iocpHandle = IocpCore::GetIocpHandleGlobal();

		for (;;)
		{
			if (bStopRequest) [[unlikely]]
				break;

			IocpCore::Dispatch(iocpHandle, 10);
			taskTimer.DistributeTask();
			ClusterUpdateQueue::UpdateCluster();
		}

		Mgr(ThreadMgr)->DestroyTLS();
	}
}