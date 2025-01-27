#pragma once
#include "Singleton.hpp"
#include "IocpCore.h"

namespace ServerCore
{
	class IocpCore;

	class CoreGlobal
		:public Singleton<CoreGlobal>
	{
		friend class Singleton;
		CoreGlobal();
		~CoreGlobal();
	public:
		const IocpCore& GetIocpCore()const noexcept { return m_iocpCore; }
		void Init()const noexcept;
	private:
		void ExitRoutine()noexcept;
	private:
		const IocpCore m_iocpCore;
#ifdef _DEBUG
		_CrtMemState mem_start;
		_CrtMemState mem_end;
#endif
	};
}