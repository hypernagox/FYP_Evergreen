#pragma once

#include "pch.h"
#include "renderer_base.h"

namespace udsdx
{
	class Mesh;

	class MeshRenderer : public RendererBase
	{
	public:
		virtual void PostUpdate(const Time& time, Scene& scene) override;
		virtual void Render(RenderParam& param, int instances = 1) override;
		virtual void OnDrawGizmos(const Camera* target) override;

	public:
		void SetMesh(Mesh* mesh);
		Mesh* GetMesh() const;

	protected:
		Mesh* m_mesh = nullptr;
	};
}