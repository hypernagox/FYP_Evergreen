#include "pch.h"
#include "resource_load.h"
#include "texture.h"
#include "mesh.h"
#include "rigged_mesh.h"
#include "animation_clip.h"
#include "shader.h"
#include "debug_console.h"
#include "audio.h"
#include "audio_clip.h"

// Assimp Library
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace udsdx
{
	Resource::Resource()
	{

	}

	Resource::~Resource()
	{

	}

	void Resource::Initialize(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* rootSignature)
	{ ZoneScoped;
		InitializeLoaders(device, commandQueue, commandList, rootSignature);
		InitializeExtensionDictionary();
		InitializeIgnoreFiles();

		DebugConsole::Log("Registering resources...");

		// if the directory does not exist, this must be an error
		assert(std::filesystem::exists(m_resourceRootPath));

		for (const auto& directory : std::filesystem::recursive_directory_iterator(m_resourceRootPath))
		{
			// if the file is not a regular file(e.g. if it is a directory), skip it
			if (!directory.is_regular_file())
			{
				continue;
			}

			std::wstring path = directory.path().wstring();
			std::wstring filename = directory.path().filename().wstring();
			std::wstring suffix = directory.path().extension().wstring();

			std::transform(path.begin(), path.end(), path.begin(), ::tolower);
			std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
			std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);

			// if the file is in the ignore list, skip it
			if (m_ignoreFiles.find(filename) != m_ignoreFiles.end())
			{
				continue;
			}

			std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);

			// if the file extension is not in the dictionary, skip it
			auto iter = m_extensionDictionary.find(suffix);
			if (iter == m_extensionDictionary.end())
			{
				continue;
			}

			// if the loader is not found, skip it
			auto loader_iter = m_loaders.find(iter->second);
			if (loader_iter == m_loaders.end())
			{
				continue;
			}

			DebugConsole::Log(L"> " + iter->second + L": " + path);
			m_resources.emplace(path, loader_iter->second->Load(path));
		}
		std::cout << std::endl;
	}

	void Resource::InitializeLoaders(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* rootSignature)
	{
		m_loaders.emplace(L"texture", std::make_unique<TextureLoader>(device, commandQueue, commandList));
		m_loaders.emplace(L"model", std::make_unique<ModelLoader>(device, commandList));
		m_loaders.emplace(L"shader", std::make_unique<ShaderLoader>(device, commandList, rootSignature));
		m_loaders.emplace(L"audio", std::make_unique<AudioClipLoader>(device, commandList));
	}

	void Resource::SetResourceRootPath(std::wstring_view path)
	{
		m_resourceRootPath = path;
	}

	void Resource::InitializeExtensionDictionary()
	{
		m_extensionDictionary.emplace(L".png", L"texture");
		m_extensionDictionary.emplace(L".jpg", L"texture");
		m_extensionDictionary.emplace(L".jpeg", L"texture");
		m_extensionDictionary.emplace(L".bmp", L"texture");
		m_extensionDictionary.emplace(L".tif", L"texture");
		m_extensionDictionary.emplace(L".tga", L"texture");
		m_extensionDictionary.emplace(L".obj", L"model");
		m_extensionDictionary.emplace(L".fbx", L"model");
		m_extensionDictionary.emplace(L".dae", L"model");
		m_extensionDictionary.emplace(L".glb", L"model");
		m_extensionDictionary.emplace(L".gltf", L"model");
		m_extensionDictionary.emplace(L".hlsl", L"shader");
		m_extensionDictionary.emplace(L".wav", L"audio");
	}

	void Resource::InitializeIgnoreFiles()
	{
		m_ignoreFiles.insert(L"common.hlsl");
	}

	ResourceLoader::ResourceLoader(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) : m_device(device), m_commandList(commandList)
	{
	}

	ResourceLoader::~ResourceLoader()
	{
	}

	TextureLoader::TextureLoader(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12GraphicsCommandList* commandList) : ResourceLoader(device, commandList), m_commandQueue(commandQueue)
	{

	}

	std::unique_ptr<ResourceObject> TextureLoader::Load(std::wstring_view path)
	{ ZoneScoped;
		auto texture = std::make_unique<Texture>(path, m_device, m_commandList);
		return texture;
	}

	ModelLoader::ModelLoader(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) : ResourceLoader(device, commandList)
	{
	}

	std::unique_ptr<ResourceObject> ModelLoader::Load(std::wstring_view path)
	{ ZoneScoped;
		std::filesystem::path pathString(path);

		// Load the model using Assimp
		Assimp::Importer importer;
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
		auto assimpScene = importer.ReadFile(
			pathString.string(),
			aiProcess_ConvertToLeftHanded | aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_LimitBoneWeights
		);

		assert(assimpScene != nullptr);

		bool hasBones = false;
		for (UINT i = 0; i < assimpScene->mNumMeshes && !hasBones; ++i)
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

		std::unique_ptr<ResourceObject> ret;
		if (assimpScene->HasMeshes())
		{
			if (assimpScene->HasAnimations())
			{
				DebugConsole::LogWarning("\tThe model has both meshes and animations. The engine regards the file as a mesh data and the animations are ignored.");
			}
			std::unique_ptr<MeshBase> mesh = nullptr;
			if (hasBones)
			{
				mesh = std::make_unique<RiggedMesh>(*assimpScene);
				mesh->UploadBuffers(m_device, m_commandList);
				DebugConsole::Log("\tRegistered the resource as RiggedMesh");
			}
			else
			{
				mesh = std::make_unique<Mesh>(*assimpScene);
				mesh->UploadBuffers(m_device, m_commandList);
				DebugConsole::Log("\tRegistered the resource as Mesh");
			}
			ret = std::move(mesh);
		}
		else if (assimpScene->HasAnimations())
		{
			ret = std::make_unique<AnimationClip>(*assimpScene);
			DebugConsole::Log("\tRegistered the resource as AnimationClip");
		}

		return ret;
	}

	ShaderLoader::ShaderLoader(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* rootSignature) : ResourceLoader(device, commandList), m_rootSignature(rootSignature)
	{
	}

	std::unique_ptr<ResourceObject> ShaderLoader::Load(std::wstring_view path)
	{ ZoneScoped;
		auto shader = std::make_unique<Shader>(path);
		shader->BuildPipelineState(m_device, m_rootSignature);
		return shader;
	}

	AudioClipLoader::AudioClipLoader(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) : ResourceLoader(device, commandList)
	{
		m_audioEngine = INSTANCE(Audio)->GetAudioEngine();
	}

	std::unique_ptr<ResourceObject> AudioClipLoader::Load(std::wstring_view path)
	{ ZoneScoped;
		return std::make_unique<AudioClip>(path, m_audioEngine);
	}
}