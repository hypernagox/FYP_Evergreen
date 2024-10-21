#include "pch.h"
#include "PlayerRenderer.h"

PlayerRenderer::PlayerRenderer(const std::shared_ptr<SceneObject>& object) : Component(object)
{
	m_rendererObj = std::make_shared<SceneObject>();

	std::shared_ptr<SceneObject> pBody = std::make_shared<SceneObject>();

	auto shader = INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl"));

	m_transformBody = pBody->GetTransform();
	m_rendererObj->AddChild(pBody);

	auto pBodyMesh = pBody->AddComponent<RiggedMeshRenderer>();
	pBodyMesh->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"untitled.glb")));
	pBodyMesh->SetShader(shader);
	pBodyMesh->SetAnimation("Bip001|idle|BaseLayer");

	for (size_t i = 0; i < m_playerMaterials.size(); ++i)
	{
		m_playerMaterials[i] = std::make_shared<udsdx::Material>();
		pBodyMesh->SetMaterial(m_playerMaterials[i].get(), i);
	}
	m_playerMaterials[2]->SetMainTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_eye_BaseColor.png")));
	m_playerMaterials[3]->SetMainTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_face_BaseColor.png")));
	m_playerMaterials[0]->SetMainTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_body_BaseColor.png")));
	m_playerMaterials[1]->SetMainTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Zelda\\zelda_hair_BaseColor.png")));

	auto sceneObject = GetSceneObject();
	sceneObject->AddChild(m_rendererObj);

	m_rendererObj->GetTransform()->SetLocalScale(Vector3::One / 32);
}

PlayerRenderer::~PlayerRenderer()
{
}

void PlayerRenderer::Update(const Time& time, Scene& scene)
{
}