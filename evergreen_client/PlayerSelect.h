#pragma once

#include "pch.h"

class PlayerSelect : public udsdx::Component
{
public:
	PlayerSelect(const std::shared_ptr<udsdx::SceneObject>& object);
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void SetShowingCharacter(unsigned int characterIndex);

private:
	float targetRotaitonPivot = 0.0f;
	unsigned int m_showingCharacterIndex = 0;
	udsdx::Vector3 m_targetPosition = Vector3::Zero;
	udsdx::Quaternion m_targetRotation = Quaternion::CreateFromYawPitchRoll(-PIDIV4, 0.0f, 0.0f);
	std::array<std::shared_ptr<udsdx::SceneObject>, 3> m_characterObjects;
};

