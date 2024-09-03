#pragma once

#include "pch.h"

using namespace udsdx;
class EntityMovement;

class Monster : public Component
{
protected:
	std::shared_ptr<SceneObject> m_rendererObj;
	EntityMovement* m_entityMovement;

	std::shared_ptr<udsdx::Material> m_monsterMaterial;

public:
	Transform* m_transformBody;
	Monster(const std::shared_ptr<SceneObject>& object);
	~Monster();

	void Update(const Time& time, Scene& scene) override;
	const auto& GetRenderObjTransform()const noexcept { return m_rendererObj->GetTransform(); }
};