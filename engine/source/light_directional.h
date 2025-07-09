#pragma once

#include "pch.h"
#include "component.h"

namespace udsdx
{
	class Scene;

	class LightDirectional : public Component
	{
	public:
		virtual void PostUpdate(const Time& time, Scene& scene) override;

	public:
		Vector3 GetLightDirection() const;
	};
}