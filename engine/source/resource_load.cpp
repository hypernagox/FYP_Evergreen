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
#include "font.h"

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
		m_loaders.emplace(L"font", std::make_unique<FontLoader>(device, commandList));
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
		m_extensionDictionary.emplace(L".yms", L"model");
		m_extensionDictionary.emplace(L".yrms", L"model");
		m_extensionDictionary.emplace(L".yac", L"model");
		m_extensionDictionary.emplace(L".hlsl", L"shader");
		m_extensionDictionary.emplace(L".wav", L"audio");
		m_extensionDictionary.emplace(L".spritefont", L"font");
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

		std::unique_ptr<ResourceObject> ret;
		if (pathString.extension().string() == ".yms")
		{
			std::unique_ptr<MeshBase> mesh = nullptr;
			mesh = std::make_unique<Mesh>(pathString);
			mesh->UploadBuffers(m_device, m_commandList);
			ret = std::move(mesh);
		}
		else if (pathString.extension().string() == ".yrms")
		{
			std::unique_ptr<MeshBase> mesh = nullptr;
			mesh = std::make_unique<RiggedMesh>(pathString);
			mesh->UploadBuffers(m_device, m_commandList);
			ret = std::move(mesh);
		}
		else if (pathString.extension().string() == ".yac")
		{
			ret = std::make_unique<AnimationClip>(pathString);
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
		shader->BuildDeferredPipelineState(m_device, m_rootSignature);
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

	FontLoader::FontLoader(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) : ResourceLoader(device, commandList)
	{

	}

	std::unique_ptr<ResourceObject> FontLoader::Load(std::wstring_view path)
	{
		return std::make_unique<Font>(path);
	}
}