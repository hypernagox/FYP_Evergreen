#pragma once

#include "pch.h"
#include "resource_object.h"

namespace udsdx
{
	struct Submesh
	{
		std::string Name;
		UINT IndexCount = 0;
		UINT StartIndexLocation = 0;
		UINT BaseVertexLocation = 0;
	};

	class MeshBase : public ResourceObject
	{
	public:
		constexpr static DXGI_FORMAT INDEX_FORMAT = DXGI_FORMAT_R32_UINT;

	public:
		MeshBase();

	public:
		D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const;
		D3D12_INDEX_BUFFER_VIEW IndexBufferView() const;
		const std::vector<Submesh>& GetSubmeshes() const;
		const BoundingBox& GetBounds() const;

	public:
		template <typename TVertex>
		void CreateBuffers(const std::vector<TVertex>& vertices, const std::vector<UINT>& indices);
		void UploadBuffers(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);

		// We can free this memory after we finish upload to the GPU.
		void DisposeUploaders();

	protected:
		std::vector<Submesh> m_submeshes;

		UINT m_vertexByteStride = 0;
		UINT m_vertexBufferByteSize = 0;
		UINT m_indexBufferByteSize = 0;

		BoundingBox m_bounds;

		// System memory copies.  Use Blobs because the vertex/index format can be generic.
		// It is up to the client to cast appropriately.  
		ComPtr<ID3DBlob> m_vertexBufferCPU = nullptr;
		ComPtr<ID3DBlob> m_indexBufferCPU = nullptr;

		ComPtr<ID3D12Resource> m_vertexBufferGPU = nullptr;
		ComPtr<ID3D12Resource> m_indexBufferGPU = nullptr;

		ComPtr<ID3D12Resource> m_vertexBufferUploader = nullptr;
		ComPtr<ID3D12Resource> m_indexBufferUploader = nullptr;
	};

	template<typename TVertex>
	inline void MeshBase::CreateBuffers(const std::vector<TVertex>& vertices, const std::vector<UINT>& indices)
	{
		const UINT vbByteSize = (UINT)vertices.size() * sizeof(TVertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(UINT);

		m_vertexByteStride = sizeof(TVertex);
		m_vertexBufferByteSize = vbByteSize;
		m_indexBufferByteSize = ibByteSize;

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &m_vertexBufferCPU));
		CopyMemory(m_vertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &m_indexBufferCPU));
		CopyMemory(m_indexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);
	}
}