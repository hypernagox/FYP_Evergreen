#pragma once
#include "Task.h"

/*------------------
	ThreadMgr
-------------------*/

namespace ServerCore
{
	class TaskQueueable;
	class Service;
	class Task;

	//extern thread_local moodycamel::ProducerToken* LPro_token;
	//extern thread_local moodycamel::ConsumerToken* LCon_token;
	constinit extern thread_local moodycamel::ProducerToken* LPro_tokenGlobalTask;
	constinit extern thread_local moodycamel::ConsumerToken* LCon_tokenGlobalTask;

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
		void Launch(const uint64_t num_of_threads, std::function<void(void)> destroyTLSCallBack = nullptr, std::function<void(void)> initTLSCallBack = nullptr);
		static c_uint32 GetCurThreadID()noexcept { 
			constinit extern thread_local uint32_t LThreadId;
			return LThreadId;
		}
		static c_uint32 GetCurThreadIdx()noexcept {
			constinit extern thread_local uint32_t LThreadId;
			return LThreadId - 1;
		}
		const bool IsServerFinish()const noexcept { return m_bStopRequest; }
		void NotifyThread()const noexcept { PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0); }
	public:
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...)noexcept, const S_ptr<U>& memFuncInstance, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(memFunc, memFuncInstance, std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...)noexcept, S_ptr<U>&& memFuncInstance, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(memFunc, std::move(memFuncInstance), std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		}
		template<typename T, typename U, typename Ret, typename... Args> requires EnableSharedFromThis<U> && std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...)noexcept, U* const memFuncInstance, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(memFunc, memFuncInstance, std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...), const S_ptr<U>& memFuncInstance, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(memFunc, memFuncInstance, std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...), S_ptr<U>&& memFuncInstance, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(memFunc, std::move(memFuncInstance), std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		}
		template<typename T, typename U, typename Ret, typename... Args> requires EnableSharedFromThis<U> && std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...), U* const memFuncInstance, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(memFunc, memFuncInstance, std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		}
		template<typename Func, typename... Args> requires std::invocable<Func, Args...>&& IsNotMemFunc<Func>
		void EnqueueGlobalTask(Func&& fp, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(std::forward<Func>(fp), std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		}
		void EnqueueGlobalTask(Task&& task_)noexcept {
			m_globalTask.enqueue(*LPro_tokenGlobalTask, std::move(task_));
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		}
		void EnqueueGlobalTaskBulk(Task* const taskBulks, const std::size_t num_of_task)noexcept {
			m_globalTask.enqueue_bulk(*LPro_tokenGlobalTask, taskBulks, num_of_task);
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		}
		template<typename Func, typename... Args>
			requires std::invocable<Func, Args...>
		std::future<std::invoke_result_t<Func, Args...>> EnqueueGlobalTaskFuture(Func&& fp, Args&&... args) noexcept
		{
			using return_type = std::invoke_result_t<Func, Args...>;
			auto task = ::MakeUnique<std::packaged_task<return_type(void)>>(std::bind_front(std::forward<Func>(fp), std::forward<Args>(args)...));
			std::future<return_type> res_future = task->get_future();
			EnqueueGlobalTask(Task(([task = std::move(task)]() noexcept {(*task)(); })));
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
			return res_future;
		}
		const bool& GetStopFlagRef()const noexcept { return m_bStopRequest; }
	public:
		void InitTLS();
		void DestroyTLS();
		void Join();
		void TryGlobalQueueTask()noexcept;
	private:
		bool m_bStopRequest = false;
		HANDLE m_iocpHandle;
		moodycamel::ConcurrentQueue<Task, LFQueueAllocator> m_globalTask{ 32 };
		std::vector<std::thread> m_threads;
		std::thread m_timerThread;
		

		constinit static inline std::atomic<uint32_t> g_threadID = 0;
		static inline std::function<void(void)> g_initTLSCallBack = {};
		static inline std::function<void(void)> g_destroyTLSCallBack = {};
		enum { WORKER_TICK = 64 };

	private:
		static void WorkerThreadFunc()noexcept;
	};
}

