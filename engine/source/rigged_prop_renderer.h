#pragma once

#include "pch.h"
#include "mesh_renderer.h"

namespace udsdx
{
	class RiggedMeshRenderer;

	class RiggedPropRenderer : public MeshRenderer
	{
	public:
		void OnInitialize() override;
		void UpdateTransformCache() override;
		std::string_view GetBoneName() const;
		void SetBoneName(const std::string& boneName);
		void SetPropLocalTransform(const Matrix4x4& transform);

	protected:
		RiggedMeshRenderer* m_targetCache = nullptr;
		std::string m_boneName;
		Matrix4x4 m_propLocalTransform = Matrix4x4::Identity;
	};
}