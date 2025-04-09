#include "pch.h"
#include "FocusAgentGUI.h"

FocusAgentGUI::FocusAgentGUI(const std::shared_ptr<udsdx::SceneObject>& object) : udsdx::GUIElement(object)
{

}

void FocusAgentGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
    if (INSTANCE(udsdx::Input)->GetKeyDown(Keyboard::Escape))
    {
        if (m_focused)
        {
            m_focused = false;
            INSTANCE(udsdx::Input)->SetRelativeMouse(false);
        }
        else if (m_tryExitCallback)
        {
            m_tryExitCallback();
        }
    }
}

void FocusAgentGUI::OnMousePress()
{
	if (!m_focused)
	{
		m_focused = true;
		INSTANCE(udsdx::Input)->SetRelativeMouse(true);
	}
    else if (m_tryClickCallback)
	{
        m_tryClickCallback();
	}
}