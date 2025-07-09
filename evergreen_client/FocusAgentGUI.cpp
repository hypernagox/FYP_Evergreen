#include "pch.h"
#include "FocusAgentGUI.h"

void FocusAgentGUI::OnMousePress()
{
    if (m_tryClickCallback)
	{
        m_tryClickCallback();
	}
}