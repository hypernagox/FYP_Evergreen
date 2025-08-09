#include "pch.h"
#include "CatapultProjectile.h"

using namespace udsdx;

void CatapultProjectile::OnInitialize()
{
	m_projectileObj = SceneObject::MakeShared();
	m_projectileObj->GetTransform()->SetLocalScale(0.01f);
	auto renderer = m_projectileObj->AddComponent<MeshRenderer>();

	udsdx::Material mat = udsdx::Material(INSTANCE(Resource)->Load<udsdx::Shader>(RESOURCE_PATH(L"colornormal.hlsl")));
	mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\Rocks\\Rock06\\Rock06_baseTexBaked.png")), 0);
	mat.SetSourceTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"environment\\Maps\\Rocks\\Rock06\\Rock06_normals.png")), 1);

	renderer->SetMaterial(mat);
	renderer->SetMesh(INSTANCE(Resource)->Load<udsdx::Mesh>(RESOURCE_PATH(L"environment\\Rocks\\Rock006.yms")));

	GetSceneObject()->AddChild(m_projectileObj);
	m_projectileObj->SetActive(false);
}

void CatapultProjectile::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	m_shootElapsedTime += time.deltaTime;
	if (m_shootElapsedTime >= m_duration)
	{
		m_projectileObj->SetActive(false);
		return;
	}
	Vector3 targetPosition = m_targetObj->GetTransform()->GetWorldPosition();
	Vector3 currentPosition = Vector3::Lerp(m_fromPosition, targetPosition, m_shootElapsedTime / m_duration);
	currentPosition += Vector3::Up * (m_shootElapsedTime * (m_duration - m_shootElapsedTime) * 32.0f); // Parabolic arc effect
	m_projectileObj->GetTransform()->SetLocalPosition(currentPosition);

	Vector3 right = Vector3::Up.Cross(targetPosition - m_fromPosition);
	right.Normalize();
	m_projectileObj->GetTransform()->SetLocalRotation(Quaternion::CreateFromAxisAngle(right, m_shootElapsedTime * 10.0f));
}

void CatapultProjectile::ShootProjectile(const Vector3& fromPosition, const std::shared_ptr<SceneObject>& targetObj, float duration)
{
	m_duration = duration;
	m_shootElapsedTime = 0.0f;
	m_targetObj = targetObj;
	m_fromPosition = fromPosition;
	m_projectileObj->SetActive(true);
}