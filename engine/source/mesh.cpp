#include "pch.h"
#include "mesh.h"
#include "debug_console.h"

// Assimp Library
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace udsdx
{
	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<UINT> indices) : MeshBase()
	{
		Submesh submesh{};
		submesh.IndexCount = static_cast<UINT>(indices.size());
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;
		m_submeshes.emplace_back(submesh);

		MeshBase::CreateBuffers<Vertex>(vertices, indices);
		BoundingBox::CreateFromPoints(m_bounds, vertices.size(), &vertices[0].position, sizeof(Vertex));
	}

	Mesh::Mesh(const aiScene& assimpScene, const Matrix4x4& preMultiplication) : MeshBase()
	{
		XMMATRIX vertexTransform = XMLoadFloat4x4(&preMultiplication);

		std::vector<Vertex> vertices;
		std::vector<UINT> indices;

		auto model = &assimpScene;

		for (UINT k = 0; k < model->mNumMeshes; ++k)
		{
			auto mesh = model->mMeshes[k];

			Submesh submesh{};
			submesh.Name = mesh->mName.C_Str();
			submesh.IndexCount = mesh->mNumFaces * 3;
			submesh.StartIndexLocation = static_cast<UINT>(indices.size());
			submesh.BaseVertexLocation = static_cast<UINT>(vertices.size());
			m_submeshes.emplace_back(submesh);

			for (UINT i = 0; i < mesh->mNumVertices; ++i)
			{
				Vertex vertex;
				vertex.position.x = mesh->mVertices[i].x;
				vertex.position.y = mesh->mVertices[i].y;
				vertex.position.z = mesh->mVertices[i].z;
				if (mesh->HasNormals())
				{
					vertex.normal.x = mesh->mNormals[i].x;
					vertex.normal.y = mesh->mNormals[i].y;
					vertex.normal.z = mesh->mNormals[i].z;
				}
				if (mesh->HasTextureCoords(0))
				{
					vertex.uv.x = mesh->mTextureCoords[0][i].x;
					vertex.uv.y = mesh->mTextureCoords[0][i].y;
				}
				if (mesh->HasTangentsAndBitangents())
				{
					vertex.tangent.x = mesh->mTangents[i].x;
					vertex.tangent.y = mesh->mTangents[i].y;
					vertex.tangent.z = mesh->mTangents[i].z;
				}

				XMVECTOR pos = XMLoadFloat3(&vertex.position);
				XMVECTOR nor = XMLoadFloat3(&vertex.normal);
				XMVECTOR tan = XMLoadFloat3(&vertex.tangent);
				pos = XMVector3Transform(pos, vertexTransform);
				nor = XMVector3TransformNormal(nor, vertexTransform);
				tan = XMVector3TransformNormal(tan, vertexTransform);
				XMStoreFloat3(&vertex.position, pos);
				XMStoreFloat3(&vertex.normal, nor);
				XMStoreFloat3(&vertex.tangent, tan);

				vertices.emplace_back(vertex);
			}

			for (UINT i = 0; i < mesh->mNumFaces; ++i)
			{
				auto face = mesh->mFaces[i];
				for (UINT j = 0; j < face.mNumIndices; ++j)
				{
					indices.push_back(face.mIndices[j]);
				}
			}

			DebugConsole::Log(std::string("\tSubmesh \'") + submesh.Name.c_str() + "\' generated");
		}

		CreateBuffers<Vertex>(vertices, indices);
		BoundingBox::CreateFromPoints(m_bounds, vertices.size(), &vertices[0].position, sizeof(Vertex));
	}
}