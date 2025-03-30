#include "pch.h"
#include "DropItemRenderer.h"

DropItemRenderer::DropItemRenderer(std::shared_ptr<udsdx::SceneObject> owner) : udsdx::Component(owner)
{
	m_rendererObject = std::make_shared<udsdx::SceneObject>();
	m_rendererObject->GetTransform()->SetLocalScale(Vector3::One * GET_DATA(float, "DropitemScale", "Value"));

	m_meshRenderer = m_rendererObject->AddComponent<udsdx::MeshRenderer>();
	m_meshRenderer->SetShader(INSTANCE(udsdx::Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl")));

	owner->AddChild(m_rendererObject);

	auto urd = std::uniform_real_distribution(0.0f, udsdx::PI2);
	auto dre = std::default_random_engine(std::clock());
	m_rotationOffset = urd(dre);
}

void DropItemRenderer::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	m_rendererObject->GetTransform()->SetLocalRotation(udsdx::Quaternion::CreateFromYawPitchRoll(time.totalTime * 2.0f + m_rotationOffset, XM_PIDIV4, 0.0f));
	m_rendererObject->GetTransform()->SetLocalPositionY(1.0f + sin(time.totalTime * 2.0f) * 0.1f);
}

void DropItemRenderer::SetDropItem(uint8_t item_id)
{
	std::string key = std::to_string(item_id);
	m_meshRenderer->SetMesh(INSTANCE(udsdx::Resource)->Load<udsdx::Mesh>(RESOURCE_PATH(GET_DATA(std::wstring, key, "DropitemResource"))));
	m_material = std::make_shared<udsdx::Material>();
	m_material->SetSourceTexture(INSTANCE(udsdx::Resource)->Load<udsdx::Texture>(RESOURCE_PATH(GET_DATA(std::wstring, key, "DropitemResourceDiffuse"))));
	m_meshRenderer->SetMaterial(m_material.get());
}
