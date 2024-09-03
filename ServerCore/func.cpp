#include "ServerCorePch.h"
#include "func.h"
#include "ThreadMgr.h"
#include "Service.h"
#include "Session.h"
#include "RefCountable.h"

namespace ServerCore
{
	void PrintError(const char* const msg, const int err_no) noexcept
	{
		WCHAR* msg_buf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
			err_no,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPWSTR>(&msg_buf),
			0,
			NULL
		);
		std::cout << msg;
		std::wcout << L": ¿¡·¯ : " << msg_buf;
		while (true);
		LocalFree(msg_buf);
	}

	S_ptr<ContentsEntity> GetSession(const uint64_t sessionID_) noexcept
	{
		return Mgr(ThreadMgr)->GetMainService()->GetSession(sessionID_);
	}

	void SendPacket(const uint64_t target_session_id, S_ptr<SendBuffer> pSendBuffer) noexcept
	{
		if (const auto pSessionEntity = Mgr(ThreadMgr)->GetMainService()->GetSession(target_session_id))
			pSessionEntity->GetSession()->SendAsync(std::move(pSendBuffer));
	}

	void LogStackTrace() noexcept
	{
		const int MaxFrames = 64;
		void* stack[MaxFrames];
		USHORT frames = CaptureStackBackTrace(0, MaxFrames, stack, NULL);

		SymInitialize(GetCurrentProcess(), NULL, TRUE);

		SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
		symbol->MaxNameLen = 255;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

		for (USHORT i = 0; i < frames; i++)
		{
			::SymFromAddr(GetCurrentProcess(), (DWORD64)(stack[i]), 0, symbol);
			std::cout << i << ": " << symbol->Name << " - 0x" << std::hex << symbol->Address << std::dec << std::endl;
		}

		free(symbol);
		SymCleanup(GetCurrentProcess());
	}

	const uint32 GetCurThreadIdx() noexcept
	{
		constinit extern thread_local uint32_t LThreadId;
		return LThreadId - 1;
	}

	S_ptr<SendBuffer> CreateHeartBeatSendBuffer(const HEART_BEAT eHeartBeatType_) noexcept
	{
		S_ptr<SendBuffer> sendBuffer = SendBufferMgr::Open(sizeof(PacketHeader));
		PacketHeader* const header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->pkt_size = sizeof(PacketHeader);
		header->pkt_id = etoi(eHeartBeatType_);
		sendBuffer->Close(sizeof(PacketHeader));
		return sendBuffer;
	}
}