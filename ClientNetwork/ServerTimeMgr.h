#pragma once
#include "Singleton.hpp"

namespace NetHelper
{
	class ServerTimeMgr
		:public Singleton<ServerTimeMgr>
	{
		friend void CALLBACK TimerCallback(LPVOID lpArg, DWORD dwTimerLow, DWORD dwTimerHigh);
		friend class Singleton;
		ServerTimeMgr() = default;
		~ServerTimeMgr();
		static constexpr const uint64_t SERVER_TIME_UPDATE_INTERVAL = 10000;
		static constexpr const std::string_view SERVER_TIME_UPDATE_FUNC_KEY = "SERVER_TIME_UPDATE";
	public:
		void RegisterTimerFunc(const uint64_t timer_inverval_ms, std::function<void(void)> timer_callback, const std::string_view func_key_name, const bool delay_exec = true)noexcept;
		inline const uint64_t GetServerTimeStamp()const noexcept {
			return (NetHelper::GetTimeStampMilliseconds() - m_server_time_stamp_updated) + m_server_time_stamp;
		}
		void InitAndWaitServerTimeStamp(std::function<void(void)> ping_pong_fp)noexcept;
	public:
		void UpdateServerTimeStamp(const uint64_t received_server_time_stamp)noexcept {
			const auto called_time = m_func_called_time_stamp[SERVER_TIME_UPDATE_FUNC_KEY.data()];
			const auto cur_time = NetHelper::GetTimeStampMilliseconds();
			const uint64_t RTT_HALF = (cur_time - called_time) / 2;
			m_server_time_stamp = received_server_time_stamp + (RTT_HALF);
			m_server_time_stamp_updated = cur_time;
		}
		void SetTimeStampByName(const std::string_view key_name)noexcept {
			m_func_called_time_stamp[key_name.data()] = NetHelper::GetTimeStampMilliseconds();
		}
		const uint64_t GetTimeStampByName(const std::string_view key_name)noexcept { return m_func_called_time_stamp[key_name.data()]; }
		const uint64_t GetElapsedTime(const std::string_view key_name)noexcept {
			return NetHelper::GetTimeStampMilliseconds() - GetTimeStampByName(key_name);
		}
	private:
		uint64_t m_server_time_stamp_updated;
		uint64_t m_server_time_stamp = 0;
		std::vector<HANDLE> m_timer_handle_list;
		std::vector<std::unique_ptr<std::pair<std::string,std::function<void(void)>>>> m_callback_func_list;
		std::map<std::string, uint64> m_func_called_time_stamp;
	};
}

