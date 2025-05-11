#include "pch.h"
#include "MonsterRenderer.h"

using namespace udsdx;

// TODO: MonsterRenderer는 몬스터 타입에 따라 다양화 시키는게 좋을 것 같다. 추후 상속이나 참조를 통해 속성을 변경할 수 있어야 한다.

MonsterRenderer::MonsterRenderer(std::shared_ptr<udsdx::SceneObject> owner) : Component(owner)
{
	m_rendererObject = std::make_shared<udsdx::SceneObject>();
	m_rendererObject->GetTransform()->SetLocalScale(Vector3::One * 0.015f);

	auto renderer = m_rendererObject->AddComponent<udsdx::MeshRenderer>();
	renderer->SetMesh(INSTANCE(Resource)->Load<udsdx::Mesh>(RESOURCE_PATH(L"sheep\\sheep_max.obj")));
	renderer->SetShader(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl")));

	m_materials[0] = std::make_shared<udsdx::Material>();
	m_materials[0]->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"sheep\\sheep_BaseColor.png")));
	m_materials[1] = std::make_shared<udsdx::Material>();
	m_materials[1]->SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"goblin\\Goblin(Wizard)\\goblin_notex_DefaultMaterial_BaseColor.png")));

	renderer->SetMaterial(m_materials[0].get(), 0);
	renderer->SetMaterial(m_materials[1].get(), 1);

	owner->AddChild(m_rendererObject);
}

void MonsterRenderer::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
}
