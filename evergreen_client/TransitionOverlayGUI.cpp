#include "pch.h"
#include "TransitionOverlayGUI.h"

using namespace udsdx;

void TransitionOverlayGUI::OnInitialize()
{
	m_panel = SceneObject::MakeShared();
	m_backgroundRenderer = m_panel->AddComponent<GUIImage>();
	m_backgroundRenderer->SetTexture(INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\square.png")));
	m_backgroundRenderer->SetSize(Vector2::One * 8192.0f);

	GetSceneObject()->AddChild(m_panel);

	m_messageText = SceneObject::MakeShared();
	auto messageText = m_messageText->AddComponent<GUIText>();
	messageText->SetFont(INSTANCE(Resource)->Load<udsdx::Font>(RESOURCE_PATH(L"pretendard.spritefont")));
	messageText->SetRaycastTarget(false);
	messageText->SetAlignment(GUIText::Alignment::LowerRight);
	m_messageText->GetTransform()->SetLocalPosition(Vector3(1260.0f, -700.0f, 0.0f));

	m_panel->AddChild(m_messageText);
}

void TransitionOverlayGUI::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	m_transitionDurationRemain -= time.deltaTime;
	if (m_transitionDurationRemain <= 0.0f)
		m_panel->SetActive(false);
	else
	{
		m_backgroundRenderer->SetColor(Color(0.0f, 0.0f, 0.0f, 1.0f - std::abs(m_transitionDurationRemain / TransitionDuration - 1.0f)));
		if (!m_transitionCallbacks.empty() && m_transitionDurationRemain <= TransitionDuration)
		{
			m_messageText->SetActive(false);
			for (const auto& callback : m_transitionCallbacks)
			{
				if (callback)
					callback();
			}
			m_transitionCallbacks.clear();
		}
	}
}

void TransitionOverlayGUI::BeginFadeOut()
{
	m_transitionDurationRemain = TransitionDuration;
	m_panel->SetActive(true);
	m_messageText->SetActive(false);
}

void TransitionOverlayGUI::AppendTransition(std::function<void()> callback, std::wstring_view message)
{
	static std::unique_ptr<SoundEffectInstance> g_menuSound;

	m_transitionCallbacks.push_back(callback);
	m_messageText->GetComponent<GUIText>()->SetText(message.data());

	if (m_transitionDurationRemain <= TransitionDuration)
	{
		m_transitionDurationRemain = TransitionDuration * 2.0f;
		m_panel->SetActive(true);
		m_messageText->SetActive(true);

		g_menuSound = INSTANCE(Resource)->Load<udsdx::AudioClip>(RESOURCE_PATH(L"audio\\transition.wav"))->CreateInstance();
		g_menuSound->SetVolume(0.5f);
		g_menuSound->Play();
	}
}
