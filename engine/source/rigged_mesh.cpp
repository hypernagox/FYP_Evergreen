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

				XMMATRIX transform = vertexTransform;
				XMVECTOR pos = XMLoadFloat3(&vertex.position);
				XMVECTOR nor = XMLoadFloat3(&vertex.normal);
				XMVECTOR tan = XMLoadFloat3(&vertex.tangent);
				pos = XMVector3Transform(pos, transform);
				nor = XMVector3TransformNormal(nor, transform);
				tan = XMVector3TransformNormal(tan, transform);
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

			if (mesh->HasBones())
			{
				std::vector<UINT> countTable(vertices.size(), 0);

				// Load the bones
				for (UINT i = 0; i < mesh->mNumBones; ++i)
				{
					auto boneSrc = mesh->mBones[i];
					auto boneIndex = GetBoneIndex(boneSrc->mName.C_Str());

					submesh.BoneNodeIDs.emplace_back(boneIndex);
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

				int channelIndex = GetBoneIndex(channel.Name);
				animation.Channels[channelIndex] = channel;
			}

			m_animations[animation.Name] = animation;
			DebugConsole::Log(std::string("\tAnimation \'") + animation.Name.c_str() + "\' generated");
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

		if (submeshIndex < 0)
		{
			out = in;
			return;
		}

		const Submesh& submesh = m_submeshes[submeshIndex];
		XMMATRIX meshInverse = XMLoadFloat4x4(&in[submesh.NodeID].Invert());

		out.resize(submesh.BoneNodeIDs.size());
		for (UINT i = 0; i < out.size(); ++i)
		{
			UINT boneID = submesh.BoneNodeIDs[i];
			XMMATRIX boneTransform = XMLoadFloat4x4(&in[boneID]);
			XMMATRIX boneOffset = XMLoadFloat4x4(&submesh.BoneOffsets[i]);
			XMStoreFloat4x4(&out[i], XMMatrixTranspose(meshInverse * boneTransform * boneOffset));
		}
	}

	void RiggedMesh::PopulateTransforms(int submeshIndex, std::string_view animationKey, float time, std::vector<Matrix4x4>& out) const
	{
		const Animation& anim = m_animations.at(animationKey.data());
		time = fmod(time * anim.TicksPerSecond, anim.Duration);
		std::vector<Matrix4x4> in(m_bones.size());

		for (UINT i = 0; i < m_bones.size(); ++i)
		{
			const Bone& bone = m_bones[i];
			const Animation::Channel& channel = anim.Channels[i];

			XMMATRIX tParent = XMMatrixIdentity();
			if (m_boneParents[i] != -1)
			{
				tParent = XMLoadFloat4x4(&in[m_boneParents[i]]);
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

			XMStoreFloat4x4(&in[i], tLocal * tParent);
		}

		if (submeshIndex < 0)
		{
			out = in;
			return;
		}

		const Submesh& submesh = m_submeshes[submeshIndex];
		XMMATRIX meshInverse = XMLoadFloat4x4(&in[submesh.NodeID].Invert());

		out.resize(submesh.BoneNodeIDs.size());
		for (UINT i = 0; i < out.size(); ++i)
		{
			UINT boneID = submesh.BoneNodeIDs[i];
			XMMATRIX boneTransform = XMLoadFloat4x4(&in[boneID]);
			XMMATRIX boneOffset = XMLoadFloat4x4(&submesh.BoneOffsets[i]);
			XMStoreFloat4x4(&out[i], XMMatrixTranspose(boneOffset * boneTransform));
		}
	}

	int RiggedMesh::GetBoneIndex(std::string_view boneName) const
	{
		auto iter = m_boneIndexMap.find(boneName.data());
		if (iter == m_boneIndexMap.end())
			return -1;
		return iter->second;
	}

	void RiggedMesh::CreateBoneBuffer()
	{
		std::vector<Matrix4x4> bufferData;
		bufferData.resize(m_submeshes.size() * m_bones.size(), Matrix4x4::Identity);

		for (UINT submeshIndex = 0; submeshIndex < m_submeshes.size(); ++submeshIndex)
		{
			const Submesh& subMesh = m_submeshes[submeshIndex];
			for (UINT i = 0; i < subMesh.BoneNodeIDs.size(); ++i)
			{
				UINT boneID = subMesh.BoneNodeIDs[i];
				if (boneID < 0)
					continue;

				// Store the bone offset matrix in the buffer
				bufferData[submeshIndex * m_bones.size() + boneID] = subMesh.BoneOffsets[i];
			}
		}

		for (const auto& [key, animation] : m_animations)
		{
			// Number of frames are reduced as 30 frames per second for each animation
			// Because, what the hell is over 1000 frames per second exists?
			UINT numFrames = static_cast<UINT>(animation.Duration * AnimationFrameRate / animation.TicksPerSecond);
			UINT frameBase = static_cast<UINT>(bufferData.size());
			m_animationFrameBaseMap[key] = frameBase / m_bones.size();
			m_animationFrameNumMap[key] = numFrames;
			bufferData.resize(bufferData.size() + numFrames * m_bones.size(), Matrix4x4::Identity);

			for (UINT frameIndex = 0; frameIndex < numFrames; ++frameIndex)
			{
				Matrix4x4* frameBuffer = &bufferData[frameBase + frameIndex * m_bones.size()];
				float frameTime = frameIndex * animation.TicksPerSecond / AnimationFrameRate;

				// Evaluate each frame of the animation
				for (UINT boneIndex = 0; boneIndex < m_bones.size(); ++boneIndex)
				{
					const Bone& bone = m_bones[boneIndex];
					const Animation::Channel& channel = animation.Channels[boneIndex];

					XMMATRIX tParent = XMMatrixIdentity();
					if (m_boneParents[boneIndex] != -1)
					{
						tParent = XMLoadFloat4x4(&frameBuffer[m_boneParents[boneIndex]]);
					}

					XMMATRIX tLocal;
					if (channel.Name.empty())
						tLocal = XMLoadFloat4x4(&bone.Transform);
					else
					{
						auto [ps1, ps2, pf] = ToTimeFraction(channel.PositionTimestamps, frameTime);
						auto [rs1, rs2, rf] = ToTimeFraction(channel.RotationTimestamps, frameTime);
						auto [ss1, ss2, sf] = ToTimeFraction(channel.ScaleTimestamps, frameTime);

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

					XMStoreFloat4x4(&frameBuffer[boneIndex], tLocal * tParent);
				}
			}
		}

		// Transpose the matrices for HLSL compatibility
		for (int i = 0; i < bufferData.size(); ++i)
		{
			bufferData[i] = bufferData[i].Transpose();
		}

		const UINT bufferByteSize = static_cast<UINT>(bufferData.size()) * sizeof(Matrix4x4);

		ThrowIfFailed(D3DCreateBlob(bufferByteSize, &m_boneBufferCPU));
		CopyMemory(m_boneBufferCPU->GetBufferPointer(), bufferData.data(), bufferByteSize);
	}

	void RiggedMesh::UploadBoneBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList)
	{
		// Make sure buffers are uploaded to the CPU.
		assert(m_boneBufferCPU != nullptr);

		m_boneBufferGpu = d3dUtil::CreateDefaultBuffer(
			device,
			commandList,
			m_boneBufferCPU->GetBufferPointer(),
			m_boneBufferCPU->GetBufferSize(),
			m_boneBufferUpload
		);
	}

	void RiggedMesh::BuildBoneDescriptors(ID3D12Device* device, DescriptorParam& descriptorParam)
	{
		m_boneCpuSrv = descriptorParam.SrvCpuHandle;
		m_boneGpuSrv = descriptorParam.SrvGpuHandle;

		descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
		descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = m_boneBufferCPU->GetBufferSize() / sizeof(Matrix4x4);
		srvDesc.Buffer.StructureByteStride = sizeof(Matrix4x4);
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		device->CreateShaderResourceView(m_boneBufferGpu.Get(), &srvDesc, m_boneCpuSrv);
	}

	std::pair<int, float> RiggedMesh::GetAnimationFrameTime(std::string_view animation, float time)
	{
		const Animation& target = m_animations[animation.data()];
		int base = m_animationFrameBaseMap[animation.data()];
		int num = m_animationFrameNumMap[animation.data()] - 1;
		float frameTime = fmod(time * AnimationFrameRate, static_cast<float>(num));
		return std::make_pair(base + static_cast<int>(frameTime), fmod(frameTime, 1.0f));
	}

	UINT RiggedMesh::GetBoneCount() const
	{
		return static_cast<UINT>(m_bones.size());
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE RiggedMesh::GetBoneGpuSrv() const
	{
		return m_boneGpuSrv;
	}
}