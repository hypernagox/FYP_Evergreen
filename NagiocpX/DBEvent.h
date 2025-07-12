#pragma once
#include "IocpObject.h"

namespace NagiocpX
{
	class DBEvent
		:public NagiocpX::IocpObject
	{
		friend class DBMgr;
	public:
		DBEvent(S_ptr<Session> session)noexcept :m_pSession{ std::move(session) } {}
		virtual ~DBEvent()noexcept = default;
		DBEvent(DBEvent&& other)noexcept
			:m_pSession{ std::move(other.m_pSession) }
		{}
	public:
		void UnBind()noexcept;
		virtual void ExecuteQuery()noexcept = 0;
		virtual void Dispatch(NagiocpX::IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept = 0;
	public:
		void SetEventPtr()noexcept { m_dbEvent.SetIocpObject(SharedFromThis<DBEvent>()); }
		template<typename T = class ClientSession>
		inline const auto GetClientSession()const noexcept { return static_cast<T* const>(m_pSession.get()); }
	protected:
		bool m_bSuccess = false;
		S_ptr<Session> m_pSession;
		NagiocpX::IocpEvent m_dbEvent{ NagiocpX::EVENT_TYPE::DB };
	};
}