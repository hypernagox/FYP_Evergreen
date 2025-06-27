#pragma once

namespace Common
{
	class JsonGenerator
	{
	private:
		friend class DataRegistry;
		JsonGenerator()noexcept = default;
		static bool GenerateJson(const std::wstring_view path)noexcept;
	};
}


