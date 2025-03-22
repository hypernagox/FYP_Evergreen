#pragma once

#include "component.h"

namespace udsdx
{
	class Scene;
	class Texture;

	class GUIElement : public Component
	{
	public:
		GUIElement(const std::shared_ptr<SceneObject>& object);

	public:
		void PostUpdate(const Time& time, Scene& scene) override;
		virtual void Render(RenderParam& param) = 0;
	};
}