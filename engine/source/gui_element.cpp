#include "pch.h"
#include "gui_element.h"
#include "scene.h"

namespace udsdx
{
	GUIElement::GUIElement(const std::shared_ptr<SceneObject>& object) : Component(object)
	{
	}

	void GUIElement::PostUpdate(const Time& time, Scene& scene)
	{
		scene.EnqueueRenderGUIObject(this);
	}
}