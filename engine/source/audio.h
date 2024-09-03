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

	private:
		std::unique_ptr<AudioEngine> m_audioEngine;
	};
}