#include "NagiocpXPch.h"
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
#include "MoveBroadcaster.h"
#include "GlobalEventQueue.h"

/*------------------
	ThreadMgr
-------------------*/

namespace NagiocpX
{
	constinit extern thread_local int8_t LThreadContainerIndex;
	constinit extern thread_local uint64 LEndTickCount;
	constinit extern thread_local class TaskQueueable* LCurTaskQueue;

	//thread_local moodycamel::ProducerToken* LPro_token;
	//thread_local moodycamel::ConsumerToken* LCon_token;
	//constinit thread_local moodycamel::ProducerToken* LPro_tokenGlobalTask;
	//constinit thread_local moodycamel::ConsumerToken* LCon_tokenGlobalTask;

	extern thread_local VectorSetUnsafe<std::pair<uint32_t, const ContentsEntity*>, XHashMap> new_view_list_session;
	extern thread_local VectorSetUnsafe<const ContentsEntity*, XHashMap> new_view_list_npc;
	extern thread_local XVector<S_ptr<SendBuffer>> clear_vec;

	constinit extern thread_local XVector<const ContentsEntity*>* LXVectorForTempCopy;
	
	ThreadMgr::ThreadMgr()
	{
		g_mainThreadID = std::this_thread::get_id();
	}

	ThreadMgr::~ThreadMgr()
	{
		std::atomic_thread_fence(std::memory_order_seq_cst);
		if (!m_bStopRequest)
		{
			Join();
		}
		Task task;
		//while (m_globalTask.try_dequeue(*LCon_tokenGlobalTask, task)) { std::destroy_at<Task>(&task); }

		//DestroyTLS();

		//xdelete<moodycamel::ProducerToken>(LPro_token);
		//xdelete<moodycamel::ConsumerToken>(LCon_token);

		//xdelete<moodycamel::ProducerToken>(LPro_tokenGlobalTask);
		//xdelete<moodycamel::ConsumerToken>(LCon_tokenGlobalTask);
	}

	void ThreadMgr::Launch(const uint64_t num_of_threads, Initiator* const initiator)
	{
		m_iocpHandle = NagiocpX::GetIocpHandleGlobal();
		g_initiator = initiator;

		if (Service::GetMainService()->GetServiceType() == SERVICE_TYPE::SERVER) {
			NAGOX_ASSERT(ThreadMgr::NUM_OF_THREADS == num_of_threads);
			NAGOX_ASSERT(nullptr != MoveBroadcaster::GetGlobalBroadcastHelper());
		}
		NAGOX_ASSERT(nullptr != initiator && nullptr != Session::GetGlobalSessionPacketHandleFunc());
		
		m_threads[0] = std::thread{ &ThreadMgr::LaunchInternal,(int8_t)num_of_threads };

		ThreadControlFunc();
		
		Join();

		// TODO: 타이머 스레드를 빼는게 나을지는 나중에 몹 많아지면 판단.
		//m_timerThread = std::thread{ []()noexcept
		//	{
		//		//Mgr(ThreadMgr)->InitTLS();
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
		//		//Mgr(ThreadMgr)->DestroyTLS();
		//	} };

		
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
			if (t.joinable())
				t.join();
		}
		//if (m_timerThread.joinable())
		//	m_timerThread.join();
	}

	void ThreadMgr::InitTLS()noexcept
	{
		NAGOX_ASSERT(g_mainThreadID != std::this_thread::get_id());
		NAGOX_ASSERT_LOG(nullptr != Session::GetGlobalSessionPacketHandleFunc(), "Session PacketHandle Func Not Init");
		SendBufferMgr::InitTLSChunkPool();
		constinit extern thread_local int8_t LThreadContainerIndex;
		LThreadContainerIndex = (int8_t)(g_threadID.fetch_add(1));

		//LPro_token = xnew<moodycamel::ProducerToken>(m_globalTaskQueue);
		//thread_local moodycamel::ProducerToken pro_token{ Mgr(ThreadMgr)->m_globalTask };
		//LPro_tokenGlobalTask = &pro_token;

		//LCon_token = xnew <moodycamel::ConsumerToken>(m_globalTaskQueue);
		//thread_local moodycamel::ConsumerToken con_token{ Mgr(ThreadMgr)->m_globalTask };
		//LCon_tokenGlobalTask = &con_token;

		LXVectorForTempCopy = &new_view_list_npc.GetItemListRef();

		
		{
			
			{
				new_view_list_session.reserve(DEFAULT_MEM_POOL_SIZE * 4);
				new_view_list_npc.reserve(DEFAULT_MEM_POOL_SIZE * 4);
				clear_vec.reserve(DEFAULT_MEM_POOL_SIZE);
			}

			LSendBufferChunk = SendBufferMgr::Pop();

			Mgr(FieldMgr)->InitTLSinField();

			g_initiator->TLSInitialize();
		}

		LRandSeed = std::uniform_int_distribution<uint32_t>{ 0, UINT32_MAX }(LRandEngine);
	}

	void ThreadMgr::DestroyTLS()noexcept
	{
		NAGOX_ASSERT(g_mainThreadID != std::this_thread::get_id());

		Mgr(FieldMgr)->DestroyTLSinField();

		g_initiator->TLSDestroy();

		if (LSendBufferChunk)
			LSendBufferChunk->DecRef<SendBufferChunk>();

		SendBufferMgr::DestroyTLSChunkPool();
	}

	void ThreadMgr::TryGlobalQueueTask()noexcept
	{
		//Task task;
		//while (m_globalTask.try_dequeue(*LCon_tokenGlobalTask, task)) {
		//	task.ExecuteTask();
		//	//std::destroy_at<Task>(&task);
		//}
	}

	void ThreadMgr::LaunchInternal(const int8_t num_of_threads) noexcept
	{
		InitTLS();

		g_initiator->GlobalInitialize();

		Mgr(FieldMgr)->ShrinkToFitBeforeStart();

		for (int i = 1; i < num_of_threads; ++i)Mgr(ThreadMgr)->m_threads[i] = std::thread{ &ThreadMgr::WorkerThreadFunc };
		
		IocpRoutine();

		DestroyTLS();
	}

	void ThreadMgr::WorkerThreadFunc() noexcept
	{
		InitTLS();

		IocpRoutine();

		DestroyTLS();
	}

	void ThreadMgr::IocpRoutine() noexcept
	{
		constinit extern thread_local int8_t LThreadContainerIndex;
		constinit extern thread_local uint64_t LEndTickCount;
		constinit extern thread_local uint64_t LCurHandleSessionID;

		TaskTimerMgr& taskTimer = *Mgr(TaskTimerMgr);
		const HANDLE iocpHandle = NagiocpX::GetIocpHandleGlobal();
		for(;;) [[likely]]
		{
			if (IocpCore::Dispatch(iocpHandle)) [[likely]]{
				taskTimer.DistributeTask();
				GlobalEventQueue::TryGlobalEvent();
				ClusterUpdateQueue::UpdateCluster();
			}
			else [[unlikely]] break;
		}
	}

	void ThreadMgr::ThreadControlFunc() noexcept
	{
		g_initiator->ControlThreadFunc();
		EnqueueGlobalTask([]()
			{
			g_initiator->GlobalDestroy();
			Mgr(FieldMgr)->ClearField();
			const_cast<Service* const>(Service::GetMainService())->CloseService();
			Mgr(Logger)->m_bStopRequest = true;
			});
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
}