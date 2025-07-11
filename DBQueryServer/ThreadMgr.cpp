#include "pch.h"
#include "ThreadMgr.h"
#include "s2q_PacketHandler.h"

ThreadMgr::ThreadMgr()
{
}

ThreadMgr::~ThreadMgr()
{
	m_bStopRequest = true;
	for (auto& t : m_threads)
		t.join();
}

void ThreadMgr::Init() noexcept
{
}

void ThreadMgr::Launch(const int32_t numOfThreads)
{
	m_threads.reserve(numOfThreads);
	for (int i = 0; i < numOfThreads; ++i)
	{
		m_threads.emplace_back([this]()noexcept
			{
				const bool& bStopRequest = m_bStopRequest;
				DBPacketInfo dbInfo;
				for (;;)
				{
					if (true == bStopRequest)[[unlikely]]
						return;
					if (false == m_dbInfoQueue.try_pop(dbInfo))
					{
						std::this_thread::yield();
						continue;
					}
					auto& bytes = dbInfo.bytes;
					s2q_PacketHandler::g_fpPacketHandler[dbInfo.pkt_id]((char*)bytes.data());
				}
			});
	}
}
