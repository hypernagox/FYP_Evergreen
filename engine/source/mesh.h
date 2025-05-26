#pragma once

#include "pch.h"
#include "mesh_base.h"
#include "vertex.h"

namespace udsdx
{
	class Mesh : public MeshBase
	{
	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<UINT> indices);
		Mesh(const std::filesystem::path& resourcePath);
	};
}