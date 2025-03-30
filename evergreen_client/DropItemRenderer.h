#pragma once

#include "pch.h"

class DropItemRenderer : public udsdx::Component
{
public:
	DropItemRenderer(std::shared_ptr<udsdx::SceneObject> owner);

	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;

	void SetDropItem(uint8_t item_id);

private:
	float m_rotationOffset = 0.0f;
	std::shared_ptr<udsdx::SceneObject> m_rendererObject;
	std::shared_ptr<udsdx::Material> m_material;
	udsdx::MeshRenderer* m_meshRenderer;
};

