#include "pch.h"
#include "FocusAgentGUI.h"

void FocusAgentGUI::OnMousePress(int mouse)
{
    if (m_tryClickCallback)
	{
        m_tryClickCallback(mouse);
	}
}