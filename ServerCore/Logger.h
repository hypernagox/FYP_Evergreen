#pragma once

namespace ServerCore
{
	class Logger
		:public Singleton<Logger>
	{
		friend class Singleton;
		friend class ThreadMgr;
		Logger();
		~Logger();
	public:
		class Log
		{
			friend class Logger;
		private:
			Log(std::wstring&& msg)noexcept :logMsg{ std::move(msg) }
			{
				Mgr(Logger)->EnqueueLogMsg(std::format(L"{} Start", logMsg));
			}
		public:
			~Log()noexcept
			{
				Mgr(Logger)->EnqueueLogMsg(
					std::format(L"{} End: {}microseconds", std::move(logMsg), std::to_wstring(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startTime).count()))
				);
			}
		private:
			std::wstring logMsg;
			std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
		};
		void EnqueueLogMsg(std::wstring&& msg)noexcept
		{
			//m_msgQueue.emplace(std::move(msg));
			//m_msgCv.notify_one();
		}
		void EnqueueLogMsg(const wchar_t* const msg)noexcept
		{
			//m_msgQueue.emplace(msg);
			//m_msgCv.notify_one();
		}
		static Log CreateFuncLog(std::wstring_view logMsg_)noexcept { return Log{ std::format(L"FUNC_LOG: {}",logMsg_) }; }
		void Init()noexcept;
	private:
#if defined(TRACK_FUNC_LOG) || defined(TRACK_LOG)
		MPSCQueue<std::wstring> m_msgQueue;
#endif
		std::condition_variable m_msgCv;
		std::thread m_msgThread;
		std::mutex m_mt;
		bool m_bStopRequest = false;
	};
}
