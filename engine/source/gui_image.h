#pragma once

#include "pch.h"
#include "component.h"
#include "frame_resource.h"

namespace udsdx
{
	class Scene;
	class Texture;

	class GUIImage : public Component
	{
	public:
		static ID3D12PipelineState* GetPipelineState();

	public:
		GUIImage(const std::shared_ptr<SceneObject>& object);

	public:
		void PostUpdate(const Time& time, Scene& scene) override;
		void Render(RenderParam& param, int instances = 1);

	public:
		Vector2 GetSize() const { return m_size; }
		void SetSize(const Vector2& value) { m_size = value; }

		Texture* GetTexture() const { return m_texture; }
		void SetTexture(Texture* value, bool setImageSize = false);

	private:
		static ComPtr<ID3D12PipelineState> g_pipelineState;

	private:
		Texture* m_texture;
		Vector2 m_size = Vector2::One * 100.0f;
	};
}