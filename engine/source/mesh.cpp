#include "pch.h"
#include "mesh.h"
#include "debug_console.h"

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

	Mesh::Mesh(const std::filesystem::path& resourcePath) : MeshBase()
	{
		std::vector<Vertex> vertices;
		std::vector<UINT> indices;

		std::ifstream file(resourcePath, std::ios::binary);
		if (!file.is_open())
		{
			DebugConsole::LogError("Failed to open rigged mesh file: " + resourcePath.string());
			return;
		}

		size_t submeshCount = 0;
		file.read(reinterpret_cast<char*>(&submeshCount), sizeof(size_t));
		m_submeshes.resize(submeshCount);
		for (size_t i = 0; i < submeshCount; ++i)
		{
			size_t nameLength = 0;
			size_t diffuseTexturePathLength = 0;
			size_t normalTexturePathLength = 0;
			
			Submesh& submesh = m_submeshes[i];
			file.read(reinterpret_cast<char*>(&nameLength), sizeof(size_t));
			submesh.Name.resize(nameLength);
			file.read(submesh.Name.data(), nameLength);
			file.read(reinterpret_cast<char*>(&submesh.IndexCount), sizeof(unsigned int));
			file.read(reinterpret_cast<char*>(&submesh.StartIndexLocation), sizeof(unsigned int));
			file.read(reinterpret_cast<char*>(&submesh.BaseVertexLocation), sizeof(unsigned int));
			file.read(reinterpret_cast<char*>(&diffuseTexturePathLength), sizeof(size_t));
			submesh.DiffuseTexturePath.resize(diffuseTexturePathLength);
			file.read(submesh.DiffuseTexturePath.data(), diffuseTexturePathLength);
			file.read(reinterpret_cast<char*>(&normalTexturePathLength), sizeof(size_t));
			submesh.NormalTexturePath.resize(normalTexturePathLength);
			file.read(submesh.NormalTexturePath.data(), normalTexturePathLength);
		}

		size_t vertexCount = 0;
		file.read(reinterpret_cast<char*>(&vertexCount), sizeof(size_t));
		vertices.resize(vertexCount);
		for (size_t i = 0; i < vertexCount; ++i)
		{
			Vertex& vertex = vertices[i];
			file.read(reinterpret_cast<char*>(&vertex.position), sizeof(Vector3));
			file.read(reinterpret_cast<char*>(&vertex.uv), sizeof(Vector2));
			file.read(reinterpret_cast<char*>(&vertex.normal), sizeof(Vector3));
			file.read(reinterpret_cast<char*>(&vertex.tangent), sizeof(Vector3));
		}

		size_t indexCount = 0;
		file.read(reinterpret_cast<char*>(&indexCount), sizeof(size_t));
		indices.resize(indexCount);
		for (size_t i = 0; i < indexCount; ++i)
		{
			file.read(reinterpret_cast<char*>(&indices[i]), sizeof(UINT));
		}

		CreateBuffers<Vertex>(vertices, indices);
		BoundingBox::CreateFromPoints(m_bounds, vertices.size(), &vertices[0].position, sizeof(Vertex));
	}
}