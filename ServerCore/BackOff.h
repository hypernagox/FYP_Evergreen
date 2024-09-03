#pragma once
#include "ServerCorePch.h"

namespace ServerCore
{
	extern "C" void delay_loop(const int delay)noexcept;

	class BackOff
	{
	public:
		BackOff(const int max)noexcept
			:maxDelay{ max }, limit{ minDelay } {}

		inline void delay()const noexcept
		{
			if (const int delay = my_rand() % limit)
			{
				limit *= 2;
				if (limit > maxDelay)limit = maxDelay;
				delay_loop(delay);
			}
		}
	private:
		inline static constexpr const int minDelay = 1;
		mutable int limit = minDelay;
		const int maxDelay;
	};
}