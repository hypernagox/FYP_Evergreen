#pragma once
#include "NetAddress.h"
#include "PacketSession.h"

namespace NetHelper
{
	class NetworkMgr
		:public Singleton<NetworkMgr>
	{
		friend class Singleton;
		NetworkMgr();
		~NetworkMgr();
		using PacketHandleFunc = const bool(*)(const S_ptr<PacketSession>&, const BYTE* const, c_int32);
	public:
		template <typename T>requires std::derived_from<T, PacketSession>
		bool Connect(std::wstring_view ip, uint16 port, const PacketHandleFunc* const handler, std::function<void(void)> disconnectFp = nullptr)
		{
			if (m_sessionFactory)
				NET_NAGOX_ASSERT(false);
			m_disconnectCallback = std::move(disconnectFp);
			m_sessionFactory = std::make_shared<T>;
			return Connect(ip, port, handler);
		}
		void DoNetworkIO(const DWORD timeout_millisecond = 0)noexcept;
		void Send(S_ptr<class SendBuffer> pSendBuffer)const noexcept { 
			if (!m_c2sSession)return;
			m_c2sSession->Send(std::move(pSendBuffer)); 
		}
		const S_ptr<PacketSession>& GetSession()const noexcept { return m_c2sSession; }
		const NetAddress& GetServerAddr()const noexcept { return m_serverAddr; }
		void SetSessionID(const uint32_t sessionID_)const noexcept;
		c_uint32 GetSessionID()const noexcept;
		void FinishNetwork()noexcept;
	private:
		bool Connect(std::wstring_view ip, uint16 port, const PacketHandleFunc* const handler)noexcept;
	private:
		S_ptr<PacketSession> m_c2sSession;
		WSAEVENT m_connectEvent;
	private:
		static inline std::function<void(void)> m_disconnectCallback = nullptr;
		static inline std::function<S_ptr<PacketSession>(void)> m_sessionFactory = nullptr;
		static inline NetAddress m_serverAddr = {};
	};
}

