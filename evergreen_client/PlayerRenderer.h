#pragma once

#include "pch.h"

using namespace udsdx;

class PlayerRenderer : public Component
{
protected:
	RiggedMeshRenderer* m_renderer;
	std::shared_ptr<SceneObject> m_rendererObj;
	std::array<std::shared_ptr<udsdx::Material>, 5> m_playerMaterials;
	
public:
	float m_attackTime = 0.0f;

	Transform* m_transformBody;
	PlayerRenderer(const std::shared_ptr<SceneObject>& object);
	~PlayerRenderer();

	void Update(const Time& time, Scene& scene) override;

	Transform* const GetRenderObjTransform() const noexcept { return m_rendererObj->GetTransform(); }
	void SetRotation(const Quaternion& rotation) { m_rendererObj->GetTransform()->SetLocalRotation(rotation); }
	void SetAnimation(const std::string& animationName) { m_renderer->SetAnimation(animationName); }
};