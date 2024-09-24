#pragma once

#include "pch.h"
#include "mesh_base.h"

#include <assimp/scene.h>

namespace udsdx
{
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

	struct Bone
	{
		std::string Name{};
		Matrix4x4 Offset{};
		Matrix4x4 Transform{};
	};

	class RiggedMesh : public MeshBase
	{
	public:
		RiggedMesh(const aiScene& assimpScene, const Matrix4x4& preMultiplication);

		// Matrices for default pose (no animation)
		void PopulateTransforms(std::vector<Matrix4x4>& out) const;
		void PopulateTransforms(std::string_view animationKey, float time, std::vector<Matrix4x4>& out) const;
		int GetBoneIndex(std::string_view boneName) const;

	protected:
		std::vector<Bone> m_bones;
		std::vector<int> m_boneParents;

		std::unordered_map<std::string, Animation> m_animations;
		std::unordered_map<std::string, int> m_boneIndexMap;
	};
}