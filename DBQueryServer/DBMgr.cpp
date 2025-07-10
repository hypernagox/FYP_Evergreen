#include "pch.h"
#include "DBMgr.h"
#include "DBConnectionHandle.h"


DBMgr::DBMgr()
{
}

DBMgr::~DBMgr()
{
	Clear();
}

void DBMgr::Init() noexcept
{
}

bool DBMgr::Connect(c_int32 connectionCount, const std::wstring_view connectionString)
{
	if (::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_environment) != SQL_SUCCESS)
		return false;

	if (::SQLSetEnvAttr(m_environment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0) != SQL_SUCCESS)
		return false;
	for (int32 i = 0; i < connectionCount; ++i)
	{
		DBConnectionHandle* const connection = new DBConnectionHandle();
		if (connection->Connect(m_environment, connectionString) == false)
			return false;

		m_conQueue.push(connection);
	}
	std::cout << "DB 연결 성공 !\n";
	return true;
}

void DBMgr::Clear()
{
	if (m_environment != SQL_NULL_HANDLE)
	{
		::SQLFreeHandle(SQL_HANDLE_ENV, m_environment);
		m_environment = SQL_NULL_HANDLE;
	}
	const DBConnectionHandle* dbCon = nullptr;
	while (m_conQueue.try_pop(dbCon)) 
	{
		delete dbCon;
	}
}

const DBConnectionHandle* const DBMgr::Pop() noexcept
{
	const DBConnectionHandle* connection;
	NAGOX_ASSERT(m_conQueue.try_pop(connection));
	return connection;
}

void DBMgr::Push(const DBConnectionHandle* const connection) noexcept
{
	m_conQueue.push(connection);
}
