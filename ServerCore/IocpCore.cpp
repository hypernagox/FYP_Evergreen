#include "ServerCorePch.h"
#include "IocpCore.h"
#include "IocpObject.h"
#include "IocpEvent.h"

namespace ServerCore
{
	constinit extern thread_local uint64_t LEndTickCount;

	IocpCore::IocpCore(const HANDLE iocp_handle)noexcept
		:m_iocpHandle{ iocp_handle }
	{
		NAGOX_ASSERT(INVALID_HANDLE_VALUE != m_iocpHandle);
	}

	IocpCore::~IocpCore()
	{
		::CloseHandle(m_iocpHandle);
	}

	const bool IocpCore::Dispatch(const HANDLE iocpHandle_) noexcept
	{
		constinit extern thread_local uint64_t LEndTickCount;
		
		DWORD numOfBytes = 0;
		IocpEvent* iocpEvent = nullptr;
		Session* curHandleSession = nullptr;

		const BOOL bResult =
			::GetQueuedCompletionStatus(iocpHandle_, OUT & numOfBytes, OUT reinterpret_cast<PULONG_PTR>(&curHandleSession), OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), IocpCore::IOCP_POOL_TIME_OUT_MS);

		LEndTickCount = ::GetTickCount64() + IocpCore::WORKER_TICK;

		if (iocpEvent)
		{
			iocpEvent -= 1;
			if (!bResult)PrintLogEndl(std::format("IOCP Error: {}", ::WSAGetLastError()));
			if (curHandleSession)curHandleSession->Session::Dispatch(iocpEvent, numOfBytes);
			else if (IocpObject* const iocpObject = iocpEvent->GetIocpObject())iocpObject->Dispatch(iocpEvent, numOfBytes);
			else [[unlikely]] PrintLogEndl(std::format("!! {}, {} !!", (int)iocpEvent->GetEventType<uint8_t>(), numOfBytes));
			return true;
		}
		else if(bResult)
		{
			if (const auto global_task = reinterpret_cast<Task* const>(curHandleSession)) [[likely]]
			{
				global_task->ExecuteTask();
				xdelete<Task>(global_task);
				return true;
			}
			else [[unlikely]]
			{
				PrintLogEndl("Thread EXIT");
				return false;
			}
		}
		else
		{
			return true;
		}

		return true;
	}
}