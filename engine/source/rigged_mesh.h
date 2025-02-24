#pragma once

#include "pch.h"
#include "mesh_base.h"

#include <assimp/scene.h>

namespace udsdx
{
	struct Bone
	{
		std::string Name{};
		Matrix4x4 Transform{};
	};

	class AnimationClip;

	class RiggedMesh : public MeshBase
	{
	public:
		RiggedMesh(const aiScene& assimpScene, const Matrix4x4& preMultiplication);

		// Matrices for default pose (no animation)
		void PopulateTransforms(int submeshIndex, std::vector<Matrix4x4>& out) const;
		void PopulateTransforms(int submeshIndex, const AnimationClip& animationClip, float animationTime, std::vector<Matrix4x4>& out) const;
		int GetBoneIndex(std::string_view boneName) const;
		UINT GetBoneCount() const;

		CD3DX12_GPU_DESCRIPTOR_HANDLE GetBoneGpuSrv() const;

	protected:
		std::vector<Bone> m_bones;
		std::vector<int> m_boneParents;

		std::unordered_map<std::string, int> m_boneIndexMap;
	};
}