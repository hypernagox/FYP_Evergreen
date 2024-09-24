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
		Texture* m_mainTex = nullptr;
		Texture* m_normalTex = nullptr;

	public:
		void SetMainTexture(Texture* texture);
		void SetNormalTexture(Texture* texture);

		Texture* GetMainTexture() const;
		Texture* GetNormalTexture() const;
	};
}