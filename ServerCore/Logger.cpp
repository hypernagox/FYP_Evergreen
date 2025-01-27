#include "ServerCorePch.h"
#include "Logger.h"

namespace ServerCore
{
	Logger::Logger()
	{
	}

	Logger::~Logger()
	{
		m_bStopRequest = true;
		m_msgCv.notify_all();
		std::atomic_thread_fence(std::memory_order_seq_cst);
		if (m_msgThread.joinable())
		{
			m_msgThread.join();
		}
		std::atomic_thread_fence(std::memory_order_seq_cst);
		//for (const auto& msg : m_msgQueue.try_flush_single())
		//{
		//	std::wcout << msg << L"\n";
		//}
		// 부검 메세지
	}

	void Logger::Init() noexcept
	{
//#if defined(TRACK_FUNC_LOG) || defined(TRACK_LOG)
//		m_msgThread = std::thread{ [this]()noexcept
//			{
//				while (!m_bStopRequest)
//				{
//					std::wstring msg = {};
//					{
//						std::unique_lock<std::mutex> lock{ m_mt };
//						m_msgCv.wait(lock, [this]()noexcept {return !m_msgQueue.empty_single() || m_bStopRequest; });
//					}
//					while (m_msgQueue.try_pop_single(msg) && !m_bStopRequest)
//					{
//						std::wcout << msg << L"\n";
//						if (msg.starts_with(L"FUNC_LOG"))
//							std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//					}
//				}
//			}
//		};
//#endif
	}
}