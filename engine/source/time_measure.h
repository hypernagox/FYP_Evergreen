#pragma once

#include "pch.h"

namespace udsdx
{
	class TimeMeasure
	{
    public:
        TimeMeasure();

    public:
        // Get elapsed time since the previous Update call.
        UINT64 GetElapsedTicks() const { return m_elapsedTicks; }
        float GetElapsedSeconds() const { return TicksToSeconds(m_elapsedTicks); }

        // Get total time since the start of the program.
        UINT64 GetTotalTicks() const { return m_totalTicks; }
        float GetTotalSeconds() const { return TicksToSeconds(m_totalTicks); }

        // Get total number of updates since start of the program.
        UINT32 GetFrameCount() const { return m_frameCount; }

        // Get the current framerate.
        UINT32 GetFramesPerSecond() const { return m_framesPerSecond; }

        // Set whether to use fixed or variable timestep mode.
        void SetFixedTimeStep(bool isFixedTimestep) { m_isFixedTimeStep = isFixedTimestep; }

        // Set how often to call Update when in fixed timestep mode.
        void SetTargetElapsedTicks(UINT64 targetElapsed) { m_targetElapsedTicks = targetElapsed; }
        void SetTargetElapsedSeconds(float targetElapsed) { m_targetElapsedTicks = SecondsToTicks(targetElapsed); }

        // Integer format represents time using 10,000,000 ticks per second.
        static const UINT64 TicksPerSecond = 10000000;

        static float TicksToSeconds(UINT64 ticks) { return static_cast<float>(ticks) / TicksPerSecond; }
        static UINT64 SecondsToTicks(float seconds) { return static_cast<UINT64>(seconds * TicksPerSecond); }

        Time GetTime() const;

    public:
        // After an intentional timing discontinuity (for instance a blocking IO operation)
        // call this to avoid having the fixed timestep logic attempt a set of catch-up 
        // Update calls.
        void ResetElapsedTime();

        typedef void(*LPUPDATEFUNC) (void);

        // Update timer state, calling the specified Update function the appropriate number of times.
        void Tick(LPUPDATEFUNC update = nullptr);

    private:
        // Source timing data uses QPC units.
        LARGE_INTEGER m_qpcFrequency;
        LARGE_INTEGER m_qpcLastTime;
        UINT64 m_qpcMaxDelta;

        // Derived timing data uses a canonical tick format.
        UINT64 m_elapsedTicks;
        UINT64 m_totalTicks;
        UINT64 m_leftOverTicks;

        // Members for tracking the framerate.
        UINT32 m_frameCount;
        UINT32 m_framesPerSecond;
        UINT32 m_framesThisSecond;
        UINT64 m_qpcSecondCounter;

        // Members for configuring fixed timestep mode.
        bool m_isFixedTimeStep;
        UINT64 m_targetElapsedTicks;
	};
}