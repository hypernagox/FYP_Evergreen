#pragma once

#include "pch.h"
#include "mesh_base.h"
#include "vertex.h"

#include <assimp/scene.h>

namespace udsdx
{
	class Mesh : public MeshBase
	{
	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<UINT> indices);
		Mesh(const aiScene& assimpScene, const Matrix4x4& preMultiplication);
	};
}