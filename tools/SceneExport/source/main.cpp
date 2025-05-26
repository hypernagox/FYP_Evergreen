#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <filesystem>
#include <cassert>
#include <memory>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

#include "exporter_base.h"
#include "static_mesh_exporter.h"
#include "rigged_mesh_exporter.h"
#include "animation_clip_exporter.h"

class AssimpLogStream : public Assimp::LogStream
{
public:
	void write(const char* message) override;
};

void AssimpLogStream::write(const char* message)
{
	// remove the newline character
	std::string_view messageView(message);
	if (messageView.back() == '\n')
	{
		messageView.remove_suffix(1);
	}
	std::cout << "[ASSIMP LOG]\t" << messageView << std::endl;
}

std::set<std::string> g_supportedExtensions;

void InitializeSuppotedExtensions(const Assimp::Importer& importer)
{
	std::string extensions;
	importer.GetExtensionList(extensions);
	
	// Erase the asterisks from the string
	extensions.erase(std::remove(extensions.begin(), extensions.end(), '*'), extensions.end());

	// Split the string by semicolon
	std::string delimiter = ";";
	size_t pos = 0;
	while ((pos = extensions.find(delimiter)) != std::string::npos)
	{
		std::string token = extensions.substr(0, pos);
		g_supportedExtensions.emplace(token);
		extensions.erase(0, pos + delimiter.length());
	}
	if (!extensions.empty())
	{
		g_supportedExtensions.emplace(extensions);
	}
}

std::vector<std::pair<std::string, std::unique_ptr<ExporterBase>>> g_exporters;
enum class ExporterType : size_t
{
	StaticMesh,
	RiggedMesh,
	AnimationClip
};

void InitializeExporters()
{
	g_exporters.resize(3);
	g_exporters[static_cast<size_t>(ExporterType::StaticMesh)] = std::make_pair(".yms", std::make_unique<StaticMeshExporter>());
	g_exporters[static_cast<size_t>(ExporterType::RiggedMesh)] = std::make_pair(".yrms", std::make_unique<RiggedMeshExporter>());
	g_exporters[static_cast<size_t>(ExporterType::AnimationClip)] = std::make_pair(".yac", std::make_unique<AnimationClipExporter>());
}

int main(int argc, char* argv[])
{
	// Getting the second argument as a file path
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
		return 1;
	}
	std::string filePath = argv[1];

	Assimp::DefaultLogger::create();
	Assimp::DefaultLogger::get()->attachStream(new AssimpLogStream(), Assimp::Logger::VERBOSE);

	Assimp::Importer importer;
	InitializeSuppotedExtensions(importer);
	InitializeExporters();

	// Iterate through all subdirectories of the given path
	for (const auto& entry : std::filesystem::recursive_directory_iterator(filePath)) {
		if (entry.is_directory())
		{
			continue;
		}

		std::string extension = entry.path().extension().string();
		// Check if the extension is supported
		if (g_supportedExtensions.find(extension) == g_supportedExtensions.end())
		{
			continue;
		}

		std::cout << "[LOG]\tProcessing file: " << entry.path() << std::endl;

		// Load the model using Assimp
		Assimp::Importer importer;
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
		auto assimpScene = importer.ReadFile(
			entry.path().string(),
			aiProcess_ConvertToLeftHanded |
			aiProcess_Triangulate |
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_LimitBoneWeights |
			aiProcess_OptimizeMeshes |
			aiProcess_RemoveRedundantMaterials
		);

		if (!assimpScene)
		{
			std::cout << "[ERROR]\tFailed to load model: " << importer.GetErrorString() << std::endl;
			continue;
		}

		bool hasBones = false;
		for (unsigned int i = 0; i < assimpScene->mNumMeshes && !hasBones; ++i)
		{
			hasBones |= assimpScene->mMeshes[i]->HasBones();
		}

		if (assimpScene->mMetaData)
		{
			int32_t UpAxis = 1, UpAxisSign = 1, FrontAxis = 2, FrontAxisSign = 1, CoordAxis = 0, CoordAxisSign = 1;
			double UnitScaleFactor = 1.0;
			for (unsigned MetadataIndex = 0; MetadataIndex < assimpScene->mMetaData->mNumProperties; ++MetadataIndex)
			{
				if (strcmp(assimpScene->mMetaData->mKeys[MetadataIndex].C_Str(), "UpAxis") == 0)
				{
					assimpScene->mMetaData->Get<int32_t>(MetadataIndex, UpAxis);
				}
				if (strcmp(assimpScene->mMetaData->mKeys[MetadataIndex].C_Str(), "UpAxisSign") == 0)
				{
					assimpScene->mMetaData->Get<int32_t>(MetadataIndex, UpAxisSign);
				}
				if (strcmp(assimpScene->mMetaData->mKeys[MetadataIndex].C_Str(), "FrontAxis") == 0)
				{
					assimpScene->mMetaData->Get<int32_t>(MetadataIndex, FrontAxis);
				}
				if (strcmp(assimpScene->mMetaData->mKeys[MetadataIndex].C_Str(), "FrontAxisSign") == 0)
				{
					assimpScene->mMetaData->Get<int32_t>(MetadataIndex, FrontAxisSign);
				}
				if (strcmp(assimpScene->mMetaData->mKeys[MetadataIndex].C_Str(), "CoordAxis") == 0)
				{
					assimpScene->mMetaData->Get<int32_t>(MetadataIndex, CoordAxis);
				}
				if (strcmp(assimpScene->mMetaData->mKeys[MetadataIndex].C_Str(), "CoordAxisSign") == 0)
				{
					assimpScene->mMetaData->Get<int32_t>(MetadataIndex, CoordAxisSign);
				}
				if (strcmp(assimpScene->mMetaData->mKeys[MetadataIndex].C_Str(), "UnitScaleFactor") == 0)
				{
					assimpScene->mMetaData->Get<double>(MetadataIndex, UnitScaleFactor);
				}
			}

			aiVector3D upVec, forwardVec, rightVec;

			upVec[UpAxis] = UpAxisSign * (float)UnitScaleFactor;
			forwardVec[FrontAxis] = FrontAxisSign * (float)UnitScaleFactor;
			rightVec[CoordAxis] = CoordAxisSign * (float)UnitScaleFactor;

			aiMatrix4x4 mat(rightVec.x, rightVec.y, rightVec.z, 0.0f,
				upVec.x, upVec.y, upVec.z, 0.0f,
				forwardVec.x, forwardVec.y, forwardVec.z, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
			assimpScene->mRootNode->mTransformation = mat;
		}

		ExporterType exporterType = ExporterType::StaticMesh;

		if (assimpScene->HasMeshes())
		{
			if (assimpScene->HasAnimations())
			{
				std::cout << "[WARNING]\tThe model has both meshes and animations. The engine regards the file as a mesh data and the animations are ignored." << std::endl;
			}
			if (hasBones)
			{
				exporterType = ExporterType::RiggedMesh;
				std::cout << "[LOG]\tExported the resource as RiggedMesh" << std::endl;
			}
			else
			{
				exporterType = ExporterType::StaticMesh;
				std::cout << "[LOG]\tExported the resource as StaticMesh" << std::endl;
			}
		}
		else if (assimpScene->HasAnimations())
		{
			exporterType = ExporterType::AnimationClip;
			std::cout << "[LOG]\tExported the resource as AnimationClip" << std::endl;
		}
		else
		{
			std::cout << "[WARNING]\tThe model has no meshes or animations. Skipping export." << std::endl;
			continue;
		}

		auto& [ext, exporter] = g_exporters[static_cast<size_t>(exporterType)];
		std::filesystem::path outputPath = entry.path();
		outputPath.replace_extension(ext);
		exporter->Export(*assimpScene, outputPath);
	}

	Assimp::DefaultLogger::kill();
}