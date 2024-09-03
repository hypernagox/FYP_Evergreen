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

	public:
		void SetMainTexture(Texture* texture);
		Texture* GetMainTexture() const;
	};
}