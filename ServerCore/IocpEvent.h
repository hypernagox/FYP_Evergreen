#pragma once

namespace ServerCore
{
	class Session;
	class IocpObject;
	class Session;
	class SendBuffer;
	class Task;

	/*--------------
		IocpEvent
	---------------*/

	// 가상함수 절대금지 (가상함수테이블 포인터로인한 구조체멤버변수 변화)
	class IocpEvent
		:public OVERLAPPED
	{
	public:
		IocpEvent(const EVENT_TYPE eType_)noexcept : m_eEVENT_TYPE{ eType_ } {}
		IocpEvent(const EVENT_TYPE eType_, S_ptr<IocpObject> pObj)noexcept
			: m_eEVENT_TYPE{ eType_ }
			, m_pIocpObject{ std::move(pObj) }
		{}
		~IocpEvent()noexcept = default;
	public:
		inline void Init()noexcept { ::memset(static_cast<OVERLAPPED* const>(this), NULL, sizeof(OVERLAPPED)); }
		constexpr inline const EVENT_TYPE GetEventType()const noexcept { return m_eEVENT_TYPE; }
		template <typename T> //requires //std::convertible_to<T,S_ptr<IocpObject>>
		constexpr inline void SetIocpObject(T&& pIocp_)noexcept { std::construct_at(&m_pIocpObject, std::forward<T>(pIocp_)); }
		const S_ptr<IocpObject>& GetIocpObject()const noexcept { return m_pIocpObject; }
		void ReleaseIocpObject()noexcept { m_pIocpObject.reset(); }
		[[nodiscard]] constexpr __forceinline S_ptr<IocpObject>&& PassIocpObject()noexcept { return std::move(m_pIocpObject); }
		template<typename T> requires std::derived_from<T, IocpEvent>
		constexpr inline T* const Cast()noexcept { return static_cast<T* const>(this); }
		template<typename T> requires std::derived_from<T, IocpEvent>
		constexpr inline const T* const Cast()const noexcept { return static_cast<const T* const>(this); }
	private:
		S_ptr<IocpObject> m_pIocpObject = {};
		const EVENT_TYPE m_eEVENT_TYPE;
	};

	/*--------------
		ConnectEvent
	---------------*/

	class ConnectEvent
		:public IocpEvent
	{
	public:
		ConnectEvent()noexcept :IocpEvent{ EVENT_TYPE::CONNECT } {}
		~ConnectEvent()noexcept = default;
	};

	/*--------------
		DisconnectEvent
	---------------*/

	class DisconnectEvent
		:public IocpEvent
	{
	public:
		DisconnectEvent()noexcept :IocpEvent{ EVENT_TYPE::DISCONNECT } {}
		~DisconnectEvent()noexcept = default;
	};


	/*--------------
		AcceptEvent
	---------------*/

	class AcceptEvent
		:public IocpEvent
	{
	public:
		AcceptEvent()noexcept :IocpEvent{ EVENT_TYPE::ACCEPT } {}
		~AcceptEvent()noexcept = default;
		const S_ptr<Session>& RegisterSession(S_ptr<Session>&& pSession_)noexcept { m_pSession.swap(pSession_); return m_pSession; }
		S_ptr<Session> ReleaseSession()noexcept { return S_ptr<Session>{std::move(m_pSession)}; }
		[[nodiscard]] constexpr __forceinline S_ptr<Session>&& PassSession()noexcept { return std::move(m_pSession); }
	private:
		S_ptr<Session> m_pSession;
	};

	/*--------------
		RecvEvent
	---------------*/

	class alignas(64) RecvEvent
		:public IocpEvent
	{
	public:
		RecvEvent()noexcept :IocpEvent{ EVENT_TYPE::RECV } {}
		~RecvEvent()noexcept = default;
	};

	/*--------------
		SendEvent
	---------------*/

	class alignas(64) SendEvent
		:public IocpEvent
	{
	public:
		SendEvent()noexcept :IocpEvent{ EVENT_TYPE::SEND }, m_registerSendEvent{ xnew<IocpEvent>(EVENT_TYPE::REGISTER_SEND) } {}
		~SendEvent()noexcept { xdelete_sized<IocpEvent>(m_registerSendEvent, sizeof(IocpEvent)); }
		Vector<S_ptr<SendBuffer>> m_sendBuffer;
		alignas(64) IocpEvent* const m_registerSendEvent;
	};

	class ContentsEntityTask
		:public IocpEvent
	{
		friend class ContentsEntity;
	public:
		ContentsEntityTask(S_ptr<IocpObject> pContentsEntity,Task&& task_)noexcept
			: IocpEvent{ EVENT_TYPE::CONTENTS_ENTITY_TASK,std::move(pContentsEntity) }
			, m_contents_entity_task{ std::move(task_) }
		{}
		~ContentsEntityTask()noexcept = default;
	private:
		const Task m_contents_entity_task;
	};
}