#include "pch.h"
#include "MonsterRenderer.h"
#include "GizmoSphereRenderer.h"

using namespace udsdx;

// TODO: MonsterRenderer는 몬스터 타입에 따라 다양화 시키는게 좋을 것 같다. 추후 상속이나 참조를 통해 속성을 변경할 수 있어야 한다.

void MonsterRenderer::OnInitialize()
{
	m_rendererObject = udsdx::SceneObject::MakeShared();

	auto renderer = m_rendererObject->AddComponent<udsdx::MeshRenderer>();
	renderer->SetMesh(INSTANCE(Resource)->Load<udsdx::Mesh>(RESOURCE_PATH(L"Sphere.yms")));
	renderer->SetMaterial(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"colornotex.hlsl")));

	GetSceneObject()->AddChild(m_rendererObject);
}
