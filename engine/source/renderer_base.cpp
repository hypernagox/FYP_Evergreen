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
	RendererBase::RendererBase(const std::shared_ptr<SceneObject>& object) : Component(object)
	{
	}

	void RendererBase::PostUpdate(const Time& time, Scene& scene)
	{
		UpdateTransformCache();
		scene.EnqueueRenderObject(this, m_renderGroup);
		if (m_castShadow)
		{
			scene.EnqueueRenderShadowObject(this);
		}
	}

	void RendererBase::SetShader(Shader* shader)
	{
		m_shader = shader;
	}

	void RendererBase::SetMaterial(Material* material, int index)
	{
		if (m_materials.size() <= index)
		{
			m_materials.resize(index + 1);
		}
		m_materials[index] = material;
	}

	Shader* RendererBase::GetShader() const
	{
		return m_shader;
	}

	Material* RendererBase::GetMaterial(int index) const
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

	void RendererBase::SetReceiveShadow(bool value)
	{
		m_receiveShadow = value;
	}

	D3D_PRIMITIVE_TOPOLOGY RendererBase::GetTopology() const
	{
		return m_topology;
	}

	bool RendererBase::GetCastShadow() const
	{
		return m_castShadow;
	}

	bool RendererBase::GetReceiveShadow() const
	{
		return m_receiveShadow;
	}

	void RendererBase::UpdateTransformCache()
	{
		m_prevTransformCache = std::move(m_transformCache);
		m_transformCache = GetSceneObject()->GetTransform()->GetWorldSRTMatrix();
	}
}