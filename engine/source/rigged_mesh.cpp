#include "pch.h"
#include "rigged_mesh.h"
#include "animation_clip.h"
#include "debug_console.h"
#include "mesh.h"

// Assimp Library
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static udsdx::Matrix4x4 ToMatrix4x4(const aiMatrix4x4& m)
{
	return udsdx::Matrix4x4(
		m.a1, m.b1, m.c1, m.d1,
		m.a2, m.b2, m.c2, m.d2,
		m.a3, m.b3, m.c3, m.d3,
		m.a4, m.b4, m.c4, m.d4
	);
}

namespace udsdx
{
	RiggedMesh::RiggedMesh(const aiScene& assimpScene) : MeshBase()
	{
		std::vector<RiggedVertex> vertices;
		std::vector<UINT> indices;

		auto model = &assimpScene;

		// Depth-first traversal of the scene graph to collect the bones
		std::vector<std::pair<aiNode*, int>> nodeStack;
		std::vector<std::pair<aiNode*, aiMesh*>> meshStack;
		nodeStack.emplace_back(model->mRootNode, -1);
		while (!nodeStack.empty())
		{
			auto node = nodeStack.back();
			nodeStack.pop_back();

			Bone boneData{};
			boneData.Name = node.first->mName.C_Str();
			boneData.Transform = ToMatrix4x4(node.first->mTransformation);

			for (UINT i = 0; i < node.first->mNumMeshes; ++i)
			{
				meshStack.emplace_back(node.first, model->mMeshes[node.first->mMeshes[i]]);
			}

			m_boneIndexMap[boneData.Name] = static_cast<int>(m_bones.size());
			m_bones.emplace_back(boneData);
			m_boneParents.push_back(node.second);

			for (UINT i = 0; i < node.first->mNumChildren; ++i)
			{
				nodeStack.emplace_back(node.first->mChildren[i], static_cast<int>(m_bones.size()) - 1);
			}
		}
		UINT numNodes = static_cast<UINT>(m_bones.size());

		// Append the vertices and indices
		for (auto [node, mesh] : meshStack)
		{
			Submesh submesh{};
			submesh.Name = node->mName.C_Str();
			submesh.StartIndexLocation = static_cast<UINT>(indices.size());
			submesh.BaseVertexLocation = static_cast<UINT>(vertices.size());
			submesh.NodeID = GetBoneIndex(submesh.Name);

			for (UINT i = 0; i < mesh->mNumVertices; ++i)
			{
				RiggedVertex vertex{};
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

				vertex.boneIndices = 0;
				vertex.boneWeights = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

				vertices.emplace_back(vertex);
			}

			// Load the triangles
			for (UINT i = 0; i < mesh->mNumFaces; ++i)
			{
				auto face = mesh->mFaces[i];
				for (UINT j = 0; j < face.mNumIndices; ++j)
				{
					indices.push_back(face.mIndices[j]);
				}
			}

			if (mesh->HasBones())
			{
				std::vector<UINT> countTable(vertices.size(), 0);

				// Load the bones
				for (UINT i = 0; i < mesh->mNumBones; ++i)
				{
					auto boneSrc = mesh->mBones[i];

					submesh.BoneNodeIDs.emplace_back(boneSrc->mName.C_Str());
					submesh.BoneOffsets.emplace_back(ToMatrix4x4(boneSrc->mOffsetMatrix));

					for (UINT j = 0; j < boneSrc->mNumWeights; ++j)
					{
						UINT vertexID = submesh.BaseVertexLocation + boneSrc->mWeights[j].mVertexId;
						float weight = boneSrc->mWeights[j].mWeight;

						switch (++countTable[vertexID])
						{
						case 1:
							vertices[vertexID].boneIndices |= i;
							vertices[vertexID].boneWeights.x = weight;
							break;
						case 2:
							vertices[vertexID].boneIndices |= i << 8;
							vertices[vertexID].boneWeights.y = weight;
							break;
						case 3:
							vertices[vertexID].boneIndices |= i << 16;
							vertices[vertexID].boneWeights.z = weight;
							break;
						case 4:
							vertices[vertexID].boneIndices |= i << 24;
							vertices[vertexID].boneWeights.w = weight;
							break;
						default:
							DebugConsole::LogError("Vertex has more than 4 bones affecting it.");
							break;
						}
					}
				}
			}

			submesh.IndexCount = static_cast<UINT>(indices.size()) - submesh.StartIndexLocation;
			m_submeshes.emplace_back(submesh);

			// Post-process the bone weights to ensure they sum to 1
			//for (auto& vertex : vertices)
			//{
			//	XMVECTOR weights = XMLoadFloat4(&vertex.boneWeights);
			//	float weightSum = XMVectorGetX(XMVector3Dot(weights, XMVectorReplicate(1.0f)));
			//	if (weightSum > 0.0f)
			//	{
			//		weights = XMVectorDivide(weights, XMVectorReplicate(weightSum));
			//		XMStoreFloat4(&vertex.boneWeights, weights);
			//	}
			//}

			DebugConsole::Log(std::string("\tSubmesh \'") + submesh.Name.c_str() + "\' generated");
		}

		MeshBase::CreateBuffers<RiggedVertex>(vertices, indices);
		BoundingBox::CreateFromPoints(m_bounds, vertices.size(), &vertices[0].position, sizeof(RiggedVertex));
	}

	void RiggedMesh::PopulateTransforms(int submeshIndex, std::vector<Matrix4x4>& out) const
	{
		std::vector<Matrix4x4> in(m_bones.size());

		for (UINT i = 0; i < in.size(); ++i)
		{
			const Bone& bone = m_bones[i];
			XMMATRIX tParent = m_boneParents[i] < 0 ? XMMatrixIdentity() : XMLoadFloat4x4(&in[m_boneParents[i]]);
			XMMATRIX tLocal = XMLoadFloat4x4(&bone.Transform);
			XMStoreFloat4x4(&in[i], XMMatrixMultiply(tLocal, tParent));
		}

		const Submesh& submesh = m_submeshes[submeshIndex];

		out.resize(submesh.BoneNodeIDs.size());
		for (UINT i = 0; i < out.size(); ++i)
		{
			UINT boneID = GetBoneIndex(submesh.BoneNodeIDs[i]);
			XMMATRIX boneTransform = XMLoadFloat4x4(&in[boneID]);
			XMMATRIX boneOffset = XMLoadFloat4x4(&submesh.BoneOffsets[i]);
			XMStoreFloat4x4(&out[i], XMMatrixTranspose(boneTransform * boneOffset));
		}
	}

	void RiggedMesh::PopulateTransforms(int submeshIndex, const AnimationClip& animationClip, float animationTime, std::vector<Matrix4x4>& out) const
	{
		const Submesh& submesh = m_submeshes[submeshIndex];
		animationClip.PopulateTransforms(animationTime, submesh.BoneNodeIDs, submesh.BoneOffsets, out);
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
}