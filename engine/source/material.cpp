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

	Texture* Material::GetMainTexture() const
	{
		return m_mainTex;
	}
}