#include "pch.h"
#include "DropItemRenderer.h"
#include "ServerObjectMgr.h"
#include "ServerObject.h"
#include "SphereParticleEmitter.h"

std::default_random_engine DropItemRenderer::randomEngine{};

using namespace udsdx;

void DropItemRenderer::OnInitialize()
{
	m_rendererObject = udsdx::SceneObject::MakeShared();
	m_meshRenderer = m_rendererObject->AddComponent<udsdx::MeshRenderer>();

	GetSceneObject()->AddChild(m_rendererObject);

	auto urd = std::uniform_real_distribution(0.0f, udsdx::PI2);
	m_rotationOffset = urd(randomEngine);

	auto ringParticleObj = SceneObject::MakeShared();
	ringParticleObj->GetTransform()->SetLocalPosition(Vector3::Up * 0.5f);
	{
		auto particleEmitter = ringParticleObj->AddComponent<SphereParticleEmitter>();
		particleEmitter->SetColor(Vector3(1.0f, 1.0f, 0.5f) * 2.0f);
		particleEmitter->SetDrawCount(1);
		particleEmitter->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"particle\\aura_ray.png")));
		particleEmitter->GetEmitterParameter().LifeTimeMin = 4.0f;
		particleEmitter->GetEmitterParameter().LifeTimeMax = 4.0f;
		particleEmitter->GetEmitterParameter().RotationMin = -PIDIV4;
		particleEmitter->GetEmitterParameter().RotationMax = PIDIV4;
		particleEmitter->GetEmitterParameter().RotationLifeExp = 1.0f;
		particleEmitter->GetEmitterParameter().SizeMin = Vector2::One * 1.0f;
		particleEmitter->GetEmitterParameter().SizeMax = Vector2::One * 1.0f;
		particleEmitter->GetEmitterParameter().SizeLifeExp = Vector2::One * 0.25f;
		particleEmitter->GetEmitterParameter().AlphaLifeExp = -1.0f;
		particleEmitter->SetEmitLoop(true);
		particleEmitter->SetAutoDestroy(false);
		particleEmitter->Play();
	}
	GetSceneObject()->AddChild(ringParticleObj);
}

void DropItemRenderer::OnActive()
{
	auto listener = ServerObjectMgr::GetInst()->GetMainHero();
	if (nullptr != listener)
	{
		m_appearSound = INSTANCE(udsdx::Resource)->Load<udsdx::AudioClip>(RESOURCE_PATH(L"audio\\item_drop.wav"))->CreateInstance3D(GetTransform()->GetLocalPosition());
	}
}

void DropItemRenderer::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	float scale = GET_DATA(float,"GlobalValues", "DropitemScale", "Value");
	m_rendererObject->GetTransform()->SetLocalRotation(udsdx::Quaternion::CreateFromYawPitchRoll(time.totalTime * 2.0f + m_rotationOffset, XM_PIDIV4, 0.0f));
	m_rendererObject->GetTransform()->SetLocalPositionY(scale / 8.0f + sin(time.totalTime * 2.0f) * scale / 80.0f);
	m_scaleFactor = std::lerp(m_scaleFactor, scale, time.deltaTime * 8.0f);
	m_rendererObject->GetTransform()->SetLocalScale(Vector3::One * m_scaleFactor);
}

void DropItemRenderer::SetDropItem(uint8_t item_id)
{
	const auto& key = DATA_TABLE->GetItemName(item_id);
	const auto texture = INSTANCE(udsdx::Resource)->Load<udsdx::Texture>(RESOURCE_PATH(GET_DATA(std::wstring, "Item", key, "DropitemResourceDiffuse")));
	m_meshRenderer->SetMesh(INSTANCE(udsdx::Resource)->Load<udsdx::Mesh>(RESOURCE_PATH(GET_DATA(std::wstring,"Item", key, "DropitemResource"))));
	m_meshRenderer->SetMaterial(udsdx::Material(INSTANCE(udsdx::Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"colorhighlight.hlsl")), texture));
}
