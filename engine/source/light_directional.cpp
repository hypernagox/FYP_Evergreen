#include "pch.h"
#include "light_directional.h"
#include "frame_resource.h"
#include "scene.h"
#include "scene_object.h"
#include "transform.h"

namespace udsdx
{
	LightDirectional::LightDirectional(const std::shared_ptr<SceneObject>& object)
		: Component(object)
	{

	}

	void LightDirectional::Update(const Time& time, Scene& scene)
	{
		scene.EnqueueRenderLight(this);
	}

	Vector3 LightDirectional::GetLightDirection() const
	{
		XMMATRIX worldSRTMatrix = XMLoadFloat4x4(&GetSceneObject()->GetTransform()->GetWorldSRTMatrix());
		XMVECTOR lightDir = XMVector4Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), worldSRTMatrix);
		Vector3 v;
		XMStoreFloat3(&v, lightDir);
		return v;
	}
}