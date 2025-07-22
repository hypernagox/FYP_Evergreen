#include "pch.h"
#include "renderer_base.h"
#include "frame_resource.h"
#include "scene_object.h"
#include "transform.h"
#include "material.h"
#include "texture.h"
#include "shader.h"
#include "scene.h"

namespace udsdx
{
	void RendererBase::PostUpdate(const Time& time, Scene& scene)
	{
		m_transformCacheDirty = true;
	}

	void RendererBase::SetMaterial(const Material& material, int index)
	{
		while (m_materials.size() <= index)
		{
			m_materials.emplace_back(material);
		}
		m_materials[index] = material;
	}

	Material RendererBase::GetMaterial(int index) const
	{
		return m_materials[index];
	}

	void RendererBase::SetTopology(D3D_PRIMITIVE_TOPOLOGY value)
	{
		m_topology = value;
	}

	void RendererBase::SetCastShadow(bool value)
	{
		m_castShadow = value;
	}

	D3D_PRIMITIVE_TOPOLOGY RendererBase::GetTopology() const
	{
		return m_topology;
	}

	bool RendererBase::GetCastShadow() const
	{
		return m_castShadow;
	}

	void RendererBase::ValidateTransformCache()
	{
		if (m_transformCacheDirty)
		{
			UpdateTransformCache();
			if (m_transformFirstValid)
			{
				UpdateTransformCache();
				m_transformFirstValid = false;
			}
			m_transformCacheDirty = false;
		}
	}

	void RendererBase::UpdateTransformCache()
	{
		m_prevTransformCache = std::move(m_transformCache);
		m_transformCache = GetSceneObject()->GetTransform()->GetWorldSRTMatrix(false);
	}
}