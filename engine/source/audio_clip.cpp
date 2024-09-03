#include "pch.h"
#include "audio_clip.h"

namespace udsdx
{
	AudioClip::AudioClip(std::wstring_view path, AudioEngine* audioEngine) : ResourceObject(path)
	{
		m_soundEffect = std::make_unique<DirectX::SoundEffect>(audioEngine, path.data());
	}

	AudioClip::~AudioClip()
	{

	}

	std::unique_ptr<DirectX::SoundEffectInstance> AudioClip::CreateInstance()
	{
		return m_soundEffect->CreateInstance();
	}
}