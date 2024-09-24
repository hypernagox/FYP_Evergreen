#include "pch.h"
#include "rigged_mesh.h"
#include "debug_console.h"
#include "mesh.h"

// Assimp Library
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static std::tuple<size_t, size_t, float> ToTimeFraction(const std::vector<float>& timeStamps, float time)
{
	auto size = timeStamps.size();
	auto seg = std::distance(timeStamps.begin(), std::lower_bound(timeStamps.begin(), timeStamps.end(), time));
	if (seg == 0)
	{
		return { 0, size - 1, 0.0f };
	}
	if (seg == size)
	{
		return { 0, size - 1, 1.0f };
	}
	float begin = timeStamps[seg - 1];
	float end = timeStamps[seg];
	float fraction = (time - begin) / (end - begin);
	return { seg - 1, seg, fraction };
}

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
	RiggedMesh::RiggedMesh(const aiScene& assimpScene, const Matrix4x4& preMultiplication) : MeshBase()
	{
		XMMATRIX vertexTransform = XMLoadFloat4x4(&preMultiplication);

		std::vector<RiggedVertex> vertices;
		std::vector<UINT> indices;

		auto model = &assimpScene;

		// Depth-first traversal of the scene graph to collect the bones
		std::vector<std::pair<aiNode*, int>> nodeStack;
		nodeStack.emplace_back(model->mRootNode, -1);
		while (!nodeStack.empty())
		{
			auto node = nodeStack.back();
			nodeStack.pop_back();

			Bone boneData{};
			boneData.Name = node.first->mName.C_Str();
			boneData.Transform = ToMatrix4x4(node.first->mTransformation);
			XMStoreFloat4x4(&boneData.Offset, XMMatrixIdentity());

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
		for (UINT k = 0; k < model->mNumMeshes; ++k)
		{
			auto mesh = model->mMeshes[k];

			Submesh submesh{};
			submesh.Name = mesh->mName.C_Str();
			submesh.StartIndexLocation = static_cast<UINT>(indices.size());
			submesh.BaseVertexLocation = static_cast<UINT>(vertices.size());

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

				XMVECTOR pos = XMLoadFloat3(&vertex.position);
				XMVECTOR nor = XMLoadFloat3(&vertex.normal);
				XMVECTOR tan = XMLoadFloat3(&vertex.tangent);
				pos = XMVector3Transform(pos, vertexTransform);
				nor = XMVector3TransformNormal(nor, vertexTransform);
				tan = XMVector3TransformNormal(tan, vertexTransform);
				XMStoreFloat3(&vertex.position, pos);
				XMStoreFloat3(&vertex.normal, nor);
				XMStoreFloat3(&vertex.tangent, tan);

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

			submesh.IndexCount = static_cast<UINT>(indices.size()) - submesh.StartIndexLocation;
			m_submeshes.emplace_back(submesh);

			// If the mesh has no bones, fallback to the hierarchy
			if (!mesh->HasBones())
			{
				UINT boneIndex = m_boneIndexMap[mesh->mName.C_Str()];
				for (UINT i = submesh.BaseVertexLocation; i < vertices.size(); ++i)
				{
					vertices[i].boneIndices = boneIndex;
					vertices[i].boneWeights.x = 1.0f;
				}
			}
			else
			{
				std::vector<UINT> countTable(vertices.size(), 0);

				// Load the bones
				for (UINT i = 0; i < mesh->mNumBones; ++i)
				{
					auto boneSrc = mesh->mBones[i];
					auto boneIndex = m_boneIndexMap[boneSrc->mName.C_Str()];
					m_bones[boneIndex].Offset = ToMatrix4x4(boneSrc->mOffsetMatrix);

					for (UINT j = 0; j < boneSrc->mNumWeights; ++j)
					{
						UINT vertexID = submesh.BaseVertexLocation + boneSrc->mWeights[j].mVertexId;
						float weight = boneSrc->mWeights[j].mWeight;

						switch (++countTable[vertexID])
						{
						case 1:
							vertices[vertexID].boneIndices |= boneIndex;
							vertices[vertexID].boneWeights.x = weight;
							break;
						case 2:
							vertices[vertexID].boneIndices |= boneIndex << 8;
							vertices[vertexID].boneWeights.y = weight;
							break;
						case 3:
							vertices[vertexID].boneIndices |= boneIndex << 16;
							vertices[vertexID].boneWeights.z = weight;
							break;
						case 4:
							vertices[vertexID].boneIndices |= boneIndex << 24;
							vertices[vertexID].boneWeights.w = weight;
							break;
						default:
							DebugConsole::LogError("Vertex has more than 4 bones affecting it.");
							break;
						}
					}
				}
			}

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

		// Load the animations
		for (UINT k = 0; k < model->mNumAnimations; ++k)
		{
			auto animationSrc = model->mAnimations[k];
			Animation animation;

			animation.Name = animationSrc->mName.C_Str();
			animation.TicksPerSecond = static_cast<float>(animationSrc->mTicksPerSecond != 0 ? animationSrc->mTicksPerSecond : 1);
			animation.Duration = static_cast<float>(animationSrc->mDuration);
			animation.Channels.resize(numNodes);

			for (UINT i = 0; i < animationSrc->mNumChannels; ++i)
			{
				auto channelSrc = animationSrc->mChannels[i];
				Animation::Channel channel;

				channel.Name = channelSrc->mNodeName.C_Str();

				for (UINT j = 0; j < channelSrc->mNumPositionKeys; ++j)
				{
					auto key = channelSrc->mPositionKeys[j];
					channel.PositionTimestamps.push_back(static_cast<float>(key.mTime));
					channel.Positions.emplace_back(key.mValue.x, key.mValue.y, key.mValue.z);
				}

				for (UINT j = 0; j < channelSrc->mNumRotationKeys; ++j)
				{
					auto key = channelSrc->mRotationKeys[j];
					channel.RotationTimestamps.push_back(static_cast<float>(key.mTime));
					channel.Rotations.emplace_back(key.mValue.x, key.mValue.y, key.mValue.z, key.mValue.w);
				}

				for (UINT j = 0; j < channelSrc->mNumScalingKeys; ++j)
				{
					auto key = channelSrc->mScalingKeys[j];
					channel.ScaleTimestamps.push_back(static_cast<float>(key.mTime));
					channel.Scales.emplace_back(key.mValue.x, key.mValue.y, key.mValue.z);
				}

				int channelIndex = m_boneIndexMap[channel.Name];
				animation.Channels[channelIndex] = channel;
			}

			m_animations[animation.Name] = animation;
			DebugConsole::Log(std::string("\tAnimation \'") + animation.Name.c_str() + "\' generated");
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

		for (UINT i = 0; i < out.size(); ++i)
		{
			XMMATRIX offset = XMLoadFloat4x4(&m_bones[i].Offset);
			XMMATRIX transform = XMLoadFloat4x4(&out[i]);
			transform = XMMatrixMultiply(offset, transform);
			XMStoreFloat4x4(&out[i], transform);
		}
	}

	void RiggedMesh::PopulateTransforms(std::string_view animationKey, float time, std::vector<Matrix4x4>& out) const
	{
		const Animation& anim = m_animations.at(animationKey.data());
		time = fmod(time * anim.TicksPerSecond, anim.Duration);
		out.resize(m_bones.size());

		for (UINT i = 0; i < out.size(); ++i)
		{
			const Bone& bone = m_bones[i];
			const Animation::Channel& channel = anim.Channels[i];

			XMMATRIX tParent = XMMatrixIdentity();
			if (m_boneParents[i] != -1)
			{
				tParent = XMLoadFloat4x4(&out[m_boneParents[i]]);
			}

			XMMATRIX tLocal;
			if (channel.Name.empty())
				tLocal = XMLoadFloat4x4(&bone.Transform);
			else
			{
				auto [ps1, ps2, pf] = ToTimeFraction(channel.PositionTimestamps, time);
				auto [rs1, rs2, rf] = ToTimeFraction(channel.RotationTimestamps, time);
				auto [ss1, ss2, sf] = ToTimeFraction(channel.ScaleTimestamps, time);

				XMVECTOR p0 = XMLoadFloat3(&channel.Positions[ps1]);
				XMVECTOR p1 = XMLoadFloat3(&channel.Positions[ps2]);
				XMVECTOR p = XMVectorLerp(p0, p1, pf);

				XMVECTOR q0 = XMLoadFloat4(&channel.Rotations[rs1]);
				XMVECTOR q1 = XMLoadFloat4(&channel.Rotations[rs2]);
				XMVECTOR q = XMQuaternionSlerp(q0, q1, rf);

				XMVECTOR s0 = XMLoadFloat3(&channel.Scales[ss1]);
				XMVECTOR s1 = XMLoadFloat3(&channel.Scales[ss2]);
				XMVECTOR s = XMVectorLerp(s0, s1, sf);

				tLocal = XMMatrixAffineTransformation(s, XMVectorZero(), q, p);
			}

			XMStoreFloat4x4(&out[i], XMMatrixMultiply(tLocal, tParent));
		}

		for (UINT i = 0; i < out.size(); ++i)
		{
			XMMATRIX offset = XMLoadFloat4x4(&m_bones[i].Offset);
			XMMATRIX transform = XMLoadFloat4x4(&out[i]);
			transform = XMMatrixMultiply(offset, transform);
			XMStoreFloat4x4(&out[i], transform);
		}
	}

	int RiggedMesh::GetBoneIndex(std::string_view boneName) const
	{
		auto iter = m_boneIndexMap.find(boneName.data());
		if (iter == m_boneIndexMap.end())
			return -1;
		return iter->second;
	}
}