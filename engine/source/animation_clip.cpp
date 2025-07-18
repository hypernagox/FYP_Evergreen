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

		size_t animationCount = 0;
		file.read(reinterpret_cast<char*>(&animationCount), sizeof(size_t));

		for (size_t i = 0; i < animationCount; ++i)
		{
			Animation animationDest = Animation(this, file);
			m_animations.emplace(animationDest.GetName().data(), std::move(animationDest));
		}
	}

	void AnimationClip::PopulateBoneMap(const std::vector<std::string>& boneNames, std::vector<int>& out) const
	{
		out.resize(boneNames.size());
		for (UINT i = 0; i < out.size(); ++i)
		{
			out[i] = GetBoneIndex(boneNames[i]);
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

	const Animation& AnimationClip::GetAnimation(std::string_view name) const
	{
		auto it = m_animations.find(name.data());
		if (it == m_animations.end())
		{
			DebugConsole::LogError("Animation not found: " + std::string(name));
			throw std::runtime_error("Animation not found");
		}
		return it->second;
	}

	const Animation& AnimationClip::GetAnimation() const
	{
		if (m_animations.empty())
		{
			DebugConsole::LogError("No animations available in the clip.");
			throw std::runtime_error("No animations available");
		}
		return m_animations.begin()->second;
	}

	UINT AnimationClip::GetBoneCount() const
	{
		return static_cast<UINT>(m_bones.size());
	}

	Animation::Animation(const AnimationClip* clip, std::ifstream& fileStream) : m_clip(clip)
	{
		// Read animation data
		size_t nameLength = 0;
		fileStream.read(reinterpret_cast<char*>(&nameLength), sizeof(size_t));
		m_name.resize(nameLength);
		fileStream.read(m_name.data(), nameLength);
		fileStream.read(reinterpret_cast<char*>(&m_ticksPerSecond), sizeof(float));
		fileStream.read(reinterpret_cast<char*>(&m_duration), sizeof(float));
		size_t channelCount = 0;

		fileStream.read(reinterpret_cast<char*>(&channelCount), sizeof(size_t));
		m_channels.resize(channelCount);
		for (size_t i = 0; i < channelCount; ++i)
		{
			Animation::Channel& channel = m_channels[i];
			size_t channelNameLength = 0;
			fileStream.read(reinterpret_cast<char*>(&channelNameLength), sizeof(size_t));
			channel.Name.resize(channelNameLength);
			fileStream.read(channel.Name.data(), channelNameLength);

			size_t positionKeyCount = 0;
			fileStream.read(reinterpret_cast<char*>(&positionKeyCount), sizeof(size_t));
			channel.PositionTimestamps.resize(positionKeyCount);
			channel.Positions.resize(positionKeyCount);
			for (size_t j = 0; j < positionKeyCount; ++j)
			{
				fileStream.read(reinterpret_cast<char*>(&channel.PositionTimestamps[j]), sizeof(float));
				fileStream.read(reinterpret_cast<char*>(&channel.Positions[j]), sizeof(Vector3));
			}
			size_t rotationKeyCount = 0;
			fileStream.read(reinterpret_cast<char*>(&rotationKeyCount), sizeof(size_t));
			channel.RotationTimestamps.resize(rotationKeyCount);
			channel.Rotations.resize(rotationKeyCount);
			for (size_t j = 0; j < rotationKeyCount; ++j)
			{
				fileStream.read(reinterpret_cast<char*>(&channel.RotationTimestamps[j]), sizeof(float));
				fileStream.read(reinterpret_cast<char*>(&channel.Rotations[j]), sizeof(Quaternion));
			}
			size_t scaleKeyCount = 0;
			fileStream.read(reinterpret_cast<char*>(&scaleKeyCount), sizeof(size_t));
			channel.ScaleTimestamps.resize(scaleKeyCount);
			channel.Scales.resize(scaleKeyCount);
			for (size_t j = 0; j < scaleKeyCount; ++j)
			{
				fileStream.read(reinterpret_cast<char*>(&channel.ScaleTimestamps[j]), sizeof(float));
				fileStream.read(reinterpret_cast<char*>(&channel.Scales[j]), sizeof(Vector3));
			}
		}
	}

	void Animation::PopulateTransforms(float animationTime, std::vector<Matrix4x4>& out) const
	{
		std::vector<int> boneMap;

		boneMap.reserve(m_clip->GetBoneCount());

		int index = 0;
		for (const Bone& bone : m_clip->GetBones())
		{
			boneMap.push_back(index++);
		}

		PopulateTransforms(animationTime, boneMap, out);
	}

	void Animation::PopulateTransforms(float animationTime, const std::vector<int>& boneMap, std::vector<Matrix4x4>& out, const std::map<std::string_view, Matrix4x4>& modifiers) const
	{
		UINT boneCount = static_cast<UINT>(m_clip->GetBoneCount());

		float animationTicks = animationTime * m_ticksPerSecond;
		std::vector<Matrix4x4> in(boneCount);

		for (UINT i = 0; i < boneCount; ++i)
		{
			const Bone& bone = m_clip->GetBones().at(i);
			const Animation::Channel& channel = m_channels[i];

			XMMATRIX tParent = XMMatrixIdentity();
			if (m_clip->GetBoneParents().at(i) != -1)
			{
				tParent = XMLoadFloat4x4(&in[m_clip->GetBoneParents().at(i)]);
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
				if (modifiers.find(bone.Name) != modifiers.end())
				{
					XMMATRIX modifier = XMLoadFloat4x4(&modifiers.at(bone.Name));
					tLocal = tLocal * modifier;
				}
			}

			XMStoreFloat4x4(&in[i], tLocal * tParent);
		}

		out.resize(boneMap.size());
		for (UINT i = 0; i < out.size(); ++i)
		{
			int boneID = boneMap[i];
			XMMATRIX boneTransform = boneID >= 0 ? XMLoadFloat4x4(&in[boneID]) : XMMatrixIdentity();
			XMStoreFloat4x4(&out[i], boneTransform);
		}
	}
}