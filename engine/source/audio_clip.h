#pragma once

#include "pch.h"
#include "resource_object.h"

namespace udsdx
{
	class AudioClip : public ResourceObject
	{
	public:
		AudioClip(std::wstring_view path, AudioEngine* audioEngine);
		~AudioClip();

	public:
		std::unique_ptr<DirectX::SoundEffectInstance> CreateInstance();
		std::unique_ptr<DirectX::SoundEffectInstance> CreateInstance3D(const Vector3& position, float volume = 1.0f);

	private:
		std::unique_ptr<DirectX::SoundEffect> m_soundEffect;
	};
}