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

		template <typename T> // Ŭ�� ����� iocp�� ���
		const bool RegisterIOCP(const T* const handleObject_, const uint64 iocpKey_ = 0)const noexcept{
			return ::CreateIoCompletionPort(handleObject_->GetHandle(), m_iocpHandle, (ULONG_PTR)(iocpKey_), 0);
		}
		static void Dispatch(const HANDLE iocpHandle_, c_uint32 timeOutMs = INFINITE)noexcept; // gqcs�� �ϰ��� ������ ���� ó���ϴ� �������Լ�
	private:
		const HANDLE m_iocpHandle;

		constinit static inline HANDLE g_iocpHandle;
	};
}