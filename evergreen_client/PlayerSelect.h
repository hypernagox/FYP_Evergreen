#pragma once

#include "pch.h"

class PlayerSelect : public udsdx::Component
{
public:
	void OnInitialize() override;
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void SetShowingCharacter(unsigned int characterIndex);

private:
	float targetRotaitonPivot = 0.0f;
	unsigned int m_showingCharacterIndex = 0;
	udsdx::Vector3 m_targetPosition = Vector3::Zero;
	udsdx::Quaternion m_targetRotation = udsdx::Quaternion::CreateFromYawPitchRoll(-udsdx::PIDIV4, 0.0f, 0.0f);
	std::array<std::shared_ptr<udsdx::SceneObject>, 3> m_characterObjects;
};

