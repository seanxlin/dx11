//////////////////////////////////////////////////////////////////////////
//
// Timer class 
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <windows.h>

struct Timer
{
    Timer();

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