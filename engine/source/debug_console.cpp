#include "pch.h"
#include "debug_console.h"

namespace udsdx
{
	std::string ToNarrowString(std::wstring_view wideString)
	{
		std::string narrowString;
		narrowString.resize(wideString.size());
		wcstombs_s(nullptr, narrowString.data(), narrowString.size() + 1, wideString.data(), wideString.size());
		return narrowString;
	}

	void DebugConsole::Log(std::string_view message)
	{
		std::string logMessage = "[LOG] " + std::string(message) + "\n";
		TracyMessageCS(message.data(), message.size(), 0xFFFFFF, STACKTRACE_SIZE);
		std::cout << logMessage;
		OutputDebugStringA(logMessage.c_str());
	}

	void DebugConsole::Log(std::wstring_view message)
	{
		std::wstring logMessage = L"[LOG] " + std::wstring(message) + L"\n";
		TracyMessageCS(ToNarrowString(message).c_str(), message.size(), 0xFFFFFF, STACKTRACE_SIZE);
		std::wcout << logMessage;
		OutputDebugStringW(logMessage.c_str());
	}

	void DebugConsole::LogWarning(std::string_view message)
	{
		std::string logMessage = "[WARNING] " + std::string(message) + "\n";
		TracyMessageCS(message.data(), message.size(), 0xFFFF00, STACKTRACE_SIZE);
		std::cout << logMessage;
		OutputDebugStringA(logMessage.c_str());
	}

	void DebugConsole::LogWarning(std::wstring_view message)
	{
		std::wstring logMessage = L"[WARNING] " + std::wstring(message) + L"\n";
		TracyMessageCS(ToNarrowString(message).c_str(), message.size(), 0xFFFF00, STACKTRACE_SIZE);
		std::wcout << logMessage;
		OutputDebugStringW(logMessage.c_str());
	}

	void DebugConsole::LogError(std::string_view message)
	{
		std::string logMessage = "[ERROR] " + std::string(message) + "\n";
		TracyMessageCS(message.data(), message.size(), 0xFF0000, STACKTRACE_SIZE);
		std::cout << logMessage;
		OutputDebugStringA(logMessage.c_str());
	}

	void DebugConsole::LogError(std::wstring_view message)
	{
		std::wstring logMessage = L"[ERROR] " + std::wstring(message) + L"\n";
		TracyMessageCS(ToNarrowString(message).c_str(), message.size(), 0xFF0000, STACKTRACE_SIZE);
		std::wcout << logMessage;
		OutputDebugStringW(logMessage.c_str());
	}
}
