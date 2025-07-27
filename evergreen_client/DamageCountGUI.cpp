#include "pch.h"
#include "DamageCountGUI.h"
#include "GameScene.h"
#include "WorldSpaceGUI.h"

using namespace udsdx;

std::default_random_engine DamageCountGUI::randomEngine{};
// TODO: 연타로직 (연출만) 여기에 있음
void DamageCountGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	m_timer.Update(DT);
	for (auto& [life, countObj, textObj, position, velocity] : m_countObjects)
	{
		// velocity.y -= 9.8f * time.deltaTime;
		position += velocity * time.deltaTime;
		life -= time.deltaTime;

		auto urd = std::uniform_real_distribution(-1.0f, 1.0f);
		auto uid = std::uniform_int_distribution<int>(0, 3);
		Vector3 offset = Vector3(urd(randomEngine), urd(randomEngine), 0.0f) * 10.0f * life;
		Vector3 colors[] = {
			Vector3(1.0f, 1.0f, 1.0f), // White
			Vector3(1.0f, 0.0f, 0.0f), // Red
			Vector3(1.0f, 0.5f, 0.0f), // Orange
			Vector3(1.0f, 1.0f, 0.0f), // Yellow
		};

		countObj->GetComponent<WorldSpaceGUI>()->SetWorldOffset(position);
		countObj->GetComponent<WorldSpaceGUI>()->SetScreenOffset(offset);
		textObj->GetTransform()->SetLocalScale(1.0f + std::pow(life, 8.0f) * 3.0f);
		Vector3 color = life > 0.5f ? colors[uid(randomEngine)] : colors[0];
		textObj->GetComponent<GUIText>()->SetColor(Vector4(color.x, color.y, color.z, std::pow(life, 0.5f)));
	}
	while (!m_countObjects.empty())
	{
		auto& [life, countObj, textObj, position, velocity] = m_countObjects.front();
		if (life <= 0.0f)
		{
			countObj->RemoveFromParent();
			m_countObjects.pop_front();
		}
		else
			break;
	}
}

void DamageCountGUI::AddCountObject(const Vector3& targetPos, unsigned int damageCount, const int hit_count)
{
	constexpr const float DMG_DELTA_STEP = 0.1f;
	for (int i = 0; i < hit_count; ++i)
	{
		m_timer.RegisterEvent((float)i * DMG_DELTA_STEP, [=]() {
			std::shared_ptr<SceneObject> countObj = SceneObject::MakeShared();
			std::shared_ptr<SceneObject> textObj = SceneObject::MakeShared();

			auto urd = std::uniform_real_distribution(-1.0f, 1.0f);
			Vector3 velocity = Vector3::Up * 0.5f;
			Vector3 offset = Vector3(urd(randomEngine), urd(randomEngine) + 1.0f, urd(randomEngine));

			auto panelWorldTransform = countObj->AddComponent<WorldSpaceGUI>();
			panelWorldTransform->SetTargetObject(textObj);
			panelWorldTransform->SetViewFar(30.0f);
			panelWorldTransform->SetWorldOffset(offset);

			auto nameRenderer = textObj->AddComponent<GUIText>();
			nameRenderer->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"sansman.spritefont")));
			nameRenderer->SetRaycastTarget(false);
			nameRenderer->SetText(std::to_wstring(damageCount / hit_count + 10 + rand() % 10));

			countObj->AddChild(textObj);
			GetSceneObject()->AddChild(countObj);

			m_countObjects.emplace_back(1.0f, countObj, textObj, targetPos + offset, velocity);
			m_soundInstance = INSTANCE(Resource)->Load<udsdx::AudioClip>(RESOURCE_PATH(L"audio\\hit_light.wav"))->CreateInstance();
			m_soundInstance->SetVolume(0.5f);
			m_soundInstance->Play();

			});
	}
}
