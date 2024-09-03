#include "pch.h"
#include "audio.h"

namespace udsdx
{
	Audio::Audio()
	{
		AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
		eflags |= AudioEngine_Debug;
#endif
		m_audioEngine = std::make_unique<DirectX::AudioEngine>(eflags);
	}

	Audio::~Audio()
	{

	}

	AudioEngine* Audio::GetAudioEngine() const
	{
		return m_audioEngine.get();
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