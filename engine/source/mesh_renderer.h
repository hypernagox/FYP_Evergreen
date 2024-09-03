#pragma once

#include "pch.h"
#include "component.h"

namespace udsdx
{
	class Scene;
	class Mesh;
	class Shader;
	class Material;

	class MeshRenderer : public Component
	{
	public:
		MeshRenderer(const std::shared_ptr<SceneObject>& object);

	public:
		virtual void Update(const Time& time, Scene& scene) override;
		virtual void Render(RenderParam& param, int instances = 1);

	public:
		void SetMesh(Mesh* mesh);
		void SetShader(Shader* shader);
		void SetMaterial(Material* material);

		void SetCastShadow(bool value);
		void SetReceiveShadow(bool value);

		Mesh* GetMesh() const;
		Shader* GetShader() const;
		Material* GetMaterial() const;

		bool GetCastShadow() const;
		bool GetReceiveShadow() const;

	protected:
		Mesh* m_mesh = nullptr;
		Shader* m_shader = nullptr;
		Material* m_material = nullptr;

		bool m_castShadow = true;
		bool m_receiveShadow = true;
	};
}