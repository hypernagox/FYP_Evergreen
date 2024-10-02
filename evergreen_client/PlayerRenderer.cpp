#include "pch.h"
#include "PlayerRenderer.h"

PlayerRenderer::PlayerRenderer(const std::shared_ptr<SceneObject>& object) : Component(object)
{
	m_rendererObj = std::make_shared<SceneObject>();

	std::shared_ptr<SceneObject> pBody = std::make_shared<SceneObject>();

	auto shader = INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl"));
	m_playerMaterial = std::make_shared<udsdx::Material>();
	m_playerMaterial->SetMainTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"Sprite-0001.png")));

	m_transformBody = pBody->GetTransform();
	m_rendererObj->AddChild(pBody);
	m_rendererObj->GetTransform()->SetLocalPositionY(2.0f);

	auto pBodyMesh = pBody->AddComponent<MeshRenderer>();
	pBodyMesh->SetMesh(INSTANCE(Resource)->Load<udsdx::Mesh>(RESOURCE_PATH(L"char_sample.fbx")));
	pBodyMesh->SetShader(shader);
	pBodyMesh->SetMaterial(m_playerMaterial.get());

	m_transformBody->SetLocalScale(Vector3::One * 0.05f);
	m_transformBody->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(PIDIV2, 0.0f, 0.0f));

	auto sceneObject = GetSceneObject();
	sceneObject->AddChild(m_rendererObj);

	m_rendererObj->GetTransform()->SetLocalRotation(Quaternion::Identity);
}

PlayerRenderer::~PlayerRenderer()
{
}

void PlayerRenderer::Update(const Time& time, Scene& scene)
{
}
