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
			velocity.y -= 9.8f * time.deltaTime;
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
				countObj->GetTransform()->SetLocalPosition(Vector3(screenPos.x, screenPos.y, 0.0f));
				countObj->GetComponent<GUIText>()->SetColor(Vector4(1.0f, 1.0f, 1.0f, life));
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
	
}

void DamageCountGUI::AddCountObject(const Vector3& targetPos, unsigned int damageCount, const int hit_count)
{
	constexpr const float DMG_DELTA_STEP = 0.1f;
	for (int i = 0; i < hit_count; ++i)
	{
		m_timer.RegisterEvent((float)i * DMG_DELTA_STEP, [=]() {
			std::shared_ptr<SceneObject> countObj = std::make_shared<SceneObject>();
			auto nameRenderer = countObj->AddComponent<GUIText>();
			nameRenderer->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"impact.spritefont")));
			nameRenderer->SetRaycastTarget(false);
			nameRenderer->SetText(std::to_wstring(damageCount / 5));
			GetSceneObject()->AddChild(countObj);

			auto urd = std::uniform_real_distribution(-1.0f, 1.0f);
			Vector3 velocity = Vector3(urd(randomEngine), 3.0f, urd(randomEngine));

			m_countObjects.emplace_back(1.0f, countObj, targetPos, velocity);
			m_soundInstance = INSTANCE(Resource)->Load<udsdx::AudioClip>(RESOURCE_PATH(L"audio\\uihover.wav"))->CreateInstance();
			m_soundInstance->SetVolume(0.5f);
			m_soundInstance->Play();

			});
	}
}
