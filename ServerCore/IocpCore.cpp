#include "ServerCorePch.h"
#include "IocpCore.h"
#include "IocpObject.h"
#include "IocpEvent.h"

namespace ServerCore
{
	IocpCore::IocpCore(const HANDLE iocp_handle)noexcept
		:m_iocpHandle{ iocp_handle }
	{
		NAGOX_ASSERT(INVALID_HANDLE_VALUE != m_iocpHandle);
		g_iocpHandle = m_iocpHandle;
	}

	IocpCore::~IocpCore()
	{
		::CloseHandle(m_iocpHandle);
	}

	void IocpCore::Dispatch(const HANDLE iocpHandle_, c_uint32 timeOutMs) noexcept
	{
		constinit extern thread_local uint64_t LEndTickCount;
		constinit extern thread_local uint64_t LCurHandleSessionID;

		DWORD numOfBytes = 0;
		IocpEvent* iocpEvent = nullptr;
		uint64_t CurHandleSessionID = 0;

		const BOOL bResult = ::GetQueuedCompletionStatus(iocpHandle_, OUT & numOfBytes, OUT reinterpret_cast<PULONG_PTR>(&CurHandleSessionID), OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeOutMs);

		const int32_t errCode = ::WSAGetLastError();
		LCurHandleSessionID = CurHandleSessionID;
		LEndTickCount = ::GetTickCount64() + 64;

		if (iocpEvent)
		{
			iocpEvent -= 1;
			if (IocpObject* const iocpObject = iocpEvent->GetIocpObject()) [[likely]]
			{
				iocpObject->Dispatch(iocpEvent, numOfBytes);
			}
		}
		else
		{
			if (const auto global_task = reinterpret_cast<Task* const>(CurHandleSessionID)) [[likely]]
			{
				global_task->ExecuteTask();
				xdelete<Task>(global_task);
			}
		}

		if (FALSE == bResult)
		{
			switch (errCode)
			{
			case WAIT_TIMEOUT:
				return;
			default:
				// TODO 왜 여기로 왔는지 로그 찍기
				break;
			}
		}
	}
}