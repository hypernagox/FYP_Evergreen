#pragma once

#include "pch.h"
#include "frame_resource.h"
#include "component.h"

namespace udsdx
{
	class RiggedMeshRenderer : public Component
	{
		constexpr static unsigned int MAX_BONES = 64U;
	};
}