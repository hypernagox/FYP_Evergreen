#pragma once

#include "pch.h"

class PlayerTagGUI : public udsdx::Component
{
public:
	void OnInitialize() override;
	void OnAttach() override;
	void OnActive() override;
	void OnInactive() override;
	void OnDetach() override;
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void UpdateTransform(udsdx::Scene& scene);
	void SetText(std::wstring_view text);

private:
	std::shared_ptr<udsdx::SceneObject> m_nameObject;
};