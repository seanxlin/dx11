#pragma once

#include <cstdint>
#include <windows.h>

struct Timer
{
    Timer()
        : mLastStartTime(0)
        , mInPauseTime(0)
        , mPreviousTickTime(0)
        , mCurrentTickTime(0)
        , mSecondsPerCount(0.0)
        , mDeltaTime(-1.0)
        , mIsStopped(false)
    {
        // Get the frequency of the high-resolution performance counter, if one exists. 
        // The frequency cannot change while the system is running.
        uint64_t countsPerSec;
        QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER *> (&countsPerSec));
        mSecondsPerCount = 1.0 / static_cast<double> (countsPerSec);
    }

    uint64_t mLastStartTime;
    uint64_t mInPauseTime;
    uint64_t mLastStopTime;
    uint64_t mPreviousTickTime;
    uint64_t mCurrentTickTime;

    double mSecondsPerCount;
    double mDeltaTime;

    bool mIsStopped;
};

namespace TimerUtils
{
    // Returns the total time elapsed since reset() was called, 
    // NOT counting any time when the clock is stopped.
    float inGameTime(const Timer& timer);

    // Call before message loop.
    void reset(Timer& timer); 

    // Call when unpaused.
    void start(Timer& timer); 

    // Call when paused.
    void stop(Timer& timer);  

    // Call every frame.
    void tick(Timer& timer); 
}  