#include "pch.h"
#include "vertex.h"

namespace udsdx
{
	Vertex::Vertex()
	{
		position = { 0.0f, 0.0f, 0.0f };
		uv = { 0.0f, 0.0f };
		normal = { 0.0f, 0.0f, 1.0f };
		tangent = { 0.0f, 1.0f, 0.0f };
	}

	Vertex::Vertex(const XMFLOAT3& position, const XMFLOAT2& uv, const XMFLOAT3& normal, const XMFLOAT3& tangent)
	{
		this->position = position;
		this->uv = uv;
		this->normal = normal;
		this->tangent = tangent;
	}

	RiggedVertex::RiggedVertex()
	{
		position = { 0.0f, 0.0f, 0.0f };
		uv = { 0.0f, 0.0f };
		normal = { 0.0f, 0.0f, 1.0f };
		tangent = { 0.0f, 1.0f, 0.0f };
		boneIndices = 0;
		boneWeights = { 0.0f, 0.0f, 0.0f, 0.0f };
	}

	RiggedVertex::RiggedVertex(const XMFLOAT3& position, const XMFLOAT2& uv, const XMFLOAT3& normal, const XMFLOAT3& tangent, const BYTE boneIndices[4], const XMFLOAT4& boneWeights)
	{
		this->position = position;
		this->uv = uv;
		this->normal = normal;
		this->tangent = tangent;
		this->boneIndices = boneIndices[0];
		this->boneIndices |= boneIndices[1] << 8;
		this->boneIndices |= boneIndices[2] << 16;
		this->boneIndices |= boneIndices[3] << 24;
		this->boneWeights = boneWeights;

	}
}