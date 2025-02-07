#pragma once
#include "IocpObject.h"

namespace NagiocpX
{
	class DBEvent
		:public NagiocpX::IocpObject
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
		virtual void Dispatch(NagiocpX::IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept = 0;
	public:
		void SetEventPtr()noexcept { m_dbEvent.SetIocpObject(SharedFromThis<DBEvent>()); }
	protected:
		bool m_bSuccess = false;
		NagiocpX::IocpEvent m_dbEvent{ NagiocpX::EVENT_TYPE::DB };
	};
}