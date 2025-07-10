#include "pch.h"
#include "func.h"


std::string WideToUtf8(const std::wstring_view wstr) noexcept
{
	const int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], sizeNeeded, NULL, NULL);
	return strTo;
}

std::wstring Utf8ToWide(const std::string_view utf8Str)noexcept
{
	const int charsNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8Str.data(), -1, NULL, 0);
	std::wstring wideStr(charsNeeded, 0);
	const int charsConverted = MultiByteToWideChar(CP_UTF8, 0, utf8Str.data(), -1, &wideStr[0], charsNeeded);
	wideStr.pop_back();
	return wideStr;
}

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

