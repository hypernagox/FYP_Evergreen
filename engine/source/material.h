#pragma once

#include "pch.h"

namespace udsdx
{
	class Shader;
	class Texture;

	struct Material
	{
	public:
		Material() = delete;
		Material(Shader* shader);
		Material(Shader* shader, Texture* texture);

	public:
		void SetShader(Shader* shader);
		void SetSourceTexture(Texture* texture, UINT index = 0);

		Shader* GetShader() const;
		UINT GetTextureCount() const;
		Texture* GetSourceTexture(UINT index = 0) const;

	private:
		Shader* m_shader = nullptr;
		std::array<Texture*, NumTextureSlots> m_mainTex = {};
	};
}