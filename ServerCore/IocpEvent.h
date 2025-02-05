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

	class IocpEvent
	{
	protected:
		IocpEvent(const uint8_t eType_)noexcept
			:m_combined_ptr{ static_cast<uint64_t>(eType_) << 56 }
		{}
	public:
		template <typename Enum> requires std::is_enum_v<Enum> && (sizeof(uint8_t) == sizeof(Enum))
		IocpEvent(const Enum eType_)noexcept
			:IocpEvent{ static_cast<uint8_t>(eType_) }
		{}
		
		template <typename Enum> requires std::is_enum_v<Enum> && (sizeof(uint8_t) == sizeof(Enum))
		IocpEvent(const Enum eType_, S_ptr<IocpObject> pObj)noexcept
			:m_combined_ptr{ (static_cast<uint64_t>(eType_) << 56) |
			(reinterpret_cast<uint64_t>(std::exchange(pObj.m_count_ptr,nullptr)) & 0x00FFFFFFFFFFFFFF) }
		{}
		constexpr ~IocpEvent()noexcept { ReleaseIocpObject(); }

		IocpEvent(const IocpEvent&) = delete;
		IocpEvent& operator=(const IocpEvent&) = delete;
		IocpEvent(IocpEvent&& other)noexcept 
			:m_combined_ptr{ std::exchange(other.m_combined_ptr,0) }
		{}
	public:
		const IocpObject* GetIocpObject()const noexcept { return GetPtr(); }
		IocpObject* GetIocpObject()noexcept { return GetPtr(); }
		
		template<typename T> requires (sizeof(uint8_t) == sizeof(T))
		inline const T GetEventType()const noexcept { return static_cast<T>(GetType()); }

		void SetIocpObject(const S_ptr<IocpObject>& pIocp_)noexcept { 
			const auto temp_ptr = pIocp_.get();
			SetPtr(temp_ptr); temp_ptr->IncRef();
		}
		void SetIocpObject(S_ptr<IocpObject>&& pIocp_)noexcept { SetPtr(pIocp_.get()); pIocp_.release(); }

		[[nodiscard]] __forceinline S_ptr<IocpObject> PassIocpObject()noexcept {
			const auto temp_ptr = GetPtr();
			m_combined_ptr &= 0xFF00000000000000;
			return S_ptr<IocpObject>{reinterpret_cast<const uint64_t>(temp_ptr)};
		}

		template<typename T = IocpObject>
		constexpr void ReleaseIocpObject()noexcept {
			if (const auto ptr = GetPtr())
			{
				ptr->DecRef<T>();
				SetPtr(nullptr);
			}
		}
		const bool IsValid()const noexcept { return nullptr != GetPtr(); }
		operator bool()const noexcept { return IsValid(); }
	protected:
		const uint8_t GetType() const noexcept { return (m_combined_ptr >> 56); }
	private:
		IocpObject* GetPtr() const noexcept { return reinterpret_cast<IocpObject*>(m_combined_ptr & 0x00FFFFFFFFFFFFFF); }
		void SetPtr(void* const ptr_)noexcept { m_combined_ptr = (m_combined_ptr & 0xFF00000000000000) | reinterpret_cast<uint64_t>(ptr_); }
	private:
		uint64_t m_combined_ptr;
	};

	class IOEvent
		:public IocpEvent
	{
	public:
		IOEvent(const EVENT_TYPE eType_)noexcept
			:IocpEvent{ eType_ }
		{}
		IOEvent(const EVENT_TYPE eType_, S_ptr<IocpObject>&& pObj)noexcept
			:IocpEvent{ eType_,std::move(pObj) }
		{}
	public:
		OVERLAPPED* const Init()noexcept { return (OVERLAPPED*)::memset(&m_overLapped, 0, sizeof(OVERLAPPED)); }
	private:
		OVERLAPPED m_overLapped;
	};

	class IocpCompEvent
		:public IocpEvent
	{
	public:
		template <typename CompType> 
			requires std::is_enum_v<CompType> && (sizeof(uint8_t) == sizeof(CompType)) && (!std::is_same_v<CompType,EVENT_TYPE>)
		IocpCompEvent(const CompType eType_)noexcept
			:IocpEvent{ eType_ }
		{}
		template <typename CompType, typename T> 
			requires (std::is_enum_v<CompType> && (sizeof(uint8_t) == sizeof(CompType)))
			&& (std::is_same_v<T,class ContentsEntity> || std::derived_from<T,class TaskQueueable>)
			&& (!std::is_same_v<CompType, EVENT_TYPE>)
		IocpCompEvent(const CompType eType_, S_ptr<T> pObj)noexcept
			:IocpEvent{ eType_,std::move(pObj) }
		{}
	public:
		void ReleaseIocpObject()noexcept { IocpEvent::ReleaseIocpObject<ContentsEntity>(); }
	};

	/*--------------
		ConnectEvent
	---------------*/

	class ConnectEvent
		:public IOEvent
	{
	public:
		ConnectEvent()noexcept :IOEvent{ EVENT_TYPE::CONNECT } {}
		~ConnectEvent()noexcept = default;
	};

	/*--------------
		DisconnectEvent
	---------------*/

	class DisconnectEvent
		:public IOEvent
	{
	public:
		DisconnectEvent()noexcept :IOEvent{ EVENT_TYPE::DISCONNECT } {}
		~DisconnectEvent()noexcept = default;
	};


	/*--------------
		AcceptEvent
	---------------*/

	class AcceptEvent
		:public IOEvent
	{
	public:
		AcceptEvent()noexcept :IOEvent{ EVENT_TYPE::ACCEPT } {}
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

	class RecvEvent
		:public IOEvent
	{
	public:
		RecvEvent()noexcept :IOEvent{ EVENT_TYPE::RECV } {}
		~RecvEvent()noexcept = default;
	};

	/*--------------
		SendEvent
	---------------*/

	class SendEvent
		:public IOEvent
	{
	public:
		SendEvent(const SOCKET sock)noexcept
			: IOEvent{ EVENT_TYPE::SEND }
			, m_sendSocket{ sock }
		{}
		XVector<S_ptr<SendBuffer>> m_sendBuffer;
		SOCKET m_sendSocket = INVALID_SOCKET;
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