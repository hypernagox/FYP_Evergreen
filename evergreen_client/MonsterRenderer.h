#pragma once

#include "pch.h"

class MonsterRenderer : public udsdx::Component
{
public:
	MonsterRenderer(std::shared_ptr<udsdx::SceneObject> owner);

	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;

private:
	std::shared_ptr<udsdx::SceneObject> m_rendererObject;
	std::array<std::shared_ptr<udsdx::Material>, 2> m_materials;
};

