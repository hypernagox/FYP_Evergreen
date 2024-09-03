#pragma once
#include "ClientNetworkPch.h"

namespace NetHelper
{
	class Session;

	class NetEvent
		:public OVERLAPPED
	{
	public:
		NetEvent(EVENT_TYPE eType_);
		~NetEvent();
	public:
		void Init()noexcept { ::memset((OVERLAPPED*)this, 0, sizeof(OVERLAPPED)); }
		EVENT_TYPE GetEventType()const noexcept{ return m_eEVENT_TYPE; }
		void SetSession(S_ptr<Session> pIocp_)noexcept { m_pSession.swap(pIocp_); }
		const S_ptr<Session>& GetSession()const noexcept { return m_pSession; }
		void ReleaseSession()noexcept { m_pSession.reset(); }

		template<typename T> requires std::derived_from<T, NetEvent>
		T* const Cast()noexcept;
		template<typename T> requires std::derived_from<T, NetEvent>
		const T* const Cast()const noexcept;
	protected:
		const EVENT_TYPE m_eEVENT_TYPE;
	private:
		S_ptr<Session> m_pSession = nullptr;
	};

	template<typename T> requires std::derived_from<T, NetEvent>
	inline T* const NetEvent::Cast()noexcept { return static_cast<T* const>(this); }

	template<typename T> requires std::derived_from<T, NetEvent>
	inline const T* const NetEvent::Cast() const noexcept { return static_cast<const T* const>(this); }

	/*--------------
		ConnectEvent
	---------------*/

	class ConnectEvent
		:public NetEvent
	{
	public:
		ConnectEvent() :NetEvent{ EVENT_TYPE::CONNECT } {}
		~ConnectEvent();
	};

	/*--------------
		DisconnectEvent
	---------------*/

	class DisconnectEvent
		:public NetEvent
	{
	public:
		DisconnectEvent() :NetEvent{ EVENT_TYPE::DISCONNECT } {}
		~DisconnectEvent();
	};

	/*--------------
		RecvEvent
	---------------*/

	class RecvEvent
		:public NetEvent
	{
	public:
		RecvEvent() :NetEvent{ EVENT_TYPE::RECV } {}
		~RecvEvent();
	};

	/*--------------
		SendEvent
	---------------*/

	class SendEvent
		:public NetEvent
	{
	public:
		SendEvent() :NetEvent{ EVENT_TYPE::SEND } {}
		~SendEvent();
		std::vector<S_ptr<class SendBuffer>> sendBuffer;
	};
}