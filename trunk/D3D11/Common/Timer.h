#pragma once

#include <cstdint>
#include <windows.h>

namespace Utils
{
    class Timer
    {
    public:
        Timer::Timer()
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
            QueryPerformanceFrequency( reinterpret_cast<LARGE_INTEGER *> (&countsPerSec) );
            mSecondsPerCount = 1.0 / static_cast<double> (countsPerSec);
        }

        __forceinline float inGameTime() const; // in seconds
        __forceinline float deltaTime() const; // in seconds

        // Call before message loop.
        void reset(); 

        // Call when unpaused.
        void start(); 

        // Call when paused.
        void stop();  

        // Call every frame.
        void tick();  

    private:
        uint64_t mLastStartTime;
        uint64_t mInPauseTime;
        uint64_t mLastStopTime;
        uint64_t mPreviousTickTime;
        uint64_t mCurrentTickTime;

        double mSecondsPerCount;
        double mDeltaTime;

        bool mIsStopped;
    };

    // Returns the total time elapsed since reset() was called, NOT counting any
    // time when the clock is stopped.
    float Timer::inGameTime() const
    {
        // IF we are stopped, do not count the time that has passed since we stopped.
        //
        // ----*---------------*------------------------------*------> time
        // mLastStartTime  mLastStopTime                mCurrentTickTime
        //
        // ELSE:
        // The distance mCurrentTickTime - mLastStartTime includes paused time,
        // which we do not want to count. To correct this, we can subtract 
        // the paused time from mCurrentTickTime:  
        //
        //  (mCurrentTickTime - mInPauseTime) - mLastStartTime 
        //
        //                     |<---mInPauseTime-->|
        // ----*---------------*-------------------*-------------*------> time
        //  mLastStartTime  mLastStopTime       newStartTime  mCurrentTickTime

        return (mIsStopped) ? static_cast<float> ( (mLastStopTime - mLastStartTime) * mSecondsPerCount )
            : static_cast<float> ( (mCurrentTickTime - mInPauseTime - mLastStartTime) * mSecondsPerCount );
    }

    float Timer::deltaTime() const
    {
        return static_cast<float> (mDeltaTime);
    }
}