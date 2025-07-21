#pragma once

#include "pch.h"

class DropItemAcquireRenderer : public udsdx::Component
{
public:
	void OnInitialize() override;
	void Initialize(udsdx::MeshRenderer* source);
	void Begin() override;
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;

private:
	std::shared_ptr<udsdx::SceneObject> m_rendererObject;
	std::shared_ptr<udsdx::SceneObject> m_heroObject;
	udsdx::MeshRenderer* m_meshRenderer;
	Vector3 m_fromPosition = Vector3::Zero;
	float m_acquireFactor = 0.0f;
};

