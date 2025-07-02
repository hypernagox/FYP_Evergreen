#include "pch.h"
#include "FocusAgentGUI.h"

FocusAgentGUI::FocusAgentGUI(const std::shared_ptr<udsdx::SceneObject>& object) : udsdx::GUIElement(object)
{

}

void FocusAgentGUI::OnMousePress()
{
    if (m_tryClickCallback)
	{
        m_tryClickCallback();
	}
}