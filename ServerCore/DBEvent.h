#pragma once
#include "IocpObject.h"

namespace ServerCore
{
	class DBEvent
		:public ServerCore::IocpObject
	{
		friend class DBMgr;
	public:
		DBEvent()noexcept = default;
		virtual ~DBEvent()noexcept = default;
		DBEvent(DBEvent&& other)noexcept
			:m_dbEvent{ std::move(other.m_dbEvent) }
		{}
	public:
		virtual void BindData()noexcept {}
		virtual void ExecuteQuery()noexcept = 0;
		virtual void Dispatch(ServerCore::IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept = 0;
	public:
		void SetEventPtr()noexcept { m_dbEvent.SetIocpObject(SharedFromThis<DBEvent>()); }
	protected:
		bool m_bSuccess = false;
		ServerCore::IocpEvent m_dbEvent{ ServerCore::EVENT_TYPE::DB };
	};
}