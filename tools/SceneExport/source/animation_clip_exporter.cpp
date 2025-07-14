#include "animation_clip_exporter.h"

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

AnimationClipExporter::AnimationClipExporter() : ExporterBase()
{
}

void AnimationClipExporter::Export(const aiScene& scene, const std::filesystem::path& outputPath)
{
	std::ofstream file(outputPath, std::ios::binary);

	if (!file.is_open())
	{
		std::cout << "[ERROR]\tFailed to open file for writing: " << outputPath << std::endl;
		return;
	}

	auto model = &scene;

	std::vector<Bone> m_bones;
	std::unordered_map<std::string, int> m_boneIndexMap;
	std::vector<int> m_boneParents;
	std::vector<Animation> m_animations;

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
		aiMatrix4x4 m = node->mTransformation.Transpose();
		boneData.Transform = XMFLOAT4X4(reinterpret_cast<float*>(&m.a1));

		for (unsigned int i = 0; i < node->mNumMeshes; ++i)
		{
			meshStack.emplace_back(node, model->mMeshes[node->mMeshes[i]]);
		}

		m_boneIndexMap[boneData.Name] = static_cast<int>(m_bones.size());
		m_bones.emplace_back(boneData);
		m_boneParents.push_back(parentIndex);

		for (unsigned int i = 0; i < node->mNumChildren; ++i)
		{
			nodeStack.emplace_back(node->mChildren[i], static_cast<int>(m_bones.size()) - 1);
		}
	}
	unsigned int numNodes = static_cast<unsigned int>(m_bones.size());

	// The engine only supports one animation for now
	for (unsigned int i = 0; i < model->mNumAnimations; ++i)
	{
		auto animationSrc = model->mAnimations[i];
		Animation& m_animation = m_animations.emplace_back();

		m_animation.Name = animationSrc->mName.C_Str();
		m_animation.TicksPerSecond = static_cast<float>(animationSrc->mTicksPerSecond != 0 ? animationSrc->mTicksPerSecond : 1);
		m_animation.Duration = static_cast<float>(animationSrc->mDuration);
		m_animation.Channels.resize(numNodes);

		for (unsigned int i = 0; i < animationSrc->mNumChannels; ++i)
		{
			auto channelSrc = animationSrc->mChannels[i];
			Animation::Channel channel;

			channel.Name = channelSrc->mNodeName.C_Str();

			for (unsigned int j = 0; j < channelSrc->mNumPositionKeys; ++j)
			{
				auto key = channelSrc->mPositionKeys[j];
				channel.PositionTimestamps.push_back(static_cast<float>(key.mTime));
				channel.Positions.emplace_back(key.mValue.x, key.mValue.y, key.mValue.z);
			}

			for (unsigned int j = 0; j < channelSrc->mNumRotationKeys; ++j)
			{
				auto key = channelSrc->mRotationKeys[j];
				channel.RotationTimestamps.push_back(static_cast<float>(key.mTime));
				channel.Rotations.emplace_back(key.mValue.x, key.mValue.y, key.mValue.z, key.mValue.w);
			}

			for (unsigned int j = 0; j < channelSrc->mNumScalingKeys; ++j)
			{
				auto key = channelSrc->mScalingKeys[j];
				channel.ScaleTimestamps.push_back(static_cast<float>(key.mTime));
				channel.Scales.emplace_back(key.mValue.x, key.mValue.y, key.mValue.z);
			}

			auto it = m_boneIndexMap.find(channel.Name);
			if (it == m_boneIndexMap.end())
			{
				std::cout << "[WARNING]\tChannel " + channel.Name + " not found in bone list." << std::endl;
				continue;
			}
			m_animation.Channels[it->second] = channel;
		}
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

	size_t animationCount = m_animations.size();
	file.write(reinterpret_cast<const char*>(&animationCount), sizeof(size_t));
	for (const auto& m_animation : m_animations)
	{
		// Write the animation data
		size_t nameLength = m_animation.Name.size();
		file.write(reinterpret_cast<const char*>(&nameLength), sizeof(size_t));
		file.write(m_animation.Name.c_str(), m_animation.Name.size());
		file.write(reinterpret_cast<const char*>(&m_animation.TicksPerSecond), sizeof(float));
		file.write(reinterpret_cast<const char*>(&m_animation.Duration), sizeof(float));

		size_t channelCount = m_animation.Channels.size();
		file.write(reinterpret_cast<const char*>(&channelCount), sizeof(size_t));
		for (const auto& channel : m_animation.Channels)
		{
			size_t channelNameLength = channel.Name.size();
			file.write(reinterpret_cast<const char*>(&channelNameLength), sizeof(size_t));
			file.write(channel.Name.c_str(), channel.Name.size());
			size_t positionKeyCount = channel.PositionTimestamps.size();
			file.write(reinterpret_cast<const char*>(&positionKeyCount), sizeof(size_t));
			for (size_t j = 0; j < positionKeyCount; ++j)
			{
				float timestamp = channel.PositionTimestamps[j];
				file.write(reinterpret_cast<const char*>(&timestamp), sizeof(float));
				file.write(reinterpret_cast<const char*>(&channel.Positions[j]), sizeof(XMFLOAT3));
			}
			size_t rotationKeyCount = channel.RotationTimestamps.size();
			file.write(reinterpret_cast<const char*>(&rotationKeyCount), sizeof(size_t));
			for (size_t j = 0; j < rotationKeyCount; ++j)
			{
				float timestamp = channel.RotationTimestamps[j];
				file.write(reinterpret_cast<const char*>(&timestamp), sizeof(float));
				file.write(reinterpret_cast<const char*>(&channel.Rotations[j]), sizeof(XMFLOAT4));
			}
			size_t scaleKeyCount = channel.ScaleTimestamps.size();
			file.write(reinterpret_cast<const char*>(&scaleKeyCount), sizeof(size_t));
			for (size_t j = 0; j < scaleKeyCount; ++j)
			{
				float timestamp = channel.ScaleTimestamps[j];
				file.write(reinterpret_cast<const char*>(&timestamp), sizeof(float));
				file.write(reinterpret_cast<const char*>(&channel.Scales[j]), sizeof(XMFLOAT3));
			}
		}
	}
}
