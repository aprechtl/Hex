#ifndef HIGHRESELAPSEDTIMER_H
#define HIGHRESELAPSEDTIMER_H
#include <QtCore/QtGlobal>

#ifdef Q_OS_WIN32
#include <windows.h>

class HighResElapsedTimer
{
public:
    HighResElapsedTimer()
    { QueryPerformanceFrequency(&countFrequency); }

    void start()
    { QueryPerformanceCounter(&startCount); }

    double elapsedMilliseconds()
    {
        QueryPerformanceCounter(&endCount);
        return 1.0e3 * (endCount.QuadPart - startCount.QuadPart) / static_cast<double>(countFrequency.QuadPart);
    }

    double elapsedMicroseconds()
    {
        QueryPerformanceCounter(&endCount);
        return 1.0e6 * (endCount.QuadPart - startCount.QuadPart) / static_cast<double>(countFrequency.QuadPart);
    }

private:
    LARGE_INTEGER startCount;
    LARGE_INTEGER endCount;
    LARGE_INTEGER countFrequency;
};
#endif // Q_OS_WIN32

#ifdef Q_OS_MAC
#include <sys/time.h>

class HighResElapsedTimer
{
public:
    HighResElapsedTimer()
    {}

    void start()
    { gettimeofday(&startTime, NULL); }

    double elapsedMicroseconds()
    {
        gettimeofday(&endTime, NULL);
        return endTime.tv_usec - startTime.tv_usec;
    }

    double elapsedMilliseconds()
    { return elapsedMicroseconds() * .001; }

private:
    timeval startTime;
    timeval endTime;
};

#endif // Q_OS_MAC

#endif
