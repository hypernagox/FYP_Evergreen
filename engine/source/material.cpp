#include "pch.h"
#include "shader.h"
#include "material.h"

namespace udsdx
{
	Material::Material(Shader* shader) : m_shader(shader)
	{
		if (shader == nullptr)
		{
			throw std::invalid_argument("Shader cannot be null");
		}
	}

	Material::Material(Shader* shader, Texture* texture) : Material(shader)
	{
		m_mainTex[0] = texture;
	}

	void Material::SetShader(Shader* shader)
	{
		if (shader == nullptr)
		{
			throw std::invalid_argument("Shader cannot be null");
		}

		m_shader = shader;
	}

	void Material::SetSourceTexture(Texture* texture, UINT index)
	{
		m_mainTex[index] = texture;
	}

	Shader* Material::GetShader() const
	{
		return m_shader;
	}

	Texture* Material::GetSourceTexture(UINT index) const
	{
		return m_mainTex[index];
	}

	UINT Material::GetTextureCount() const
	{
		return static_cast<UINT>(m_mainTex.size());
	}
}