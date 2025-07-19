#include "pch.h"
#include "rigged_mesh.h"
#include "animation_clip.h"
#include "debug_console.h"
#include "mesh.h"

namespace udsdx
{
	RiggedMesh::RiggedMesh(const std::filesystem::path& resourcePath) : MeshBase()
	{
		std::vector<RiggedVertex> vertices;
		std::vector<UINT> indices;

		std::ifstream file(resourcePath, std::ios::binary);
		if (!file.is_open())
		{
			DebugConsole::LogError("Failed to open rigged mesh file: " + resourcePath.string());
			return;
		}

		// Read bone data
		size_t boneCount = 0;
		file.read(reinterpret_cast<char*>(&boneCount), sizeof(size_t));
		m_bones.resize(boneCount);
		m_boneParents.resize(boneCount, -1);
		m_boneIndexMap.clear();
		for (size_t i = 0; i < boneCount; ++i)
		{
			Bone& bone = m_bones[i];
			size_t nameLength = 0;
			file.read(reinterpret_cast<char*>(&nameLength), sizeof(size_t));
			bone.Name.resize(nameLength);
			file.read(bone.Name.data(), nameLength);
			file.read(reinterpret_cast<char*>(&bone.Transform), sizeof(Matrix4x4));
			m_boneIndexMap[bone.Name] = static_cast<int>(i);
		}

		// Read bone parent data
		for (size_t i = 0; i < boneCount; ++i)
		{
			int parentIndex = -1;
			file.read(reinterpret_cast<char*>(&parentIndex), sizeof(int));
			m_boneParents[i] = parentIndex;
		}

		size_t submeshCount = 0;
		file.read(reinterpret_cast<char*>(&submeshCount), sizeof(size_t));
		m_submeshes.resize(submeshCount);
		for (size_t i = 0; i < submeshCount; ++i)
		{
			size_t nameLength = 0;

			Submesh& submesh = m_submeshes[i];
			file.read(reinterpret_cast<char*>(&nameLength), sizeof(size_t));
			submesh.Name.resize(nameLength);
			file.read(submesh.Name.data(), nameLength);
			file.read(reinterpret_cast<char*>(&submesh.IndexCount), sizeof(unsigned int));
			file.read(reinterpret_cast<char*>(&submesh.StartIndexLocation), sizeof(unsigned int));
			file.read(reinterpret_cast<char*>(&submesh.BaseVertexLocation), sizeof(unsigned int));

			file.read(reinterpret_cast<char*>(&submesh.NodeID), sizeof(int));
			size_t boneCount = 0;
			file.read(reinterpret_cast<char*>(&boneCount), sizeof(size_t));
			submesh.BoneNodeIDs.resize(boneCount);
			for (size_t j = 0; j < boneCount; ++j)
			{
				size_t boneNameLength = 0;
				file.read(reinterpret_cast<char*>(&boneNameLength), sizeof(size_t));
				submesh.BoneNodeIDs[j].resize(boneNameLength);
				file.read(submesh.BoneNodeIDs[j].data(), boneNameLength);
			}

			submesh.BoneOffsets.resize(boneCount);
			for (size_t j = 0; j < boneCount; ++j)
			{
				file.read(reinterpret_cast<char*>(&submesh.BoneOffsets[j]), sizeof(Matrix4x4));
			}
		}

		size_t vertexCount = 0;
		file.read(reinterpret_cast<char*>(&vertexCount), sizeof(size_t));
		vertices.resize(vertexCount);
		for (size_t i = 0; i < vertexCount; ++i)
		{
			RiggedVertex& vertex = vertices[i];
			file.read(reinterpret_cast<char*>(&vertex.position), sizeof(Vector3));
			file.read(reinterpret_cast<char*>(&vertex.uv), sizeof(Vector2));
			file.read(reinterpret_cast<char*>(&vertex.normal), sizeof(Vector3));
			file.read(reinterpret_cast<char*>(&vertex.tangent), sizeof(Vector3));
			file.read(reinterpret_cast<char*>(&vertex.boneIndices), sizeof(UINT));
			file.read(reinterpret_cast<char*>(&vertex.boneWeights), sizeof(Vector4));

		}

		size_t indexCount = 0;
		file.read(reinterpret_cast<char*>(&indexCount), sizeof(size_t));
		indices.resize(indexCount);
		for (size_t i = 0; i < indexCount; ++i)
		{
			file.read(reinterpret_cast<char*>(&indices[i]), sizeof(UINT));
		}

		MeshBase::CreateBuffers<RiggedVertex>(vertices, indices);
		BoundingBox::CreateFromPoints(m_bounds, vertices.size(), &vertices[0].position, sizeof(RiggedVertex));
	}

	void RiggedMesh::PopulateTransforms(std::vector<Matrix4x4>& out) const
	{
		out.resize(m_bones.size());

		for (UINT i = 0; i < out.size(); ++i)
		{
			const Bone& bone = m_bones[i];
			XMMATRIX tParent = m_boneParents[i] < 0 ? XMMatrixIdentity() : XMLoadFloat4x4(&out[m_boneParents[i]]);
			XMMATRIX tLocal = XMLoadFloat4x4(&bone.Transform);
			XMStoreFloat4x4(&out[i], XMMatrixMultiply(tLocal, tParent));
		}
	}

	int RiggedMesh::GetBoneIndex(std::string_view boneName) const
	{
		auto iter = m_boneIndexMap.find(boneName.data());
		if (iter == m_boneIndexMap.end())
			return -1;
		return iter->second;
	}

	UINT RiggedMesh::GetBoneCount() const
	{
		return static_cast<UINT>(m_bones.size());
	}

	std::vector<std::string> RiggedMesh::GetBoneNames() const
	{
		std::vector<std::string> boneNames;
		boneNames.reserve(m_bones.size());
		for (const auto& bone : m_bones)
		{
			boneNames.emplace_back(bone.Name);
		}
		return boneNames;
	}

	const std::vector<int>& RiggedMesh::GetBoneParents() const
	{
		return m_boneParents;
	}
}