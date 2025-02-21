#pragma once
#include "Singleton.hpp"

namespace NagiocpX
{
	class TimeMgr
		:public Singleton<TimeMgr>
	{
		friend class Singleton;
		TimeMgr()noexcept = default;
		~TimeMgr()noexcept = default;
	public:
		inline const uint64_t GetServerStartTime()const noexcept { return m_server_start_time; }
		inline const uint64_t GetServerTimeStamp()const noexcept { return ::GetTickCount64() - m_server_start_time; }
		inline const uint32_t GetServerTimeStamp32()const noexcept { return ::GetTickCount() - static_cast<DWORD>(m_server_start_time); }
	private:
		const uint64_t m_server_start_time = ::GetTickCount64();
	};
}

