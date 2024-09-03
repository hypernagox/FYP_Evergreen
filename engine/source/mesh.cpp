#include "pch.h"
#include "mesh.h"

// Assimp Library
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace udsdx
{
	D3D12_VERTEX_BUFFER_VIEW Mesh::VertexBufferView() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = m_vertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = m_vertexByteStride;
		vbv.SizeInBytes = m_vertexBufferByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW Mesh::IndexBufferView() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = m_indexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = m_indexFormat;
		ibv.SizeInBytes = m_indexBufferByteSize;

		return ibv;
	}

	const std::vector<Mesh::Submesh>& Mesh::GetSubmeshes() const
	{
		return m_submeshes;
	}

	void Mesh::DisposeUploaders()
	{
		m_vertexBufferUploader = nullptr;
		m_indexBufferUploader = nullptr;

		m_vertexBufferCPU = nullptr;
		m_indexBufferCPU = nullptr;
	}

	const BoundingBox& Mesh::GetBounds() const
	{
		return m_bounds;
	}

	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<UINT> indices) : ResourceObject()
	{
		Submesh submesh{};
		submesh.IndexCount = static_cast<UINT>(indices.size());
		submesh.StartIndexLocation = 0;
		submesh.BaseVertexLocation = 0;
		m_submeshes.emplace_back(submesh);

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(UINT);

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &m_vertexBufferCPU));
		CopyMemory(m_vertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &m_indexBufferCPU));
		CopyMemory(m_indexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		m_vertexByteStride = sizeof(Vertex);
		m_vertexBufferByteSize = vbByteSize;
		m_indexBufferByteSize = ibByteSize;

		BoundingBox::CreateFromPoints(m_bounds, vertices.size(), &vertices[0].position, sizeof(Vertex));
	}

	Mesh::Mesh(std::wstring_view resourcePath) : ResourceObject()
	{
		std::vector<Vertex> vertices;
		std::vector<UINT> indices;

		// Read the model from file
		ComPtr<ID3DBlob> modelData;
		ThrowIfFailed(D3DReadFileToBlob(resourcePath.data(), &modelData));

		// Load the model using Assimp
		Assimp::Importer importer;
		auto model = importer.ReadFileFromMemory(
			modelData->GetBufferPointer(),
			static_cast<size_t>(modelData->GetBufferSize()),
			aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace
		);

		assert(model != nullptr);

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
				vertices.push_back(vertex);
			}

			for (UINT i = 0; i < mesh->mNumFaces; ++i)
			{
				auto face = mesh->mFaces[i];
				for (UINT j = 0; j < face.mNumIndices; ++j)
				{
					indices.push_back(face.mIndices[j]);
				}
			}
		}

		const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(UINT);

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &m_vertexBufferCPU));
		CopyMemory(m_vertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &m_indexBufferCPU));
		CopyMemory(m_indexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		m_vertexByteStride = sizeof(Vertex);
		m_vertexBufferByteSize = vbByteSize;
		m_indexBufferByteSize = ibByteSize;

		BoundingBox::CreateFromPoints(m_bounds, vertices.size(), &vertices[0].position, sizeof(Vertex));
	}

	void Mesh::CreateBuffers(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
	{
		m_vertexBufferGPU = d3dUtil::CreateDefaultBuffer(
			device,
			commandList,
			m_vertexBufferCPU->GetBufferPointer(),
			m_vertexBufferByteSize,
			m_vertexBufferUploader
		);

		m_indexBufferGPU = d3dUtil::CreateDefaultBuffer(
			device,
			commandList,
			m_indexBufferCPU->GetBufferPointer(),
			m_indexBufferByteSize,
			m_indexBufferUploader
		);
	}
}