#include "DropItemAcquireRenderer.h"
#include "GameScene.h"

using namespace udsdx;

void DropItemAcquireRenderer::OnInitialize()
{
	m_rendererObject = SceneObject::MakeShared();
	m_meshRenderer = m_rendererObject->AddComponent<MeshRenderer>();

	GetSceneObject()->AddChild(m_rendererObject);
}

void DropItemAcquireRenderer::Initialize(MeshRenderer* source)
{
	Transform* sourceTransform = source->GetSceneObject()->GetTransform();
	m_fromPosition = sourceTransform->GetWorldPosition();
	m_meshRenderer->SetMesh(source->GetMesh());
	m_meshRenderer->SetMaterial(source->GetMaterial());
	GetSceneObject()->GetTransform()->SetLocalPosition(m_fromPosition);
	GetSceneObject()->GetTransform()->SetLocalScale(sourceTransform->GetLocalScale());
	m_meshRenderer->GetTransform()->SetLocalRotation(sourceTransform->GetWorldRotation());
}

void DropItemAcquireRenderer::Begin()
{
	if (Scene* scene = GetSceneObject()->GetScene())
	{
		if (GameScene* gameScene = dynamic_cast<GameScene*>(scene))
		{
			m_heroObject = gameScene->GetHeroObject();
		}
	}
}

void DropItemAcquireRenderer::Update(const Time& time, Scene& scene)
{
	m_acquireFactor += time.deltaTime / 0.5f;
	if (m_acquireFactor >= 1.0f)
	{
		GetSceneObject()->RemoveFromParent();
		return;
	}

	Vector3 lerpPosition = Vector3::Lerp(m_fromPosition, m_heroObject->GetTransform()->GetLocalPosition(), m_acquireFactor);
	float yOffset =  m_acquireFactor * (1.0f - m_acquireFactor) * 10.0f;
	GetTransform()->SetLocalPosition(lerpPosition + Vector3::Up * yOffset);
	GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(m_acquireFactor * 10.0f, 0.0f, 0.0f));
	m_rendererObject->GetTransform()->SetLocalScale(std::pow(1.0f - m_acquireFactor, 0.5f));
}
