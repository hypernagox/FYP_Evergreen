#pragma once

#include "pch.h"

#include <assimp/LogStream.hpp>

namespace udsdx
{
	class AssimpLogStream : public Assimp::LogStream
	{
	public:
		void write(const char* message) override;
	};

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