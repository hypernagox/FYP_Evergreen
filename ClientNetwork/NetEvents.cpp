#include "ClientNetworkPch.h"
#include "NetEvents.h"

namespace NetHelper
{
	NetEvent::NetEvent(EVENT_TYPE eType_)
		:m_eEVENT_TYPE{ eType_ }
	{
		Init();
	}

	NetEvent::~NetEvent()
	{
	}

	ConnectEvent::~ConnectEvent()
	{
		ReleaseSession();
	}

	DisconnectEvent::~DisconnectEvent()
	{
		ReleaseSession();
	}

	RecvEvent::~RecvEvent()
	{
		ReleaseSession();
	}

	SendEvent::~SendEvent()
	{
		ReleaseSession();
	}
}

