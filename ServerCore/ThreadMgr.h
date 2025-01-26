#pragma once
#include "Task.h"

/*------------------
	ThreadMgr
-------------------*/

namespace ServerCore
{
	class Initiator
	{
	public:
		virtual void GlobalInitialize()noexcept = 0;
		virtual void TLSInitialize()noexcept = 0;
		virtual void TLSDestroy()noexcept = 0;
		virtual void GlobalDestroy()noexcept = 0;
		virtual void ControlThreadFunc()noexcept = 0;
	};

	class TaskQueueable;
	class Service;
	class Task;

	//extern thread_local moodycamel::ProducerToken* LPro_token;
	//extern thread_local moodycamel::ConsumerToken* LCon_token;
	constinit extern thread_local moodycamel::ProducerToken* LPro_tokenGlobalTask;
	constinit extern thread_local moodycamel::ConsumerToken* LCon_tokenGlobalTask;
	constinit extern thread_local int8_t LThreadContainerIndex;

	class ThreadMgr
		:public Singleton<ThreadMgr>
	{
		friend class Singleton;
		ThreadMgr();
		~ThreadMgr();
		struct LFQueueAllocator
			:public moodycamel::ConcurrentQueueDefaultTraits
		{
			static inline void* const malloc(const size_t size)noexcept { return  Memory::Alloc(size); }
			static inline void free(void* const ptr)noexcept { return Memory::Free(ptr); }
		};
	public:
		static constexpr const uint64 NUM_OF_THREADS = ServerCore::NUM_OF_THREADS;
		void Launch(const uint64_t num_of_threads, Initiator* const initiator);
		static inline const int32_t GetCurThreadNumber()noexcept { 
			constinit extern thread_local int8_t LThreadContainerIndex;
			return LThreadContainerIndex + 1;
		}
		static inline const int8_t GetCurThreadIdx()noexcept {
			constinit extern thread_local int8_t LThreadContainerIndex;
			return LThreadContainerIndex;
		}
		const bool IsServerFinish()const noexcept { return m_bStopRequest; }
		void NotifyThread()const noexcept { PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0); }
	public:
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...)noexcept, const S_ptr<U>& memFuncInstance, Args&&... args)noexcept
		{
			//m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(memFunc, memFuncInstance, std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, (ULONG_PTR)xnew<Task>(Task(memFunc, memFuncInstance, std::forward<Args>(args)...)), 0);
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...)noexcept, S_ptr<U>&& memFuncInstance, Args&&... args)noexcept
		{
			//m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(memFunc, std::move(memFuncInstance), std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, (ULONG_PTR)xnew<Task>(Task(memFunc, std::move(memFuncInstance), std::forward<Args>(args)...)), 0);
		}
		template<typename T, typename U, typename Ret, typename... Args> requires EnableSharedFromThis<U> && std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...)noexcept, U* const memFuncInstance, Args&&... args)noexcept
		{
			//m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(memFunc, memFuncInstance, std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, (ULONG_PTR)xnew<Task>(Task(memFunc, memFuncInstance, std::forward<Args>(args)...)), 0);
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...), const S_ptr<U>& memFuncInstance, Args&&... args)noexcept
		{
			//m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(memFunc, memFuncInstance, std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, (ULONG_PTR)xnew<Task>(Task(memFunc, memFuncInstance, std::forward<Args>(args)...)), 0);
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...), S_ptr<U>&& memFuncInstance, Args&&... args)noexcept
		{
			//m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(memFunc, std::move(memFuncInstance), std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, (ULONG_PTR)xnew<Task>(Task(memFunc, std::move(memFuncInstance), std::forward<Args>(args)...)), 0);
		}
		template<typename T, typename U, typename Ret, typename... Args> requires EnableSharedFromThis<U> && std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...), U* const memFuncInstance, Args&&... args)noexcept
		{
			//m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(memFunc, memFuncInstance, std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, (ULONG_PTR)xnew<Task>(Task(memFunc, memFuncInstance, std::forward<Args>(args)...)), 0);
		}
		template<typename Func, typename... Args> requires std::invocable<Func, Args...>&& IsNotMemFunc<Func>
		void EnqueueGlobalTask(Func&& fp, Args&&... args)noexcept
		{
			//m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(std::forward<Func>(fp), std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, (ULONG_PTR)xnew<Task>(Task(std::forward<Func>(fp), std::forward<Args>(args)...)), 0);
		}
		void EnqueueGlobalTask(Task&& task_)noexcept {
			//m_globalTask.enqueue(*LPro_tokenGlobalTask, std::move(task_));
			PostQueuedCompletionStatus(m_iocpHandle, 0, (ULONG_PTR)xnew<Task>(std::move(task_)), 0);
		}
		//void EnqueueGlobalTaskBulk(Task* const taskBulks, const std::size_t num_of_task)noexcept {
		//	m_globalTask.enqueue_bulk(*LPro_tokenGlobalTask, taskBulks, num_of_task);
		//	PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		//}
		template<typename Func, typename... Args>
			requires std::invocable<Func, Args...>
		std::future<std::invoke_result_t<Func, Args...>> EnqueueGlobalTaskFuture(Func&& fp, Args&&... args) noexcept
		{
			using return_type = std::invoke_result_t<Func, Args...>;
			auto task = MakeUnique<std::packaged_task<return_type(void)>>(std::bind_front(std::forward<Func>(fp), std::forward<Args>(args)...));
			std::future<return_type> res_future = task->get_future();
			//EnqueueGlobalTask(Task(([task = std::move(task)]() noexcept {(*task)(); })));
			PostQueuedCompletionStatus(m_iocpHandle, 0, (ULONG_PTR)xnew<Task>(Task(([task = std::move(task)]() noexcept {(*task)(); }))), 0);
			return res_future;
		}
		const bool& GetStopFlagRef()const noexcept { return m_bStopRequest; }
	private:
		static void InitTLS()noexcept;
		static void DestroyTLS()noexcept;
		void Join();
		void TryGlobalQueueTask()noexcept;
	private:
		HANDLE m_iocpHandle;
		bool m_bStopRequest = false;
		
		//std::thread m_timerThread;

		moodycamel::ConcurrentQueue<Task, LFQueueAllocator> m_globalTask{ 32 };

		static inline std::thread m_threads[ThreadMgr::NUM_OF_THREADS];
		constinit static inline std::thread::id g_mainThreadID;
		constinit static inline std::atomic<int32_t> g_threadID = 0;
		constinit static inline Initiator* g_initiator = nullptr;
	private:
		static void LaunchInternal(const int8_t num_of_threads)noexcept;
		static void WorkerThreadFunc()noexcept;
		static void IocpRoutine()noexcept;
		void ThreadControlFunc()noexcept;
	};
}

