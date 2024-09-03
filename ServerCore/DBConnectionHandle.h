#pragma once
#include "ServerCorePch.h"
#include <sql.h>
#include <sqlext.h>

namespace ServerCore
{
	enum
	{
		WVARCHAR_MAX = 4000,
		BINARY_MAX = 8000,
		VARCHAR_MAX = 8000,
	};

	class DBConnectionHandle
	{
		friend class DBMgr;
	public:
		const bool Execute(const std::wstring_view query)const noexcept;
		const bool Fetch()const noexcept;
		const int32 GetRowCount()const noexcept;
		void Unbind()const noexcept;
	private:
		const bool Connect(const SQLHENV henv, const std::wstring_view connectionString)noexcept;
		void Clear();
	public:
		const bool BindParam(c_int32 paramIndex, bool* const value, SQLLEN* const index)const noexcept;
		const bool BindParam(c_int32 paramIndex, float* const value, SQLLEN* const index)const noexcept;
		const bool BindParam(c_int32 paramIndex, double* const value, SQLLEN* const index)const noexcept;
		const bool BindParam(c_int32 paramIndex, int8* const value, SQLLEN* const index)const noexcept;
		const bool BindParam(c_int32 paramIndex, int16* const value, SQLLEN* const index)const noexcept;
		const bool BindParam(c_int32 paramIndex, int32* const value, SQLLEN* const index)const noexcept;
		const bool BindParam(c_int32 paramIndex, int64* const value, SQLLEN* const index)const noexcept;
		const bool BindParam(c_int32 paramIndex, TIMESTAMP_STRUCT* const value, SQLLEN* const index)const noexcept;
		const bool BindParam(c_int32 paramIndex, const std::wstring_view str, SQLLEN* index)const noexcept;
		const bool BindParam(c_int32 paramIndex, const std::string_view str, SQLLEN* index)const noexcept;
		const bool BindParam(c_int32 paramIndex, std::byte* const bin, int32 size, SQLLEN* const index)const noexcept;
			 
		const bool BindCol(c_int32 columnIndex, bool* const value, SQLLEN* const index)const noexcept;
		const bool BindCol(c_int32 columnIndex, float* const value, SQLLEN* const index)const noexcept;
		const bool BindCol(c_int32 columnIndex, double* const value, SQLLEN* const index)const noexcept;
		const bool BindCol(c_int32 columnIndex, int8* const value, SQLLEN* const index)const noexcept;
		const bool BindCol(c_int32 columnIndex, int16* const value, SQLLEN* const index)const noexcept;
		const bool BindCol(c_int32 columnIndex, int32* const value, SQLLEN* const index)const noexcept;
		const bool BindCol(c_int32 columnIndex, int64* const value, SQLLEN* const index)const noexcept;
		const bool BindCol(c_int32 columnIndex, TIMESTAMP_STRUCT* const value, SQLLEN* const index)const noexcept;
		const bool BindCol(c_int32 columnIndex, WCHAR* const str, c_int32 size, SQLLEN* const index)const noexcept;
		const bool BindCol(c_int32 columnIndex, char* const str, c_int32 size, SQLLEN* const index)const noexcept;
		const bool BindCol(c_int32 columnIndex, std::byte* const bin, c_int32 size, SQLLEN* const index)const noexcept;
	private:
		const bool BindParam(const SQLUSMALLINT paramIndex, const SQLSMALLINT cType, const SQLSMALLINT sqlType, const SQLULEN len, const SQLPOINTER ptr, SQLLEN* const index)const noexcept;
		const bool BindCol(const SQLUSMALLINT columnIndex, const SQLSMALLINT cType, const SQLULEN len, const SQLPOINTER value, SQLLEN* const index)const noexcept;
		void HandleError(const SQLRETURN ret)const noexcept;
	private:
		SQLHDBC	 m_connection = SQL_NULL_HANDLE;
		SQLHSTMT m_statement = SQL_NULL_HANDLE;
	};
}
