#pragma once

#include "pch.h"
#include "resource_object.h"

namespace udsdx
{
	struct Bone;
	class AnimationClip;

	class Animation
	{
	private:
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

	public:
		Animation() = delete;
		Animation(const AnimationClip* clip, std::ifstream& fileStream);
		void PopulateTransforms(float animationTime, std::vector<Matrix4x4>& out) const;
		void PopulateTransforms(float animationTime, const std::vector<int>& boneMap, std::vector<Matrix4x4>& out, const std::map<std::string_view, Matrix4x4>& modifiers = {}) const;
		float GetAnimationDuration() const { return m_duration / m_ticksPerSecond; }
		std::string_view GetName() const { return m_name; }
		const AnimationClip* GetAnimationClip() const { return m_clip; }

	private:
		const AnimationClip* m_clip = nullptr;
		std::vector<Channel> m_channels;
		std::string m_name;

		float m_duration = 0.0f;
		float m_ticksPerSecond = 30.0f;
	};

	class AnimationClip : public ResourceObject
	{
	public:
		AnimationClip(const std::filesystem::path& resourcePath);

	public:
		void PopulateBoneMap(const std::vector<std::string>& boneNames, std::vector<int>& out) const;
		int GetBoneIndex(std::string_view boneName) const;
		const std::vector<Bone>& GetBones() const { return m_bones; };
		const std::vector<int>& GetBoneParents() const { return m_boneParents; }
		const Animation& GetAnimation(std::string_view name) const;
		const Animation& GetAnimation() const;
		UINT GetBoneCount() const;

	protected:
		std::unordered_map<std::string, Animation> m_animations;

		std::vector<Bone> m_bones;
		std::vector<int> m_boneParents;
		std::unordered_map<std::string, int> m_boneIndexMap;
	};
}