#include "Timer.h"

namespace Utils
{
    namespace TimerUtils
    {
        // Returns the total time elapsed since reset() was called, NOT counting any
        // time when the clock is stopped.
        float inGameTime(const Timer& timer)
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
            float time = 0.0f;
            if (timer.mIsStopped) 
            {
                time = static_cast<float> ((timer.mLastStopTime - timer.mLastStartTime) * timer.mSecondsPerCount);
            } 

            else 
            {
                time = static_cast<float> ((timer.mCurrentTickTime - timer.mInPauseTime - timer.mLastStartTime) * timer.mSecondsPerCount );
            }
            
            return time;
        }

        // Call before message loop.
        void reset(Timer& timer)
        {
            // Last start time will be the current elapsed time.
            QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *> (&timer.mLastStartTime));

            // Reset the previous tick frame.
            timer.mPreviousTickTime = timer.mLastStartTime;

            // As we are resetting, then there is no last stop time.
            timer.mLastStopTime = 0;
            timer.mIsStopped  = false;
        }

        // Call when unpaused.
        void start(Timer& timer)
        {
            // Accumulate the time elapsed between stop and start pairs.
            //
            //                     |<-------d------->|
            // ----*---------------*-----------------*------------> time
            //  mLastStartTime    mLastStopTime      newStartTime     

            if (timer.mIsStopped)
            {
                // If timer was stopped then the previous tick time will be the new start time.
                QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *> (&timer.mPreviousTickTime));

                // Update the total in pause time.
                timer.mInPauseTime += (timer.mPreviousTickTime - timer.mLastStopTime);	

                // Because the timer was stopped and now started, then there is no last stop time.
                timer.mLastStopTime = 0;
                timer.mIsStopped = false;
            }
        }

        // Call when paused.
        void stop(Timer& timer)
        {
            if (!timer.mIsStopped)
            {
                // If the timer is not stopped, then we need to update the 
                // last stop time.
                QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *> (&timer.mLastStopTime));

                timer.mIsStopped  = true;
            }
        }

        // Call every frame.
        void tick(Timer& timer)
        {
            // If timer is stopped, then there is no delta time between
            // previous and current tick time.
            if (timer.mIsStopped)
            {
                timer.mDeltaTime = 0.0;
            }

            else 
            {
                // Update current tick time.
                QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *> (&timer.mCurrentTickTime));

                // Time difference between this frame and the previous.
                // Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
                // processor goes into a power save mode or we get shuffled to another
                // processor, then mDeltaTime can be negative.
                const double newDeltaTime = (timer.mCurrentTickTime - timer.mPreviousTickTime) * timer.mSecondsPerCount;
                timer.mDeltaTime = (newDeltaTime < 0.0) ? 0.0 : newDeltaTime;

                // Prepare for next frame.
                timer.mPreviousTickTime = timer.mCurrentTickTime;
            }
        }
    }
}