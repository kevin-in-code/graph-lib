#pragma once

/**
* StopWatch.hpp
* Purpose: For measuring the running time of algorithms.
*
* @author Kevin A. Naudé
* @version 1.1
*/

#ifdef _WIN32

#include <windows.h>
typedef LARGE_INTEGER TimerValue;
#define TimerValue_Clear(val)val.QuadPart = 0
#define Timer_Capture(var)QueryPerformanceCounter(&(var))

#else

#include <stddef.h>
#include <sys/time.h>
typedef struct timeval TimerValue;
#define TimerValue_Clear(val)val.tv_sec = val.tv_usec = 0
#define Timer_Capture(var)gettimeofday(&(var), NULL)

#endif

namespace kn
{

    class StopWatch
    {
    private:
#ifdef _WIN32
        typedef LARGE_INTEGER TimerValue;
#else
        typedef struct timeval TimerValue;
#endif
        double seconds;
        TimerValue startTime;
        TimerValue endTime;

#ifdef _WIN32
        TimerValue frequency;
#endif

        static void clear(TimerValue& tv)
        {
#ifdef _WIN32
            tv.QuadPart = 0;
#else
            tv.tv_sec = tv.tv_usec = 0;
#endif
        }

        static void capture(TimerValue& tv)
        {
#ifdef _WIN32
            QueryPerformanceCounter(&tv);
#else
            gettimeofday(&tv, NULL);
#endif
        }

    public:
        StopWatch()
        {
            reset();
        }

        double elapsedSeconds()
        {
            return seconds;
        }

        void start()
        {
            capture(startTime);
        }

        void stop()
        {
            capture(endTime);
#ifdef _WIN32
            double time = ((double)(endTime.QuadPart - startTime.QuadPart) / frequency.QuadPart);
#else
            double time = ((double)(endTime.tv_sec) + 0.000001*endTime.tv_usec) - ((double)(startTime.tv_sec) + 0.000001*startTime.tv_usec);
#endif
            if (time >= 0.0) seconds += time;
        }

        void reset()
        {
            seconds = 0.0;
            clear(startTime);
            clear(endTime);
#ifdef _WIN32
            QueryPerformanceFrequency(&frequency);
#endif
        }
    };

}
