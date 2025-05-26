#pragma once

#include "exporter_base.h"

#include <vector>
#include <string>
#include <DirectXMath.h>

using namespace DirectX;

struct Animation
{
	struct Channel
	{
		std::string Name{};

		std::vector<float> PositionTimestamps{};
		std::vector<float> RotationTimestamps{};
		std::vector<float> ScaleTimestamps{};

		std::vector<XMFLOAT3> Positions{};
		std::vector<XMFLOAT4> Rotations{};
		std::vector<XMFLOAT3> Scales{};
	};

	std::string Name{};

	float Duration = 0.0f;
	float TicksPerSecond = 1.0f;

	std::vector<Channel> Channels{};
};

class AnimationClipExporter : public ExporterBase
{
public:
	AnimationClipExporter();

public:
	void Export(const aiScene& scene, const std::filesystem::path& outputPath) override;
};