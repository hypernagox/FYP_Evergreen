#include "static_mesh_exporter.h"

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

StaticMeshExporter::StaticMeshExporter() : ExporterBase()
{
}

void StaticMeshExporter::Export(const aiScene& scene, const std::filesystem::path& outputPath)
{
	std::ofstream file(outputPath, std::ios::binary);

	if (!file.is_open())
	{
		std::cout << "[ERROR]\tFailed to open file for writing: " << outputPath << std::endl;
		return;
	}

	std::vector<Submesh> m_submeshes;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	
	auto model = &scene;
	std::unordered_map<std::string, XMFLOAT4X4> boneTransforms;
	std::queue<aiNode*> bfsSearch;
	bfsSearch.push(model->mRootNode);

	std::vector<std::tuple<unsigned int, aiMesh*, std::string>> meshData;

	while (!bfsSearch.empty())
	{
		auto node = bfsSearch.front();
		bfsSearch.pop();

		const auto nodeName = node->mName.C_Str();

		XMMATRIX parentTransform = XMMatrixIdentity();
		if (node->mParent != nullptr && boneTransforms.find(node->mParent->mName.C_Str()) != boneTransforms.end())
		{
			parentTransform = XMLoadFloat4x4(&boneTransforms[node->mParent->mName.C_Str()]);
		}

		auto nodeTransform = node->mTransformation;
		aiMatrix4x4 m = nodeTransform.Transpose();
		XMMATRIX nodeTransformMatrix = XMLoadFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&m.a1));
		XMMATRIX nodeTransformMatrixGlobal = nodeTransformMatrix * parentTransform;
		XMStoreFloat4x4(&boneTransforms[nodeName], nodeTransformMatrixGlobal);

		for (unsigned int i = 0; i < node->mNumMeshes; ++i)
		{
			auto mesh = model->mMeshes[node->mMeshes[i]];
			meshData.emplace_back(mesh->mMaterialIndex, mesh, nodeName);
		}

		for (unsigned int i = 0; i < node->mNumChildren; ++i)
		{
			bfsSearch.push(node->mChildren[i]);
		}
	}

	std::sort(meshData.begin(), meshData.end());

	for (size_t index = 0; index < meshData.size(); ++index)
	{
		const auto& [materialIndex, mesh, nodeName] = meshData[index];
		XMMATRIX vertexTransform = XMLoadFloat4x4(&boneTransforms[nodeName]);
		unsigned int baseVertexLocation = 0;

		if (index == 0 || std::get<0>(meshData[index - 1]) != materialIndex)
		{
			Submesh& submeshRef = m_submeshes.emplace_back();

			submeshRef.Name = nodeName;
			submeshRef.IndexCount = mesh->mNumFaces * 3;
			submeshRef.StartIndexLocation = static_cast<unsigned int>(indices.size());
			submeshRef.BaseVertexLocation = static_cast<unsigned int>(vertices.size());

			aiMaterial* material = model->mMaterials[materialIndex];
			if (material != nullptr)
			{
				aiString texturePath;
				if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
				{
					submeshRef.DiffuseTexturePath = std::filesystem::path(texturePath.C_Str()).filename().string();
					std::cout << "[LOG]\tDiffuse Texture File Name: " << submeshRef.DiffuseTexturePath.c_str() << std::endl;
				}
				if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS)
				{
					submeshRef.NormalTexturePath = std::filesystem::path(texturePath.C_Str()).filename().string();
					std::cout << "[LOG]\tNormal Texture File Name: " << submeshRef.NormalTexturePath.c_str() << std::endl;
				}
			}

			std::cout << "[LOG]\tSubmesh \'" << submeshRef.Name.c_str() << "\' generated" << std::endl;
		}
		else
		{
			Submesh& submeshRef = m_submeshes.back();
			baseVertexLocation = static_cast<unsigned int>(vertices.size()) - submeshRef.BaseVertexLocation;
			submeshRef.IndexCount += mesh->mNumFaces * 3;
		}

		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
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

		for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		{
			auto face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; ++j)
			{
				indices.push_back(face.mIndices[j] + baseVertexLocation);
			}
		}
	}

	// Write submeshes
	size_t submeshCount = m_submeshes.size();
	file.write(reinterpret_cast<const char*>(&submeshCount), sizeof(size_t));
	for (const auto& submesh : m_submeshes)
	{
		size_t nameLength = submesh.Name.size();
		size_t diffuseTexturePathLength = submesh.DiffuseTexturePath.size();
		size_t normalTexturePathLength = submesh.NormalTexturePath.size();

		file.write(reinterpret_cast<const char*>(&nameLength), sizeof(size_t));
		file.write(submesh.Name.c_str(), submesh.Name.size());
		file.write(reinterpret_cast<const char*>(&submesh.IndexCount), sizeof(unsigned int));
		file.write(reinterpret_cast<const char*>(&submesh.StartIndexLocation), sizeof(unsigned int));
		file.write(reinterpret_cast<const char*>(&submesh.BaseVertexLocation), sizeof(unsigned int));
		file.write(reinterpret_cast<const char*>(&diffuseTexturePathLength), sizeof(size_t));
		file.write(submesh.DiffuseTexturePath.c_str(), submesh.DiffuseTexturePath.size());
		file.write(reinterpret_cast<const char*>(&normalTexturePathLength), sizeof(size_t));
		file.write(submesh.NormalTexturePath.c_str(), submesh.NormalTexturePath.size());
	}

	// Write vertices
	size_t vertexCount = vertices.size();
	file.write(reinterpret_cast<const char*>(&vertexCount), sizeof(size_t));
	for (const auto& vertex : vertices)
	{
		file.write(reinterpret_cast<const char*>(&vertex.position), sizeof(XMFLOAT3));
		file.write(reinterpret_cast<const char*>(&vertex.uv), sizeof(XMFLOAT2));
		file.write(reinterpret_cast<const char*>(&vertex.normal), sizeof(XMFLOAT3));
		file.write(reinterpret_cast<const char*>(&vertex.tangent), sizeof(XMFLOAT3));
	}

	// Write indices
	size_t indexCount = indices.size();
	file.write(reinterpret_cast<const char*>(&indexCount), sizeof(size_t));
	for (const auto& index : indices)
	{
		file.write(reinterpret_cast<const char*>(&index), sizeof(unsigned int));
	}
}