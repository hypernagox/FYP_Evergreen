#include "pch.h"
#include "rigged_prop_renderer.h"
#include "rigged_mesh_renderer.h"
#include "scene_object.h"
#include "rigged_mesh.h"
#include "transform.h"

namespace udsdx
{
	udsdx::RiggedPropRenderer::RiggedPropRenderer(const std::shared_ptr<SceneObject>& object) : MeshRenderer(object)
	{
		m_targetCache = object->GetComponent<RiggedMeshRenderer>();
		if (!m_targetCache)
		{
			throw std::runtime_error("RiggedPropRenderer requires a RiggedMeshRenderer component on the target object.");
		}
	}

	void RiggedPropRenderer::UpdateTransformCache()
	{
		Matrix4x4 boneMatrix = m_targetCache->PopulateTransform(m_boneName).Transpose();
		m_prevTransformCache = std::move(m_transformCache);
		m_transformCache = m_propLocalTransform * boneMatrix * GetSceneObject()->GetTransform()->GetWorldSRTMatrix();
	}

	std::string_view RiggedPropRenderer::GetBoneName() const
	{
		return m_boneName;
	}

	void RiggedPropRenderer::SetBoneName(const std::string& boneName)
	{
		m_boneName = boneName;
	}

	void RiggedPropRenderer::SetPropLocalTransform(const Matrix4x4& transform)
	{
		m_propLocalTransform = transform;
	}
}