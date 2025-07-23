#include "pch.h"
#include "GUISimpleButton.h"

using namespace udsdx;

void GUISimpleButton::OnMouseEnter()
{
	if (GetInteractable())
	{
		m_soundInstance = INSTANCE(Resource)->Load<udsdx::AudioClip>(RESOURCE_PATH(L"audio\\uihover.wav"))->CreateInstance();
		m_soundInstance->SetVolume(0.25f);
		m_soundInstance->Play();
	}

	GUIButton::OnMouseEnter();
}

void GUISimpleButton::Render(udsdx::RenderParam& param)
{
	GUIButton::Render(param);
}

void GUISimpleButton::OnMouseRelease(int mouse)
{
	if (GetInteractable())
	{
		m_soundInstance = INSTANCE(Resource)->Load<udsdx::AudioClip>(RESOURCE_PATH(L"audio\\uiselect.wav"))->CreateInstance();
		m_soundInstance->SetVolume(0.25f);
		m_soundInstance->Play();
	}

	GUIButton::OnMouseRelease(mouse);
}
