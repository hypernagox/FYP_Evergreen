#pragma once

#include "pch.h"

namespace udsdx
{
	class Texture;
	class Material
	{
	public:
		Material();
		~Material();

	private:
		std::array<Texture*, 8> m_mainTex = {};

	public:
		void SetSourceTexture(Texture* texture, UINT index = 0);

		UINT GetTextureCount() const;
		Texture* GetSourceTexture(UINT index = 0) const;
	};
}