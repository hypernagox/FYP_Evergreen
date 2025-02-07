#include "NagiocpXPch.h"
#include "func.h"
#include "ThreadMgr.h"
#include "Service.h"
#include "Session.h"
#include "RefCountable.h"

namespace NagiocpX
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

	void PrintKoreaRealTime(
		  const std::string_view log_msg
		, const std::wstring_view wlog_msg
		, std::ostream* const stream)noexcept
	{
		if (!wlog_msg.empty()) PrintLogEndl(wlog_msg);
		const auto now_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm local_time{};
		localtime_s(&local_time, &now_time_t);
		char buffer[64];
		const size_t offset = std::strftime(buffer, sizeof(buffer), "  %Y-%m-%d %H:%M:%S [LOG]: ", &local_time);
		std::snprintf(buffer + offset, sizeof(buffer) - offset, "%.*s", static_cast<int>(log_msg.size()), log_msg.data());
		if (stream)*stream << buffer << '\n';
		else PrintLogEndl(buffer);
	}

	S_ptr<ContentsEntity> GetSessionEntity(const uint64_t sessionID_) noexcept
	{
		return Service::GetMainService()->GetSession(sessionID_);
	}

	std::pair<const Session* const, S_ptr<ContentsEntity>> GetSessionAndEntity(const uint64_t sessionID_) noexcept
	{
		if (auto session_entity = GetSessionEntity(sessionID_))
			return { session_entity->GetSession(),session_entity };
		return {};
	}

	void SendPacket(const uint64_t target_session_id, const S_ptr<SendBuffer>& pSendBuffer) noexcept
	{
		if (const auto pSessionEntity = Service::GetMainService()->GetSession(target_session_id))
			pSessionEntity->GetSession()->SendAsync(pSendBuffer);
	}

	void SendPacket(const uint64_t target_session_id, S_ptr<SendBuffer>&& pSendBuffer) noexcept
	{
		if (const auto pSessionEntity = Service::GetMainService()->GetSession(target_session_id))
			pSessionEntity->GetSession()->SendAsync(std::move(pSendBuffer));
	}

	void LogStackTrace() noexcept
	{
		std::ofstream os{ "..\\Crash_Dump.txt" };

		constexpr const int MaxFrames = 64;
		void* stack[MaxFrames];
		const USHORT frames = CaptureStackBackTrace(0, MaxFrames, stack, NULL);
		const auto cur_process = GetCurrentProcess();

		SymInitialize(cur_process, NULL, TRUE);

		SYMBOL_INFO* const symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
		symbol->MaxNameLen = 255;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

		for (USHORT i = 0; i < frames; ++i)
		{
			::SymFromAddr(cur_process, (DWORD64)(stack[i]), 0, symbol);
			std::cout << i << ": " << symbol->Name << " - 0x" << std::hex << symbol->Address << std::dec << std::endl;
			os << i << ": " << symbol->Name << " - 0x" << std::hex << symbol->Address << std::dec << std::endl;
		}

		PrintKoreaRealTime({}, {}, &os);

		free(symbol);
		SymCleanup(cur_process);
	}

	void ReturnSession(Session* const pSession) noexcept
	{
		const_cast<Service* const>(Service::GetMainService())->ReturnSession(pSession);
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