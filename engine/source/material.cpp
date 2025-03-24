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

	void Material::SetSourceTexture(Texture* texture, UINT index)
	{
		m_mainTex[index] = texture;
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