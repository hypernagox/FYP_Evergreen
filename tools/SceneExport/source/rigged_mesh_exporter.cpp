#include "rigged_mesh_exporter.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <DirectXMath.h>

#include <assimp/scene.h>

#include "vertex.h"

using namespace DirectX;

RiggedMeshExporter::RiggedMeshExporter() : ExporterBase()
{
}

void RiggedMeshExporter::Export(const aiScene& scene, const std::filesystem::path& outputPath)
{
	std::ofstream file(outputPath, std::ios::binary);

	if (!file.is_open())
	{
		std::cout << "[ERROR]\tFailed to open file for writing: " << outputPath << std::endl;
		return;
	}

	std::vector<RiggedVertex> vertices;
	std::vector<unsigned int> indices;

	auto model = &scene;

	std::vector<Bone> m_bones;
	std::unordered_map<std::string, int> m_boneIndexMap;
	std::vector<int> m_boneParents;
	std::vector<Submesh> m_submeshes;

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
		aiMatrix4x4 m = node.first->mTransformation.Transpose();
		boneData.Transform = XMFLOAT4X4(reinterpret_cast<float*>(&m.a1));

		for (unsigned int i = 0; i < node.first->mNumMeshes; ++i)
		{
			meshStack.emplace_back(node.first, model->mMeshes[node.first->mMeshes[i]]);
		}

		m_boneIndexMap[boneData.Name] = static_cast<int>(m_bones.size());
		m_bones.emplace_back(boneData);
		m_boneParents.push_back(node.second);

		for (unsigned int i = 0; i < node.first->mNumChildren; ++i)
		{
			nodeStack.emplace_back(node.first->mChildren[i], static_cast<int>(m_bones.size()) - 1);
		}
	}
	unsigned int numNodes = static_cast<unsigned int>(m_bones.size());

	// Append the vertices and indices
	for (auto [node, mesh] : meshStack)
	{
		Submesh submesh{};
		submesh.Name = node->mName.C_Str();
		submesh.StartIndexLocation = static_cast<unsigned int>(indices.size());
		submesh.BaseVertexLocation = static_cast<unsigned int>(vertices.size());
		auto it = m_boneIndexMap.find(submesh.Name);
		if (it == m_boneIndexMap.end())
			submesh.NodeID = -1;
		else
			submesh.NodeID = it->second;

		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
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
		for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		{
			auto face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; ++j)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		if (mesh->HasBones())
		{
			std::vector<unsigned int> countTable(vertices.size(), 0);

			// Load the bones
			for (unsigned int i = 0; i < mesh->mNumBones; ++i)
			{
				auto boneSrc = mesh->mBones[i];

				submesh.BoneNodeIDs.emplace_back(boneSrc->mName.C_Str());
				aiMatrix4x4 m = boneSrc->mOffsetMatrix.Transpose();
				submesh.BoneOffsets.emplace_back(reinterpret_cast<float*>(&m.a1));

				for (unsigned int j = 0; j < boneSrc->mNumWeights; ++j)
				{
					unsigned int vertexID = submesh.BaseVertexLocation + boneSrc->mWeights[j].mVertexId;
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
						std::cout << "[ERROR]\tVertex has more than 4 bones affecting it." << std::endl;
						break;
					}
				}
			}
		}

		submesh.IndexCount = static_cast<unsigned int>(indices.size()) - submesh.StartIndexLocation;
		m_submeshes.emplace_back(submesh);

		std::cout << "[LOG]\tSubmesh \'" << submesh.Name.c_str() << "\' generated" << std::endl;
	}

	// Write the number of bones
	size_t boneCount = m_bones.size();
	file.write(reinterpret_cast<const char*>(&boneCount), sizeof(size_t));
	for (const auto& bone : m_bones)
	{
		size_t nameLength = bone.Name.size();
		file.write(reinterpret_cast<const char*>(&nameLength), sizeof(size_t));
		file.write(bone.Name.c_str(), bone.Name.size());
		file.write(reinterpret_cast<const char*>(&bone.Transform), sizeof(XMFLOAT4X4));
	}

	// Write the bone parents
	for (const auto& parent : m_boneParents)
	{
		file.write(reinterpret_cast<const char*>(&parent), sizeof(int));
	}

	// Write the number of submeshes
	size_t submeshCount = m_submeshes.size();
	file.write(reinterpret_cast<const char*>(&submeshCount), sizeof(size_t));
	for (const auto& submesh : m_submeshes)
	{
		size_t nameLength = submesh.Name.size();

		file.write(reinterpret_cast<const char*>(&nameLength), sizeof(size_t));
		file.write(submesh.Name.c_str(), submesh.Name.size());
		file.write(reinterpret_cast<const char*>(&submesh.IndexCount), sizeof(unsigned int));
		file.write(reinterpret_cast<const char*>(&submesh.StartIndexLocation), sizeof(unsigned int));
		file.write(reinterpret_cast<const char*>(&submesh.BaseVertexLocation), sizeof(unsigned int));

		file.write(reinterpret_cast<const char*>(&submesh.NodeID), sizeof(int));
		size_t boneCount = submesh.BoneNodeIDs.size();
		file.write(reinterpret_cast<const char*>(&boneCount), sizeof(size_t));
		for (const auto& boneID : submesh.BoneNodeIDs)
		{
			size_t boneIDLength = boneID.size();
			file.write(reinterpret_cast<const char*>(&boneIDLength), sizeof(size_t));
			file.write(boneID.c_str(), boneID.size());
		}
		for (const auto& boneOffset : submesh.BoneOffsets)
		{
			file.write(reinterpret_cast<const char*>(&boneOffset), sizeof(XMFLOAT4X4));
		}
	}

	// Write the vertices
	size_t vertexCount = vertices.size();
	file.write(reinterpret_cast<const char*>(&vertexCount), sizeof(size_t));
	for (const auto& vertex : vertices)
	{
		file.write(reinterpret_cast<const char*>(&vertex.position), sizeof(XMFLOAT3));
		file.write(reinterpret_cast<const char*>(&vertex.uv), sizeof(XMFLOAT2));
		file.write(reinterpret_cast<const char*>(&vertex.normal), sizeof(XMFLOAT3));
		file.write(reinterpret_cast<const char*>(&vertex.tangent), sizeof(XMFLOAT3));
		file.write(reinterpret_cast<const char*>(&vertex.boneIndices), sizeof(unsigned int));
		file.write(reinterpret_cast<const char*>(&vertex.boneWeights), sizeof(XMFLOAT4));
	}

	// Write the indices
	size_t indexCount = indices.size();
	file.write(reinterpret_cast<const char*>(&indexCount), sizeof(size_t));
	for (const auto& index : indices)
	{
		file.write(reinterpret_cast<const char*>(&index), sizeof(unsigned int));
	}
}