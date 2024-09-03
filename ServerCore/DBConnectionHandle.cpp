#include "ServerCorePch.h"
#include "DBConnectionHandle.h"

namespace ServerCore
{
	const bool DBConnectionHandle::Connect(const SQLHENV henv, const std::wstring_view connectionString)noexcept
	{
		if (::SQLAllocHandle(SQL_HANDLE_DBC, henv, & m_connection) != SQL_SUCCESS)
			return false;

		WCHAR stringBuffer[MAX_PATH] = { 0 };
		::wcscpy_s(stringBuffer, connectionString.data());

		WCHAR resultString[MAX_PATH] = { 0 };
		SQLSMALLINT resultStringLen = 0;

		const SQLRETURN ret = ::SQLDriverConnectW(
			m_connection,
			NULL,
			reinterpret_cast<SQLWCHAR*const>(stringBuffer),
			_countof(stringBuffer),
			OUT reinterpret_cast<SQLWCHAR*const>(resultString),
			_countof(resultString),
			OUT & resultStringLen,
			SQL_DRIVER_NOPROMPT
		);

		if (::SQLAllocHandle(SQL_HANDLE_STMT, m_connection, &m_statement) != SQL_SUCCESS)
			return false;

		return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
	}

	void DBConnectionHandle::Clear()
	{
		if (m_connection != SQL_NULL_HANDLE)
		{
			::SQLFreeHandle(SQL_HANDLE_DBC, m_connection);
			m_connection = SQL_NULL_HANDLE;
		}

		if (m_statement != SQL_NULL_HANDLE)
		{
			::SQLFreeHandle(SQL_HANDLE_STMT,m_statement);
			m_statement = SQL_NULL_HANDLE;
		}
	}

	const bool DBConnectionHandle::Execute(const std::wstring_view query)const noexcept
	{
		const SQLRETURN ret = ::SQLExecDirectW(m_statement, (SQLWCHAR*const)query.data(), SQL_NTSL);
		if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
			return true;

		HandleError(ret);
		return false;
	}

	const bool DBConnectionHandle::Fetch()const noexcept
	{
		const SQLRETURN ret = ::SQLFetch(m_statement);

		switch (ret)
		{
		case SQL_SUCCESS:
		case SQL_SUCCESS_WITH_INFO:
			return true;
		case SQL_NO_DATA:
			return false;
		case SQL_ERROR:
			HandleError(ret);
			return false;
		default:
			return true;
		}
	}

	const int32 DBConnectionHandle::GetRowCount()const noexcept
	{
		SQLLEN count = 0;
		const SQLRETURN ret = ::SQLRowCount(m_statement, OUT & count);

		if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
			return static_cast<c_int32>(count);

		return -1;
	}

	void DBConnectionHandle::Unbind()const noexcept
	{
		::SQLFreeStmt(m_statement, SQL_UNBIND);
		::SQLFreeStmt(m_statement, SQL_RESET_PARAMS);
		::SQLFreeStmt(m_statement, SQL_CLOSE);
	}

	const bool DBConnectionHandle::BindParam(c_int32 paramIndex, bool* const value, SQLLEN* const index)const noexcept
	{
		return BindParam(paramIndex, SQL_C_TINYINT, SQL_TINYINT, size32(bool), value, index);
	}
	
	const bool DBConnectionHandle::BindParam(c_int32 paramIndex, float* const value, SQLLEN* const index)const noexcept
	{
		return BindParam(paramIndex, SQL_C_FLOAT, SQL_REAL, 0, value, index);
	}
	
	const bool DBConnectionHandle::BindParam(c_int32 paramIndex, double* const value, SQLLEN* const index)const noexcept
	{
		return BindParam(paramIndex, SQL_C_DOUBLE, SQL_DOUBLE, 0, value, index);
	}
	
	const bool DBConnectionHandle::BindParam(c_int32 paramIndex, int8* const value, SQLLEN* const index)const noexcept
	{
		return BindParam(paramIndex, SQL_C_TINYINT, SQL_TINYINT, size32(int8), value, index);
	}
	
	const bool DBConnectionHandle::BindParam(c_int32 paramIndex, int16* const value, SQLLEN* const index)const noexcept
	{
		return BindParam(paramIndex, SQL_C_SHORT, SQL_SMALLINT, size32(int16), value, index);
	}

	const bool DBConnectionHandle::BindParam(c_int32 paramIndex, int32* const value, SQLLEN* const index)const noexcept
	{
		return BindParam(paramIndex, SQL_C_LONG, SQL_INTEGER, size32(int32), value, index);
	}

	const bool DBConnectionHandle::BindParam(c_int32 paramIndex, int64* const value, SQLLEN* const index)const noexcept
	{
		return BindParam(paramIndex, SQL_C_SBIGINT, SQL_BIGINT, size32(int64), value, index);
	}
	
	const bool DBConnectionHandle::BindParam(c_int32 paramIndex, TIMESTAMP_STRUCT* const value, SQLLEN* const index)const noexcept
	{
		return BindParam(paramIndex, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, size32(TIMESTAMP_STRUCT), value, index);
	}

	const bool DBConnectionHandle::BindParam(c_int32 paramIndex, const std::wstring_view str, SQLLEN* const index)const noexcept
	{
		const SQLULEN size = static_cast<const SQLULEN>((str.size() + 1) * 2);
		*index = SQL_NTSL;

		if (size > WVARCHAR_MAX)
			return BindParam(paramIndex, SQL_C_WCHAR, SQL_WLONGVARCHAR, size, (SQLPOINTER)str.data(), index);
		else
			return BindParam(paramIndex, SQL_C_WCHAR, SQL_WVARCHAR, size, (SQLPOINTER)str.data(), index);
	}

	const bool DBConnectionHandle::BindParam(c_int32 paramIndex, const std::string_view str, SQLLEN* index) const noexcept
	{
		const SQLULEN size = static_cast<const SQLULEN>(str.size() + 1);
		*index = SQL_NTSL;

		if (size > VARCHAR_MAX)
			return BindParam(paramIndex, SQL_C_WCHAR, SQL_WLONGVARCHAR, size, (SQLPOINTER)str.data(), index);
		else
			return BindParam(paramIndex, SQL_C_WCHAR, SQL_WVARCHAR, size, (SQLPOINTER)str.data(), index);
	}

	const bool DBConnectionHandle::BindParam(c_int32 paramIndex, std::byte* const bin, int32 size, SQLLEN* const index)const noexcept
	{
		if (bin == nullptr)
		{
			*index = SQL_NULL_DATA;
			size = 1;
		}
		else
		{
			*index = size;
		}
	
		if (size > BINARY_MAX)
			return BindParam(paramIndex, SQL_C_BINARY, SQL_LONGVARBINARY, size, (BYTE*const)bin, index);
		else
			return BindParam(paramIndex, SQL_C_BINARY, SQL_BINARY, size, (BYTE*const)bin, index);
	}

	const bool DBConnectionHandle::BindCol(c_int32 columnIndex, bool* const value, SQLLEN* const  index)const noexcept
	{
		return BindCol(columnIndex, SQL_C_TINYINT, size32(bool), value, index);
	}
	
	const bool DBConnectionHandle::BindCol(c_int32 columnIndex, float* const value, SQLLEN* const index)const noexcept
	{
		return BindCol(columnIndex, SQL_C_FLOAT, size32(float), value, index);
	}
	
	const bool DBConnectionHandle::BindCol(c_int32 columnIndex, double* const value, SQLLEN* const index)const noexcept
	{
		return BindCol(columnIndex, SQL_C_DOUBLE, size32(double), value, index);
	}
	
	const bool DBConnectionHandle::BindCol(c_int32 columnIndex, int8* const value, SQLLEN* const index)const noexcept
	{
		return BindCol(columnIndex, SQL_C_TINYINT, size32(int8), value, index);
	}
	
	const bool DBConnectionHandle::BindCol(c_int32 columnIndex, int16* const value, SQLLEN* const index)const noexcept
	{
		return BindCol(columnIndex, SQL_C_SHORT, size32(int16), value, index);
	}

	const bool DBConnectionHandle::BindCol(c_int32 columnIndex, int32* const value, SQLLEN* const index)const noexcept
	{
		return BindCol(columnIndex, SQL_C_LONG, size32(int32), value, index);
	}

	const bool DBConnectionHandle::BindCol(c_int32 columnIndex, int64* const value, SQLLEN* const index)const noexcept
	{
		return BindCol(columnIndex, SQL_C_SBIGINT, size32(int64), value, index);
	}

	const bool DBConnectionHandle::BindCol(c_int32 columnIndex, TIMESTAMP_STRUCT* const  value, SQLLEN* const index)const noexcept
	{
		return BindCol(columnIndex, SQL_C_TYPE_TIMESTAMP, size32(TIMESTAMP_STRUCT), value, index);
	}

	const bool DBConnectionHandle::BindCol(c_int32 columnIndex, WCHAR* const str, c_int32 size, SQLLEN* const index)const noexcept
	{
		return BindCol(columnIndex, SQL_C_WCHAR, size, str, index);
	}

	const bool DBConnectionHandle::BindCol(c_int32 columnIndex, char* const str, c_int32 size, SQLLEN* const index) const noexcept
	{
		return BindCol(columnIndex, SQL_C_CHAR, size, str, index);
	}

	const bool DBConnectionHandle::BindCol(c_int32 columnIndex, std::byte* const bin, c_int32 size, SQLLEN* const index)const noexcept
	{
		return BindCol(columnIndex, SQL_BINARY, size, bin, index);
	}

	const bool DBConnectionHandle::BindParam(const SQLUSMALLINT paramIndex, const SQLSMALLINT cType, const SQLSMALLINT sqlType, const SQLULEN len, const SQLPOINTER ptr, SQLLEN* const index)const noexcept
	{
		const SQLRETURN ret = ::SQLBindParameter(m_statement, paramIndex, SQL_PARAM_INPUT, cType, sqlType, len, 0, ptr, 0, index);
		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
		{
			HandleError(ret);
			return false;
		}

		return true;
	}

	const bool DBConnectionHandle::BindCol(const SQLUSMALLINT columnIndex, const SQLSMALLINT cType, const SQLULEN len, const SQLPOINTER value, SQLLEN* const index)const noexcept
	{
		const SQLRETURN ret = ::SQLBindCol(m_statement, columnIndex, cType, value, len, index);
		if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
		{
			HandleError(ret);
			return false;
		}
		return true;
	}

	void DBConnectionHandle::HandleError(const SQLRETURN ret)const noexcept
	{
		if (ret == SQL_SUCCESS)
			return;

		SQLSMALLINT index = 1;
		SQLWCHAR sqlState[MAX_PATH] = { 0 };
		SQLINTEGER nativeErr = 0;
		SQLWCHAR errMsg[MAX_PATH] = { 0 };
		SQLSMALLINT msgLen = 0;
		SQLRETURN errorRet = 0;

		for(;;)
		{
			errorRet = ::SQLGetDiagRecW(
				SQL_HANDLE_STMT,
				m_statement,
				index,
				sqlState,
				OUT & nativeErr,
				errMsg,
				_countof(errMsg),
				OUT & msgLen
			);

			if (errorRet == SQL_NO_DATA)
				break;

			if (errorRet != SQL_SUCCESS && errorRet != SQL_SUCCESS_WITH_INFO)
				break;

			std::wcout.imbue(std::locale("kor"));
			std::wcout << errMsg << std::endl;

			++index;
		}
	}
}