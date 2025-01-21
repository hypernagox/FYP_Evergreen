#pragma once

namespace ServerCore
{
	class IocpObject;

	/*--------------
		IocpCore
	---------------*/


	class IocpCore
	{
	public:
		constexpr static inline const HANDLE GetIocpHandleGlobal()noexcept { return g_iocpHandle; }
	public:
		IocpCore(const HANDLE iocp_handle)noexcept;
		~IocpCore();

		HANDLE GetIocpHandle()const noexcept { return m_iocpHandle; }

		template <typename T> requires std::derived_from<std::decay_t<T>,IocpObject>
		const bool RegisterIOCP(const T* const handleObject_, const uint64 iocpKey_ = 0)const noexcept{
			if constexpr (std::derived_from<std::decay_t<T>, Session>)
				return ::CreateIoCompletionPort((HANDLE)handleObject_->GetSocket(), m_iocpHandle, (ULONG_PTR)(iocpKey_), 0);

			else
				return ::CreateIoCompletionPort(handleObject_->GetHandle(), m_iocpHandle, (ULONG_PTR)(iocpKey_), 0);
		}
		static void Dispatch(const HANDLE iocpHandle_)noexcept; // gqcs로 일감을 빼내서 일을 처리하는 스레드함수
	private:
		const HANDLE m_iocpHandle;

		constinit static inline HANDLE g_iocpHandle;

		static constexpr const DWORD IOCP_POOL_TIME_OUT_MS = 10;
	};
}