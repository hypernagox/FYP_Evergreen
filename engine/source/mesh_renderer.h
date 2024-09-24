#pragma once

#include "pch.h"
#include "renderer_base.h"

namespace udsdx
{
	class Scene;
	class Mesh;
	class Shader;
	class Material;

	class MeshRenderer : public RendererBase
	{
	public:
		MeshRenderer(const std::shared_ptr<SceneObject>& object);

	public:
		virtual void Render(RenderParam& param, int instances = 1) override;

	public:
		void SetMesh(Mesh* mesh);
		Mesh* GetMesh() const;

		virtual ID3D12PipelineState* GetPipelineState() const override;
		virtual ID3D12PipelineState* GetShadowPipelineState() const override;

	protected:
		Mesh* m_mesh = nullptr;
	};
}