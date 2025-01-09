#include "ClientNetworkPch.h"
#include "ServerTimeMgr.h"

namespace NetHelper
{
	void CALLBACK TimerCallback(LPVOID lpArg, DWORD dwTimerLow, DWORD dwTimerHigh)
	{
		std::cout << "PING PONG\n";
		const auto& [func_key, func_ptr] = *((std::pair<std::string, std::function<void(void)>>*)(lpArg));
		NetMgr(ServerTimeMgr)->m_func_called_time_stamp[func_key] = NetHelper::GetTimeStampMilliseconds();
		func_ptr();
	}

	ServerTimeMgr::~ServerTimeMgr()
	{
		for (const auto handle : m_timer_handle_list)
		{
			CancelWaitableTimer(handle);
			CloseHandle(handle);
		}
	}

	void ServerTimeMgr::RegisterTimerFunc(const uint64_t timer_inverval_ms, std::function<void(void)> timer_callback, const std::string_view func_key_name, const bool delay_exec) noexcept
	{
		const HANDLE timer_handle = CreateWaitableTimer(NULL, FALSE, NULL);

		if (!timer_handle)
		{
			std::cerr << "Failed to create timer!" << std::endl;
			return;
		}

		m_timer_handle_list.emplace_back(timer_handle);

		const auto callback_ptr = m_callback_func_list.emplace_back(std::make_unique<std::pair<std::string, std::function<void(void)>>>
			(func_key_name, std::move(timer_callback))).get();

		LARGE_INTEGER liDueTime;
		liDueTime.QuadPart = (-static_cast<LONGLONG>(timer_inverval_ms) * 10000LL) * delay_exec;
		if (!SetWaitableTimer(timer_handle, &liDueTime, (LONG)timer_inverval_ms, TimerCallback, callback_ptr, TRUE))
		{
			std::cerr << "Failed to set timer!" << std::endl;
			CloseHandle(timer_handle);
			return;
		}
	}
	void ServerTimeMgr::InitAndWaitServerTimeStamp(std::function<void(void)> ping_pong_fp) noexcept
	{
		std::this_thread::yield();
		m_func_called_time_stamp[SERVER_TIME_UPDATE_FUNC_KEY.data()] = NetHelper::GetTimeStampMilliseconds();
		while (0 == m_server_time_stamp) { 
			m_server_time_stamp_updated = NetHelper::GetTimeStampMilliseconds();
			NetMgr(NetworkMgr)->DoNetworkIO();
		}
		RegisterTimerFunc(SERVER_TIME_UPDATE_INTERVAL, std::move(ping_pong_fp), SERVER_TIME_UPDATE_FUNC_KEY, false);
	}
}
