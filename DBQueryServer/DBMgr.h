#pragma once
#include "pch.h"

class DBConnectionHandle;

class DBMgr
	:public Singleton<DBMgr>
{
	friend class Singleton;
	DBMgr();
	~DBMgr();
public:
	void Init()noexcept override;
	bool Connect(c_int32 connectionCount, const std::wstring_view connectionString);
	void Clear();
	const DBConnectionHandle* const Pop()noexcept;
	void Push(const DBConnectionHandle* const connection)noexcept;
private:
	concurrency::concurrent_queue<const DBConnectionHandle*> m_conQueue;
	SQLHENV	m_environment = SQL_NULL_HANDLE;
};