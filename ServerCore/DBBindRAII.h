#pragma once
#include "ServerCorePch.h"
#include "DBConnectionHandle.h"
#include "DBMgr.h"

namespace ServerCore
{
	template<c_int32 C>
	struct FullBitsChecker { enum { value = (1 << (C - 1)) | FullBitsChecker<C - 1>::value }; };

	template<>
	struct FullBitsChecker<1> { enum { value = 1 }; };

	template<>
	struct FullBitsChecker<0> { enum { value = 0 }; };

	template<c_int32 ParamCount, c_int32 ColumnCount>
	class DBBindRAII
	{
	public:
		DBBindRAII(const std::wstring_view query)noexcept
			: m_dbConnection{ Mgr(DBMgr)->GetDBHandle() }
			, m_query{ query }
		{}

		DBBindRAII(DBBindRAII&& other)noexcept
			: m_dbConnection{ other.m_dbConnection }
			, m_query{ std::move(other.m_query) }
			, m_paramFlag{ 0 }
			, m_columnFlag{ 0 }
			, m_paramIndex{ 0 }
			, m_columnIndex{ 0 }
		{}

		void UnBind()noexcept { 
			if (nullptr == m_dbConnection)return;
			m_dbConnection->Unbind();
			m_dbConnection = nullptr;
		}
		~DBBindRAII()noexcept {
			UnBind();
		}

		const bool Validate()const noexcept
		{
			return m_paramFlag == FullBitsChecker<ParamCount>::value && m_columnFlag == FullBitsChecker<ColumnCount>::value;
		}

		const bool Execute()const noexcept
		{
			const bool bRes = Validate() && m_dbConnection->Execute(m_query);
			//NAGOX_ASSERT(bRes);
			return bRes;
		}

		const bool Fetch()const noexcept
		{
			return m_dbConnection->Fetch();
		}

	public:
		template<typename T>
		void BindParam(const int32 idx, T& value)noexcept
		{
			m_dbConnection->BindParam(idx + 1, &value, &m_paramIndex[idx]);
			m_paramFlag |= (1LL << idx);
		}

		void BindParam(const int32 idx, std::string& value)noexcept
		{
			m_dbConnection->BindParam(idx + 1, value.data(), &m_paramIndex[idx]);
			m_paramFlag |= (1LL << idx);
		}

		void BindParam(const int32 idx, std::wstring& value)noexcept
		{
			m_dbConnection->BindParam(idx + 1, value.data(), &m_paramIndex[idx]);
			m_paramFlag |= (1LL << idx);
		}

		void BindParam(const int32 idx, const WCHAR* const value)noexcept
		{
			m_dbConnection->BindParam(idx + 1, value, &m_paramIndex[idx]);
			m_paramFlag |= (1LL << idx);
		}

		template<typename T, c_int32 N>
		void BindParam(const int32 idx, T(&value)[N])noexcept
		{
			m_dbConnection->BindParam(idx + 1, (std::byte*const)value, size32(T) * N, &m_paramIndex[idx]);
			m_paramFlag |= (1LL << idx);
		}

		template<typename T>
		void BindParam(const int32 idx, T* const value, const int32 N)noexcept
		{
			m_dbConnection->BindParam(idx + 1, (std::byte*const)value, size32(T) * N, &m_paramIndex[idx]);
			m_paramFlag |= (1LL << idx);
		}

		template<typename T>
		void BindCol(const int32 idx, T& value)noexcept
		{
			m_dbConnection->BindCol(idx + 1, &value, &m_columnIndex[idx]);
			m_columnFlag |= (1LL << idx);
		}

		template<c_int32 N>
		void BindCol(const int32 idx, WCHAR(&value)[N])noexcept
		{
			m_dbConnection->BindCol(idx + 1, value, N - 1, &m_columnIndex[idx]);
			m_columnFlag |= (1LL << idx);
		}

		void BindCol(const int32 idx, WCHAR* const value, const int32 len)noexcept
		{
			m_dbConnection->BindCol(idx + 1, value, len - 1, &m_columnIndex[idx]);
			m_columnFlag |= (1LL << idx);
		}

		void BindCol(const int32 idx, CHAR* const value, const int32 len)noexcept
		{
			m_dbConnection->BindCol(idx + 1, value, len - 1, &m_columnIndex[idx]);
			m_columnFlag |= (1LL << idx);
		}

		void BindCol(const int32 idx, std::string& value)noexcept
		{
			m_dbConnection->BindCol(idx + 1, value.data(), (c_int32)value.size(), &m_columnIndex[idx]);
			m_columnFlag |= (1LL << idx);
		}

		void BindCol(const int32 idx, std::wstring& value)noexcept
		{
			m_dbConnection->BindCol(idx + 1, value.data(), (c_int32)value.size(), &m_columnIndex[idx]);
			m_columnFlag |= (1LL << idx);
		}

		template<typename T, c_int32 N>
		void BindCol(const int32 idx, T(&value)[N])noexcept
		{
			m_dbConnection->BindCol(idx + 1, value, size32(T) * N, &m_columnIndex[idx]);
			m_columnFlag |= (1LL << idx);
		}

	private:
		const DBConnectionHandle* m_dbConnection;
		WString m_query;
		SQLLEN m_paramIndex[ParamCount > 0 ? ParamCount : 1];
		SQLLEN m_columnIndex[ColumnCount > 0 ? ColumnCount : 1];
		uint64 m_paramFlag;
		uint64 m_columnFlag;
	};
}

