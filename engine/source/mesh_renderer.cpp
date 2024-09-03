#include "pch.h"
#include "frame_resource.h"
#include "mesh_renderer.h"
#include "scene_object.h"
#include "transform.h"
#include "material.h"
#include "texture.h"
#include "shader.h"
#include "scene.h"
#include "mesh.h"

namespace udsdx
{
	MeshRenderer::MeshRenderer(const std::shared_ptr<SceneObject>& object)
		: Component(object)
	{
	}

	void MeshRenderer::Update(const Time& time, Scene& scene)
	{
		if (m_mesh == nullptr)
		{
			return;
		}
		scene.EnqueueRenderObject(this);
		if (m_castShadow)
		{
			scene.EnqueueRenderShadowObject(this);
		}
	}

	void MeshRenderer::Render(RenderParam& param, int instances)
	{
		Transform* transform = GetSceneObject()->GetTransform();
		Matrix4x4 worldMat = transform->GetWorldSRTMatrix();

		if (param.UseFrustumCulling)
		{
			// Perform frustum culling
			BoundingBox boundsWorld;
			m_mesh->GetBounds().Transform(boundsWorld, worldMat);
			if (param.ViewFrustumWorld.Contains(boundsWorld) == ContainmentType::DISJOINT)
			{
				return;
			}
		}

		ObjectConstants objectConstants;
		objectConstants.World = worldMat.Transpose();

		param.CommandList->SetGraphicsRoot32BitConstants(0, sizeof(ObjectConstants) / 4, &objectConstants, 0);

		if (m_material != nullptr && m_material->GetMainTexture() != nullptr)
		{
			Texture* mainTex = m_material->GetMainTexture();
			param.CommandList->SetGraphicsRootDescriptorTable(4, mainTex->GetSrvGpu());
		}

		param.CommandList->IASetVertexBuffers(0, 1, &m_mesh->VertexBufferView());
		param.CommandList->IASetIndexBuffer(&m_mesh->IndexBufferView());

		const auto& submeshes = m_mesh->GetSubmeshes();
		for (const auto& submesh : submeshes)
		{
			param.CommandList->DrawIndexedInstanced(submesh.IndexCount, instances, submesh.StartIndexLocation, submesh.BaseVertexLocation, 0);
		}
	}

	void MeshRenderer::SetMesh(Mesh* mesh)
	{
		m_mesh = mesh;
	}

	void MeshRenderer::SetShader(Shader* shader)
	{
		m_shader = shader;
	}

	void MeshRenderer::SetMaterial(Material* material)
	{
		m_material = material;
	}

	Mesh* MeshRenderer::GetMesh() const
	{
		return m_mesh;
	}

	Shader* MeshRenderer::GetShader() const
	{
		return m_shader;
	}

	Material* MeshRenderer::GetMaterial() const
	{
		return m_material;
	}

	void MeshRenderer::SetCastShadow(bool value)
	{
		m_castShadow = value;
	}

	void MeshRenderer::SetReceiveShadow(bool value)
	{
		m_receiveShadow = value;
	}

	bool MeshRenderer::GetCastShadow() const
	{
		return m_castShadow;
	}

	bool MeshRenderer::GetReceiveShadow() const
	{
		return m_receiveShadow;
	}
}