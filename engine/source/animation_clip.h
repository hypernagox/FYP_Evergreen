#pragma once

#include "pch.h"
#include "resource_object.h"

#include <assimp/scene.h>

namespace udsdx
{
	struct Bone;

	struct Animation
	{
		struct Channel
		{
			std::string Name{};

			std::vector<float> PositionTimestamps{};
			std::vector<float> RotationTimestamps{};
			std::vector<float> ScaleTimestamps{};

			std::vector<Vector3> Positions{};
			std::vector<Quaternion> Rotations{};
			std::vector<Vector3> Scales{};
		};

		std::string Name{};

		float Duration = 0.0f;
		float TicksPerSecond = 1.0f;

		std::vector<Channel> Channels{};
	};

	class AnimationClip : public ResourceObject
	{
	public:
		AnimationClip(const aiScene& assimpScene);

	public:
		void PopulateTransforms(float animationTime, const std::vector<std::string>& boneNames, const std::vector<Matrix4x4>& boneOffsets, std::vector<Matrix4x4>& out) const;
		int GetBoneIndex(std::string_view boneName) const;
		UINT GetBoneCount() const;
		float GetAnimationDuration() const;

	protected:
		Animation m_animation;

		std::vector<Bone> m_bones;
		std::vector<int> m_boneParents;
		std::unordered_map<std::string, int> m_boneIndexMap;
	};
}