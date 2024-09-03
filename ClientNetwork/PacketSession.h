#pragma once
#include "Session.h"

namespace NetHelper
{
	class PacketSession
		:public Session
	{
		using PacketHandleFunc = const bool(*)(const S_ptr<PacketSession>&, const BYTE* const, c_int32);
		friend class NetworkMgr;
	public:
		PacketSession();
		~PacketSession();
		virtual c_int32 OnRecv(BYTE* const buffer, c_int32 len) override sealed;
	protected:
		virtual void OnConnected() abstract;
		virtual void OnSend(c_int32 len) {}
		virtual void OnDisconnected()abstract;
	private:
		const PacketHandleFunc* __restrict m_sessionPacketHandler;
	};
}
