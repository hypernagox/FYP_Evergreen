#pragma once

#include "pch.h"

namespace udsdx
{
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT2 uv;
		XMFLOAT3 normal;
		XMFLOAT3 tangent;

		// Input Layout:
		// used to make vertex propertices associate with the semantics of shaders
		constexpr static D3D12_INPUT_ELEMENT_DESC DescriptionTable[] = {
			// Vertex::position
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			// Vertex::texcoord
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			// Vertex::normal
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			// Vertex::tangent
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
		constexpr static UINT DescriptionTableSize = sizeof(DescriptionTable) / sizeof(D3D12_INPUT_ELEMENT_DESC);

		Vertex();
		Vertex(const XMFLOAT3& position, const XMFLOAT2& uv, const XMFLOAT3& normal, const XMFLOAT3& tangent);
	};

	struct RiggedVertex
	{
		XMFLOAT3 position;
		XMFLOAT2 uv;
		XMFLOAT3 normal;
		XMFLOAT3 tangent;
		uint32_t boneIndices;
		XMFLOAT4 boneWeights;

		// Input Layout:
		// used to make vertex propertices associate with the semantics of shaders
		constexpr static D3D12_INPUT_ELEMENT_DESC DescriptionTable[] = {
			// RiggedVertex::position
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			// RiggedVertex::texcoord
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			// RiggedVertex::normal
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			// RiggedVertex::tangent
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			// RiggedVertex::boneIndices
			{ "BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			// RiggedVertex::boneWeights
			{ "BONEWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
		constexpr static UINT DescriptionTableSize = sizeof(DescriptionTable) / sizeof(D3D12_INPUT_ELEMENT_DESC);

		RiggedVertex();
		RiggedVertex(const XMFLOAT3& position, const XMFLOAT2& uv, const XMFLOAT3& normal, const XMFLOAT3& tangent, const BYTE boneIndices[4], const XMFLOAT4& boneWeights);
	};
}