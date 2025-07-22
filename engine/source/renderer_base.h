#pragma once

#include "pch.h"
#include "component.h"
#include "material.h"

namespace udsdx
{
	class Scene;
	class Shader;

	class RendererBase : public Component
	{
	public:
		virtual void PostUpdate(const Time& time, Scene& scene) override;
		virtual void Render(RenderParam& param, int parameter) = 0;

	public:
		void SetMaterial(const Material& material, int index = 0);

		void SetTopology(D3D_PRIMITIVE_TOPOLOGY value);
		void SetCastShadow(bool value);
		void SetDrawOutline(bool value) { m_drawOutline = value; }

		D3D_PRIMITIVE_TOPOLOGY GetTopology() const;
		Material GetMaterial(int index = 0) const;

		bool GetCastShadow() const;
		bool GetDrawOutline() const { return m_drawOutline; }

		void ValidateTransformCache();
		virtual void UpdateTransformCache();

	protected:
		std::vector<Material> m_materials;

		D3D_PRIMITIVE_TOPOLOGY m_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		bool m_castShadow = true;
		bool m_drawOutline = false;
		RenderGroup m_renderGroup = RenderGroup::Deferred;

		bool m_transformCacheDirty = true;
		bool m_transformFirstValid = true;
		Matrix4x4 m_transformCache = Matrix4x4::Identity;
		Matrix4x4 m_prevTransformCache = Matrix4x4::Identity;
	};
}