#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;

	Vertex();
	Vertex(const XMFLOAT3& position, const XMFLOAT2& uv, const XMFLOAT3& normal, const XMFLOAT3& tangent);
};

struct RiggedVertex
{
	XMFLOAT3 position;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	unsigned boneIndices;
	XMFLOAT4 boneWeights;

	RiggedVertex();
	RiggedVertex(const XMFLOAT3& position, const XMFLOAT2& uv, const XMFLOAT3& normal, const XMFLOAT3& tangent, const char boneIndices[4], const XMFLOAT4& boneWeights);
};