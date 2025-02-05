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
		void Init()noexcept;
	private:
		static void ExitRoutine()noexcept;
	private:
		const IocpCore m_iocpCore;
	};
}