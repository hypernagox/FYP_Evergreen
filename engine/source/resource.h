#pragma once

#include "pch.h"

namespace udsdx
{
	class ResourceObject;
	class ResourceLoader
	{
	protected:
		ID3D12Device* m_device;
		ID3D12GraphicsCommandList* m_commandList;

	public:
		ResourceLoader(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
		~ResourceLoader();

		virtual std::unique_ptr<ResourceObject> Load(std::wstring_view path) = 0;
	};

	class TextureLoader : public ResourceLoader
	{
	protected:
		ID3D12CommandQueue* m_commandQueue;

	public:
		TextureLoader(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12GraphicsCommandList* commandList);

		std::unique_ptr<ResourceObject> Load(std::wstring_view path) override;
	};

	class ModelLoader : public ResourceLoader
	{
	public:
		ModelLoader(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

		std::unique_ptr<ResourceObject> Load(std::wstring_view path) override;
	};

	class ShaderLoader : public ResourceLoader
	{
	protected:
		ID3D12RootSignature* m_rootSignature;

	public:
		ShaderLoader(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* rootSignature);

		std::unique_ptr<ResourceObject> Load(std::wstring_view path) override;
	};

	class AudioClipLoader : public ResourceLoader
	{
	protected:
		AudioEngine* m_audioEngine;

	public:
		AudioClipLoader(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

		std::unique_ptr<ResourceObject> Load(std::wstring_view path) override;
	};

	class Resource
	{
	private:
		std::unordered_map<std::wstring, std::unique_ptr<ResourceObject>> m_resources;

		std::unordered_map<std::wstring, std::unique_ptr<ResourceLoader>> m_loaders;
		std::unordered_map<std::wstring, std::wstring> m_extensionDictionary;
		std::unordered_set<std::wstring> m_ignoreFiles;

	public:
		Resource();
		~Resource();

		void Initialize(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* rootSignature);

	private:
		void InitializeLoaders(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12GraphicsCommandList* commandList, ID3D12RootSignature* rootSignature);
		void InitializeExtensionDictionary();
		void InitializeIgnoreFiles();

	public:
		template <typename T>
		T* Load(std::wstring_view path);
		template <typename T>
		std::vector<T*> LoadAll();
	};

	template<typename T>
	inline T* Resource::Load(std::wstring_view path)
	{
		auto iter = m_resources.find(path.data());
		if (iter == m_resources.end())
		{
			return nullptr;
		}
		return dynamic_cast<T*>(iter->second.get());
	}

	template<typename T>
	inline std::vector<T*> Resource::LoadAll()
	{
		std::vector<T*> ret;
		for (auto& resource : m_resources)
		{
			auto casted = dynamic_cast<T*>(resource.second.get());
			if (casted != nullptr)
			{
				ret.push_back(casted);
			}
		}
		return ret;
	}
}