#pragma once

#include "pch.h"

namespace udsdx
{
	class DebugConsole
	{
	private:
		static constexpr int STACKTRACE_SIZE = 16;

	public:
		static void Log(std::string_view message);
		static void Log(std::wstring_view message);
		static void LogWarning(std::string_view message);
		static void LogWarning(std::wstring_view message);
		static void LogError(std::string_view message);
		static void LogError(std::wstring_view message);
	};
}