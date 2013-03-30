#include "Timer.h"

namespace Utils
{
    // Call before message loop.
    void Timer::reset()
    {
        // Last start time will be the current elapsed time.
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *> (&mLastStartTime));

        // Reset the previous tick frame.
        mPreviousTickTime = mLastStartTime;

        // As we are resetting, then there is no last stop time.
        mLastStopTime = 0;
        mIsStopped  = false;
    }

    // Call when unpaused.
    void Timer::start()
    {
        // Accumulate the time elapsed between stop and start pairs.
        //
        //                     |<-------d------->|
        // ----*---------------*-----------------*------------> time
        //  mLastStartTime    mLastStopTime      newStartTime     

        if (mIsStopped)
        {
            // If timer was stopped then the previous tick time will be the new start time.
            QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *> (&mPreviousTickTime));

            // Update the total in pause time.
            mInPauseTime += (mPreviousTickTime - mLastStopTime);	

            // Because the timer was stopped and now started, then there is no last stop time.
            mLastStopTime = 0;
            mIsStopped = false;
        }
    }

    // Call when paused.
    void Timer::stop()
    {
        if (!mIsStopped)
        {
            // If the timer is not stopped, then we need to update the 
            // last stop time.
            QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *> (&mLastStopTime));

            mIsStopped  = true;
        }
    }

    // Call every frame.
    void Timer::tick()
    {
        // If timer is stopped, then there is no delta time between
        // previous and current tick time.
        if (mIsStopped)
        {
            mDeltaTime = 0.0;
        }

        else 
        {
            // Update current tick time.
            QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *> (&mCurrentTickTime));

            // Time difference between this frame and the previous.
            // Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
            // processor goes into a power save mode or we get shuffled to another
            // processor, then mDeltaTime can be negative.
            const double newDeltaTime = (mCurrentTickTime - mPreviousTickTime) * mSecondsPerCount;
            mDeltaTime = (newDeltaTime < 0.0) ? 0.0 : newDeltaTime;

            // Prepare for next frame.
            mPreviousTickTime = mCurrentTickTime;
        }
    }
}