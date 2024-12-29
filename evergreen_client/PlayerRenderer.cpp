#include "pch.h"
#include "PlayerRenderer.h"
#include "EntityMovement.h"
#include "ServerObject.h"

extern std::shared_ptr<SceneObject> g_heroObj;

PlayerRenderer::PlayerRenderer(const std::shared_ptr<SceneObject>& object) : Component(object)
{
	m_rendererObj = std::make_shared<SceneObject>();

	std::shared_ptr<SceneObject> pBody = std::make_shared<SceneObject>();

	auto shader = INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"color.hlsl"));

	m_transformBody = pBody->GetTransform();
	m_rendererObj->AddChild(pBody);

	m_renderer = pBody->AddComponent<RiggedMeshRenderer>();
	m_renderer->SetMesh(INSTANCE(Resource)->Load<udsdx::RiggedMesh>(RESOURCE_PATH(L"Zelda\\zelda.glb")));
	m_renderer->SetShader(shader);

	for (int i = 0; i < m_playerMaterials.size(); ++i)
	{
		m_playerMaterials[i] = std::make_shared<udsdx::Material>();
		m_renderer->SetMaterial(m_playerMaterials[i].get(), i);
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
	const bool flag = g_heroObj->GetComponent<ServerObject>()->GetObjID() == GetSceneObject()->GetComponent<ServerObject>()->GetObjID();
	if (INSTANCE(Input)->GetMouseLeftButtonDown() && m_attackTime <= 0.0f && flag)
	{
		m_attackTime = 1.0f;
	}
	const Vector3 velocity = GetComponent<EntityMovement>()->GetVelocity();
	// check if xz component of velocity is not zero
	float mag = Vector2(velocity.x, velocity.z).LengthSquared();
	if ((m_attackTime > 0.0f && flag) || m_attackTime ==1.f)
	{
		SetAnimation("Bip001|attack1|BaseLayer");
	}
	else if (mag > 10.0f)
	{
		SetAnimation("Bip001|run|BaseLayer");
	}
	else
	{
		if (!flag && m_attackTime < 0.0f) 
		{
			m_attackTime = 1.f;
			SetAnimation("Bip001|stand|BaseLayer");
		}
		else if(flag)
			SetAnimation("Bip001|stand|BaseLayer");
	}

	m_attackTime -= time.deltaTime;
}
