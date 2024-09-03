#include "ClientNetworkPch.h"
#include "func.h"
#include "SendBuffer.h"

namespace NetHelper
{
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

	S_ptr<SendBuffer> CreateHeartBeatSendBuffer(const HEART_BEAT eHeartBeatType_) noexcept
	{
		S_ptr<SendBuffer> sendBuffer = NetMgr(SendBufferMgr)->Open(sizeof(PacketHeader));
		PacketHeader* const header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->pkt_size = sizeof(PacketHeader);
		header->pkt_id = etoi(eHeartBeatType_);
		sendBuffer->Close(sizeof(PacketHeader));
		return sendBuffer;
	}

	void Send(S_ptr<SendBuffer> pSendBuffer) noexcept
	{
		NetMgr(NetworkMgr)->Send(std::move(pSendBuffer));
	}
}
