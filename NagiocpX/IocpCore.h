#pragma once

namespace NagiocpX
{
	class IocpObject;
	/*--------------
		IocpCore
	---------------*/

	class IocpCore
	{
	public:
		IocpCore(const HANDLE iocp_handle)noexcept;
		~IocpCore()noexcept;

		inline const HANDLE GetIocpHandle()const noexcept { return m_iocpHandle; }

		template <typename T> requires std::derived_from<std::decay_t<T>,IocpObject>
		const bool RegisterIOCP(const T* const handleObject_, const uint64 iocpKey_ = 0)const noexcept{
			if constexpr (std::derived_from<std::decay_t<T>, Session>)
				return ::CreateIoCompletionPort((HANDLE)handleObject_->GetSocket(), m_iocpHandle, (ULONG_PTR)(iocpKey_), 0);

			else
				return ::CreateIoCompletionPort(handleObject_->GetHandle(), m_iocpHandle, (ULONG_PTR)(iocpKey_), 0);
		}
		static const bool Dispatch(const HANDLE iocpHandle_)noexcept;
		static inline const bool IsTimeOut(const uint64_t endTick)noexcept { return ::GetTickCount64() >= endTick; }
	private:
		const HANDLE m_iocpHandle;

		static constexpr const DWORD IOCP_POOL_TIME_OUT_MS = 10;

		enum { WORKER_TICK = 24 };
	};
}