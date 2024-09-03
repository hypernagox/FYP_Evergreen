#pragma once

#include "pch.h"

namespace udsdx
{
	class ResourceObject
	{
	public:
		ResourceObject();
		ResourceObject(std::wstring_view path);
		virtual ~ResourceObject();
	};
}