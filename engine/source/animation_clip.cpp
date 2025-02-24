#include "pch.h"
#include "animation_clip.h"
#include "rigged_mesh.h"

// Assimp Library
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


namespace udsdx
{
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

	AnimationClip::AnimationClip(const aiScene& assimpScene, const Matrix4x4& preMultiplication)
	{
		XMMATRIX vertexTransform = XMLoadFloat4x4(&preMultiplication);

		auto model = &assimpScene;

		// Depth-first traversal of the scene graph to collect the bones
		std::vector<std::pair<aiNode*, int>> nodeStack;
		std::vector<std::pair<aiNode*, aiMesh*>> meshStack;
		nodeStack.emplace_back(model->mRootNode, -1);
		while (!nodeStack.empty())
		{
			auto [node, parentIndex] = nodeStack.back();
			nodeStack.pop_back();

			Bone boneData{};
			boneData.Name = node->mName.C_Str();
			if (node == model->mRootNode)
				boneData.Transform = preMultiplication;
			else
				boneData.Transform = ToMatrix4x4(node->mTransformation);

			for (UINT i = 0; i < node->mNumMeshes; ++i)
			{
				meshStack.emplace_back(node, model->mMeshes[node->mMeshes[i]]);
			}

			m_boneIndexMap[boneData.Name] = static_cast<int>(m_bones.size());
			m_bones.emplace_back(boneData);
			m_boneParents.push_back(parentIndex);

			for (UINT i = 0; i < node->mNumChildren; ++i)
			{
				nodeStack.emplace_back(node->mChildren[i], static_cast<int>(m_bones.size()) - 1);
			}
		}
		UINT numNodes = static_cast<UINT>(m_bones.size());

		// The engine only supports one animation for now
		auto animationSrc = model->mAnimations[0];

		m_animation.Name = animationSrc->mName.C_Str();
		m_animation.TicksPerSecond = static_cast<float>(animationSrc->mTicksPerSecond != 0 ? animationSrc->mTicksPerSecond : 1);
		m_animation.Duration = static_cast<float>(animationSrc->mDuration);
		m_animation.Channels.resize(numNodes);

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
			m_animation.Channels[channelIndex] = channel;
		}
	}

	void AnimationClip::PopulateTransforms(float animationTime, const std::vector<std::string>& boneNames, const std::vector<Matrix4x4>& boneOffsets, std::vector<Matrix4x4>& out) const
	{
		animationTime = fmod(animationTime * m_animation.TicksPerSecond, m_animation.Duration);
		std::vector<Matrix4x4> in(m_bones.size());

		for (UINT i = 0; i < m_bones.size(); ++i)
		{
			const Bone& bone = m_bones[i];
			const Animation::Channel& channel = m_animation.Channels[i];

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
				auto [ps1, ps2, pf] = ToTimeFraction(channel.PositionTimestamps, animationTime);
				auto [rs1, rs2, rf] = ToTimeFraction(channel.RotationTimestamps, animationTime);
				auto [ss1, ss2, sf] = ToTimeFraction(channel.ScaleTimestamps, animationTime);

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

		out.resize(boneNames.size());
		for (UINT i = 0; i < out.size(); ++i)
		{
			int boneID = GetBoneIndex(boneNames[i]);
			XMMATRIX boneTransform = boneID >= 0 ? XMLoadFloat4x4(&in[boneID]) : XMMatrixIdentity();
			XMMATRIX boneOffset = XMLoadFloat4x4(&boneOffsets[i]);
			XMStoreFloat4x4(&out[i], XMMatrixTranspose(boneOffset * boneTransform));
		}
	}

	int AnimationClip::GetBoneIndex(std::string_view boneName) const
	{
		auto it = m_boneIndexMap.find(boneName.data());
		if (it == m_boneIndexMap.end())
		{
			return -1;
		}
		return it->second;
	}

	UINT AnimationClip::GetBoneCount() const
	{
		return static_cast<UINT>(m_bones.size());
	}
}