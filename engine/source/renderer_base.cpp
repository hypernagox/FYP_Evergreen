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

	void RendererBase::Update(const Time& time, Scene& scene)
	{
		scene.EnqueueRenderObject(this);
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

	void RendererBase::SetCastShadow(bool value)
	{
		m_castShadow = value;
	}

	void RendererBase::SetReceiveShadow(bool value)
	{
		m_receiveShadow = value;
	}

	bool RendererBase::GetCastShadow() const
	{
		return m_castShadow;
	}

	bool RendererBase::GetReceiveShadow() const
	{
		return m_receiveShadow;
	}
}