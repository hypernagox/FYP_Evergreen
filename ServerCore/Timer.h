#pragma once

namespace ServerCore
{
	class Timer
	{
	public:
		inline void Update()noexcept {
			const auto CurTime = ::GetTickCount64();
			m_DeltaTime = CurTime - m_PrevTime;
			m_PrevTime = CurTime;
		}
		inline const float GetDT()const noexcept { return  ((float)m_DeltaTime) / 1000.f; }
		inline const uint64_t GetDTMs()const noexcept { return m_DeltaTime; }
	private:
		uint64_t m_PrevTime = ::GetTickCount64();
		uint64_t m_DeltaTime = 16;
	};
}