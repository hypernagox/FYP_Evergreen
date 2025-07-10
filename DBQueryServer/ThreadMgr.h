#pragma once

struct DBPacketInfo
{
	uint8_t pkt_id;
	std::vector<BYTE> bytes;
};

class ThreadMgr
	:public Singleton<ThreadMgr>
{
	friend class Singleton;
	ThreadMgr();
	~ThreadMgr();
public:
	virtual void Init()noexcept override;
	void EnqueueDBPacket(const uint8_t pkt_id,std::vector<BYTE>&& bytes)noexcept
	{
		m_dbInfoQueue.push({ pkt_id,std::move(bytes) });
	}
	void Launch(const int32_t numOfThreads);
private:
	bool m_bStopRequest = false;
	concurrency::concurrent_queue<DBPacketInfo> m_dbInfoQueue;
	std::vector<std::jthread> m_threads;
};

