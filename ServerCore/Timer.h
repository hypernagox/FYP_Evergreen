#pragma once

namespace ServerCore
{
	class Timer
	{
	public:
		inline void Update()noexcept {
			const auto CurTime = std::chrono::steady_clock::now();
			m_DeltaTime = CurTime - m_PrevTime;
			m_PrevTime = CurTime;
		}
		inline const float GetDT()const noexcept { return  m_DeltaTime.count(); }
	private:
		std::chrono::steady_clock::time_point m_PrevTime = std::chrono::steady_clock::now();
		std::chrono::duration<float> m_DeltaTime = std::chrono::duration<float>(0.016f);
	};
}