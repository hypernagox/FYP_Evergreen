#pragma once

#include "pch.h"

class DropItemRenderer : public udsdx::Component
{
public:
	void OnInitialize() override;
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;

	void SetDropItem(uint8_t item_id);

private:
	static std::default_random_engine randomEngine;
	float m_rotationOffset = 0.0f;
	float m_scaleFactor = 0.0f;
	std::shared_ptr<udsdx::SceneObject> m_rendererObject;
	udsdx::MeshRenderer* m_meshRenderer;
};

