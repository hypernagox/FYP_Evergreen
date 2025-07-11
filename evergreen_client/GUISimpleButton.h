#pragma once

#include "pch.h"

class GUISimpleButton : public udsdx::GUIButton
{
public:
	void Render(udsdx::RenderParam& param) override;

	void OnMouseEnter() override;
	void OnMouseRelease() override;

private:
	std::unique_ptr<DirectX::SoundEffectInstance> m_soundInstance;
};