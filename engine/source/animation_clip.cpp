#include "pch.h"
#include "animation_clip.h"
#include "rigged_mesh.h"
#include "debug_console.h"


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

	AnimationClip::AnimationClip(const std::filesystem::path& resourcePath)
	{
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

		// Read animation data
		size_t nameLength = 0;
		file.read(reinterpret_cast<char*>(&nameLength), sizeof(size_t));
		m_animation.Name.resize(nameLength);
		file.read(m_animation.Name.data(), nameLength);
		file.read(reinterpret_cast<char*>(&m_animation.TicksPerSecond), sizeof(float));
		file.read(reinterpret_cast<char*>(&m_animation.Duration), sizeof(float));
		size_t channelCount = 0;

		file.read(reinterpret_cast<char*>(&channelCount), sizeof(size_t));
		m_animation.Channels.resize(channelCount);
		for (size_t i = 0; i < channelCount; ++i)
		{
			Animation::Channel& channel = m_animation.Channels[i];
			size_t channelNameLength = 0;
			file.read(reinterpret_cast<char*>(&channelNameLength), sizeof(size_t));
			channel.Name.resize(channelNameLength);
			file.read(channel.Name.data(), channelNameLength);

			size_t positionKeyCount = 0;
			file.read(reinterpret_cast<char*>(&positionKeyCount), sizeof(size_t));
			channel.PositionTimestamps.resize(positionKeyCount);
			channel.Positions.resize(positionKeyCount);
			for (size_t j = 0; j < positionKeyCount; ++j)
			{
				file.read(reinterpret_cast<char*>(&channel.PositionTimestamps[j]), sizeof(float));
				file.read(reinterpret_cast<char*>(&channel.Positions[j]), sizeof(Vector3));
			}
			size_t rotationKeyCount = 0;
			file.read(reinterpret_cast<char*>(&rotationKeyCount), sizeof(size_t));
			channel.RotationTimestamps.resize(rotationKeyCount);
			channel.Rotations.resize(rotationKeyCount);
			for (size_t j = 0; j < rotationKeyCount; ++j)
			{
				file.read(reinterpret_cast<char*>(&channel.RotationTimestamps[j]), sizeof(float));
				file.read(reinterpret_cast<char*>(&channel.Rotations[j]), sizeof(Quaternion));
			}
			size_t scaleKeyCount = 0;
			file.read(reinterpret_cast<char*>(&scaleKeyCount), sizeof(size_t));
			channel.ScaleTimestamps.resize(scaleKeyCount);
			channel.Scales.resize(scaleKeyCount);
			for (size_t j = 0; j < scaleKeyCount; ++j)
			{
				file.read(reinterpret_cast<char*>(&channel.ScaleTimestamps[j]), sizeof(float));
				file.read(reinterpret_cast<char*>(&channel.Scales[j]), sizeof(Vector3));
			}
		}
	}

	void AnimationClip::PopulateTransforms(float animationTime, std::vector<Matrix4x4>& out) const
	{
		std::vector<std::string> boneNames;
		std::vector<Matrix4x4> boneOffsets;

		boneNames.reserve(m_bones.size());
		boneOffsets.reserve(m_bones.size());

		for (const Bone& bone : m_bones)
		{
			boneNames.push_back(bone.Name);
			boneOffsets.push_back(Matrix4x4::Identity);
		}

		PopulateTransforms(animationTime, boneNames, boneOffsets, out);
	}

	void AnimationClip::PopulateTransforms(float animationTime, const std::vector<std::string>& boneNames, const std::vector<Matrix4x4>& boneOffsets, std::vector<Matrix4x4>& out) const
	{
		float animationTicks = animationTime * m_animation.TicksPerSecond;
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
				auto [ps1, ps2, pf] = ToTimeFraction(channel.PositionTimestamps, animationTicks);
				auto [rs1, rs2, rf] = ToTimeFraction(channel.RotationTimestamps, animationTicks);
				auto [ss1, ss2, sf] = ToTimeFraction(channel.ScaleTimestamps, animationTicks);

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

	float AnimationClip::GetAnimationDuration() const
	{
		return m_animation.Duration / m_animation.TicksPerSecond;
	}
}