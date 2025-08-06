#include "pch.h"
#include "audio_clip.h"
#include "audio.h"

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
		return m_soundEffect->CreateInstance(SoundEffectInstance_NoSetPitch);
	}

	std::unique_ptr<DirectX::SoundEffectInstance> AudioClip::CreateInstance3D(const Vector3& position, float volume)
	{
		auto instance = m_soundEffect->CreateInstance(SoundEffectInstance_Use3D | SoundEffectInstance_NoSetPitch);
		AudioEmitter emitter;
		emitter.SetPosition(position);
		emitter.EnableInverseSquareCurves();
		emitter.CurveDistanceScaler = 10.0f;
		instance->SetVolume(volume);
		instance->Play();
		instance->Apply3D(INSTANCE(Audio)->GetAudioListener(), emitter, false);
		return instance;
	}
}