#pragma once
#include "ServerCorePch.h"

namespace ServerCore
{
	extern "C" void delay_loop(const int delay)noexcept;
	// TODO: 템플릿으로 만들어서 백오프 카운트 조절
	class BackOff
	{
	public:
		inline void delay()const noexcept
		{
			if (const int delay = my_rand() % limit)
			{
				delay_loop(delay);
				if ((limit = (limit << 1)) > maxDelay)limit = maxDelay;
			}
		}
	private:
		inline static constexpr const int minDelay = 1;
		inline static constexpr const int maxDelay = NUM_OF_THREADS;
		mutable int limit = minDelay;
	};
}