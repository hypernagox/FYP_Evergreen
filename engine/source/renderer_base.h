#pragma once

#include "pch.h"
#include "component.h"

namespace udsdx
{
	class Scene;
	class Shader;
	class Material;

	class RendererBase : public Component
	{
	public:
		RendererBase(const std::shared_ptr<SceneObject>& object);

	public:
		virtual void Update(const Time& time, Scene& scene) override;
		virtual void Render(RenderParam& param, int instances = 1) = 0;

	public:
		void SetShader(Shader* shader);
		void SetMaterial(Material* material, int index = 0);

		void SetCastShadow(bool value);
		void SetReceiveShadow(bool value);

		Shader* GetShader() const;
		Material* GetMaterial(int index = 0) const;

		bool GetCastShadow() const;
		bool GetReceiveShadow() const;

		virtual ID3D12PipelineState* GetPipelineState() const = 0;
		virtual ID3D12PipelineState* GetShadowPipelineState() const = 0;

	protected:
		Shader* m_shader = nullptr;
		std::vector<Material*> m_materials;

		bool m_castShadow = true;
		bool m_receiveShadow = true;
	};
}