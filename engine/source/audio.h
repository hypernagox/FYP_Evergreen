#pragma once

#include "pch.h"

namespace udsdx
{
	class Audio
	{
	public:
		Audio();
		~Audio();

		void Update();
		AudioEngine* GetAudioEngine() const;
		const AudioListener& GetAudioListener() const;
		void UpdateAudioListener(const Vector3& position, const Quaternion& orientation);

	private:
		std::unique_ptr<AudioEngine> m_audioEngine;
		AudioListener m_audioListener;
	};
}