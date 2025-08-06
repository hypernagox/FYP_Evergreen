#include "pch.h"
#include "audio.h"
#include "debug_console.h"

namespace udsdx
{
	Audio::Audio()
	{
		std::wstring logMessage = L"Audio Renderer Details:\n\n";
		for (const auto& detail : AudioEngine::GetRendererDetails())
		{
			logMessage += std::format(L"  > Audio Renderer: {} {}\n", detail.description, detail.deviceId);
		}
		DebugConsole::Log(logMessage);

		AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
		eflags |= AudioEngine_Debug;
#endif
		m_audioEngine = std::make_unique<DirectX::AudioEngine>(eflags);
		m_audioEngine->SetReverb(Reverb_Cave);
	}

	Audio::~Audio()
	{

	}

	AudioEngine* Audio::GetAudioEngine() const
	{
		return m_audioEngine.get();
	}

	const AudioListener& Audio::GetAudioListener() const
	{
		return m_audioListener;
	}

	void Audio::UpdateAudioListener(const Vector3& position, const Quaternion& orientation)
	{
		if (!m_audioEngine)
		{
			return;
		}
		
		m_audioListener.SetPosition(position);
		m_audioListener.SetOrientationFromQuaternion(orientation);
	}

	void Audio::Update()
	{ ZoneScoped;
		if (!m_audioEngine)
		{
			return;
		}

		m_audioEngine->Update();
	}
}