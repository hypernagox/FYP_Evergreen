#include "pch.h"
#include "DamageCountGUI.h"
#include "GameScene.h"

using namespace udsdx;

std::default_random_engine DamageCountGUI::randomEngine{};
// TODO: 연타로직 (연출만) 여기에 있음
void DamageCountGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	auto gameScene = dynamic_cast<GameScene*>(&scene);
	if (gameScene != nullptr)
	{
		auto camera = gameScene->GetMainCamera();
		m_timer.Update(DT);
		for (auto& [life, countObj, targetPos, velocity] : m_countObjects)
		{
			// velocity.y -= 9.8f * time.deltaTime;
			targetPos += velocity * time.deltaTime;
			life -= time.deltaTime;

			Vector3 viewPos = Vector3::Transform(targetPos, camera->GetViewMatrix());
			if (viewPos.z > 0.0f && viewPos.z < 16.0f)
			{
				countObj->SetActive(true);
				float aspectRatio = INSTANCE(Core)->GetAspectRatio();
				Vector3 screenPos = Vector3::Transform(viewPos, camera->GetProjMatrix(aspectRatio));
				screenPos.x *= GUIElement::RefScreenSize.y * aspectRatio * 0.5f;
				screenPos.y *= GUIElement::RefScreenSize.y * 0.5f;

				auto urd = std::uniform_real_distribution(-1.0f, 1.0f);
				auto uid = std::uniform_int_distribution<int>(0, 3);
				Vector3 offset = Vector3(urd(randomEngine), urd(randomEngine), 0.0f) * 10.0f * life;
				Vector3 colors[] = {
					Vector3(1.0f, 1.0f, 1.0f), // White
					Vector3(1.0f, 0.0f, 0.0f), // Red
					Vector3(1.0f, 0.5f, 0.0f), // Orange
					Vector3(1.0f, 1.0f, 0.0f), // Yellow
				};
				Vector3 color = life > 0.5f ? colors[uid(randomEngine)] : colors[0];
				countObj->GetTransform()->SetLocalPosition(Vector3(screenPos.x, screenPos.y, 0.0f) + offset);
				countObj->GetTransform()->SetLocalScale(1.0f + std::pow(life, 8.0f) * 3.0f);
				countObj->GetComponent<GUIText>()->SetColor(Vector4(color.x, color.y, color.z, std::pow(life, 0.5f)));
			}
			else
				countObj->SetActive(false);
		}
		while (!m_countObjects.empty())
		{
			auto &[life, countObj, targetPos, velocity] = m_countObjects.front();
			if (life <= 0.0f)
			{
				countObj->RemoveFromParent();
				m_countObjects.pop_front();
			}
			else
				break;
		}
	}
	else
	{
		// TODO: 업데이트 하면 안될 때 밀어줘야 이전 이벤트가 갑자기 뜬금포로 나타나는 일이 없지 않을까?
		m_timer.ClearEvents();
	}
	
}

void DamageCountGUI::AddCountObject(const Vector3& targetPos, unsigned int damageCount, const int hit_count)
{
	constexpr const float DMG_DELTA_STEP = 0.1f;
	for (int i = 0; i < hit_count; ++i)
	{
		m_timer.RegisterEvent((float)i * DMG_DELTA_STEP, [=]() {
			std::shared_ptr<SceneObject> countObj = std::make_shared<SceneObject>();
			auto nameRenderer = countObj->AddComponent<GUIText>();
			nameRenderer->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"sansman.spritefont")));
			nameRenderer->SetRaycastTarget(false);
			nameRenderer->SetText(std::to_wstring(damageCount / hit_count));
			GetSceneObject()->AddChild(countObj);

			auto urd = std::uniform_real_distribution(-1.0f, 1.0f);
			Vector3 velocity = Vector3::Up * 0.5f;
			Vector3 offset = Vector3(urd(randomEngine), urd(randomEngine) + 1.0f, urd(randomEngine));

			m_countObjects.emplace_back(1.0f, countObj, targetPos + offset, velocity);
			m_soundInstance = INSTANCE(Resource)->Load<udsdx::AudioClip>(RESOURCE_PATH(L"audio\\hit_light.wav"))->CreateInstance();
			m_soundInstance->SetVolume(0.5f);
			m_soundInstance->Play();

			});
	}
}
