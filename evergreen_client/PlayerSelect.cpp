#include "pch.h"
#include "PlayerSelect.h"
#include "PlayerRenderer.h"

using namespace udsdx;

void PlayerSelect::OnInitialize()
{
	m_targetPosition = GetSceneObject()->GetTransform()->GetLocalPosition();
	m_targetPosition.y = 256.0f;

	m_characterObjects[0] = std::make_shared<SceneObject>();
	{
		auto playerRenderer = m_characterObjects[0]->AddComponent<PlayerRenderer>();
		playerRenderer->InitializeWarrior();
		playerRenderer->SetPlayerWeapon(DATA_TABLE->GetWeaponIDInt("Master Sword"));
		playerRenderer->SetPlayerArmor(DATA_TABLE->GetArmorIDInt("Zelda Armor 1"));
	}
	GetSceneObject()->AddChild(m_characterObjects[0]);

	m_characterObjects[1] = std::make_shared<SceneObject>();
	{
		auto playerRenderer = m_characterObjects[1]->AddComponent<PlayerRenderer>();
		playerRenderer->InitializePriest();
		playerRenderer->SetPlayerWeapon(DATA_TABLE->GetWeaponIDInt("Staff Priest"));
		playerRenderer->SetPlayerArmor(DATA_TABLE->GetArmorIDInt("Priest Armor 1"));
	}
	GetSceneObject()->AddChild(m_characterObjects[1]);

	m_characterObjects[2] = std::make_shared<SceneObject>();
	{
		auto playerRenderer = m_characterObjects[2]->AddComponent<PlayerRenderer>();
		playerRenderer->InitializePriest();
		playerRenderer->SetPlayerWeapon(DATA_TABLE->GetWeaponIDInt("Bow"));
		playerRenderer->SetPlayerArmor(DATA_TABLE->GetArmorIDInt("Priest Armor 1"));
	}
	GetSceneObject()->AddChild(m_characterObjects[2]);

	SetShowingCharacter(0);
}

void PlayerSelect::Update(const Time& time, Scene& scene)
{
	targetRotaitonPivot = udsdx::LerpAngleRadian(targetRotaitonPivot, m_showingCharacterIndex * 2.0f * PI / m_characterObjects.size(), time.deltaTime * 8.0f);

	Vector3 lastPosition = GetTransform()->GetLocalPosition();
	Vector3 currentPosition = Vector3::Lerp(lastPosition, m_targetPosition, time.deltaTime * 4.0f);
	GetTransform()->SetLocalPosition(currentPosition);

	Quaternion lastRotation = GetTransform()->GetLocalRotation();
	Quaternion currentRotation = Quaternion::Slerp(lastRotation, m_targetRotation, time.deltaTime * 4.0f);
	GetTransform()->SetLocalRotation(currentRotation);

	for (size_t i = 0; i < m_characterObjects.size(); ++i)
	{
		float angle = i * 2.0f * PI / m_characterObjects.size() - targetRotaitonPivot;
		Vector3 characterPosition = Vector3(sinf(angle), -0.75f, -cosf(angle)) * 5.0f;
		characterPosition += Vector3::Backward * 20.0f;
		m_characterObjects[i]->GetTransform()->SetLocalPosition(characterPosition);
		m_characterObjects[i]->GetTransform()->SetLocalRotation(Quaternion::CreateFromYawPitchRoll(-angle, 0.0f, 0.0f));
		m_characterObjects[i]->GetTransform()->SetLocalScale(Vector3::One * 5.0f);
	}
}

void PlayerSelect::SetShowingCharacter(unsigned int characterIndex)
{
	m_showingCharacterIndex = characterIndex;

	for (size_t i = 0; i < m_characterObjects.size(); ++i)
	{
		PlayerRenderer* renderer = m_characterObjects[i]->GetComponent<PlayerRenderer>();
		if (renderer == nullptr)
		{
			continue;
		}
		if (i == characterIndex)
		{
			renderer->SetAnimation(INSTANCE(Resource)->Load<AnimationClip>(RESOURCE_PATH(L"Zelda\\stand_arguing.yac")), true, true);
		}
		else
		{
			renderer->SetAnimation(INSTANCE(Resource)->Load<AnimationClip>(RESOURCE_PATH(L"Zelda\\zelda_stand.yac")), true, false);
		}
	}
}
