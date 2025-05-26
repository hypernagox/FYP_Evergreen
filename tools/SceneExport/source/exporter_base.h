#pragma once

#include <assimp/scene.h>
#include <filesystem>
#include <string>
#include <vector>
#include <DirectXMath.h>

struct Submesh
{
	// For Regular Mesh
	std::string Name;
	unsigned int IndexCount = 0;
	unsigned int StartIndexLocation = 0;
	unsigned int BaseVertexLocation = 0;

	// For Rigged Mesh
	unsigned int NodeID = 0;
	std::vector<std::string> BoneNodeIDs;
	std::vector<DirectX::XMFLOAT4X4> BoneOffsets;

	// Metadata
	std::string DiffuseTexturePath = {};
	std::string NormalTexturePath = {};
};

struct Bone
{
	std::string Name{};
	DirectX::XMFLOAT4X4 Transform{};
};

class ExporterBase
{
public:
	ExporterBase();

public:
	virtual void Export(const aiScene& scene, const std::filesystem::path& outputPath) = 0;
};