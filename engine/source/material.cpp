#include "pch.h"
#include "material.h"

namespace udsdx
{
	Material::Material()
	{

	}

	Material::~Material()
	{

	}

	void Material::SetMainTexture(Texture* texture)
	{
		m_mainTex = texture;
	}

	void Material::SetNormalTexture(Texture* texture)
	{
		m_normalTex = texture;
	}

	Texture* Material::GetMainTexture() const
	{
		return m_mainTex;
	}

	Texture* Material::GetNormalTexture() const
	{
		return m_normalTex;
	}
}