/* 
        MIT License

        Copyright (c) 2019 Braedon Wooding

        Permission is hereby granted, free of charge, to any person obtaining a copy
        of this software and associated documentation files (the "Software"), to deal
        in the Software without restriction, including without limitation the rights
        to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
        copies of the Software, and to permit persons to whom the Software is
        furnished to do so, subject to the following conditions:

        The above copyright notice and this permission notice shall be included in all
        copies or substantial portions of the Software.

        THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
        IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
        FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
        LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
        OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
        SOFTWARE.
*/

/*
    A relatively simple benchmarking library
    Note: you can use system to externally run programs
    Inspiration taken from: http://nadeausoftware.com/articles/2012/03/c_c_tip_how_measure_cpu_time_benchmarking
*/

#ifndef CBENCH_H

#if defined _MSC_VER || defined __MINGW32_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <sys/time.h>
#include <time.h>
#endif

#define CBENCH_MAJOR_V "1"
#define CBENCH_MINOR_V "0"
#define CBENCH_PATCH_V "a"

#define CBENCH_VERSION CBENCH_MAJOR_V "." CBENCH_MINOR_V "." CBENCH_PATCH_V

/*
    Represents a time split into user and system components.
*/
typedef struct cbench_time_t {
    double userTime;
    double systemTime;
} cbenchTime;

/*
    Uses high accuracy timers to get overall 'wall' time
    i.e. real time.
*/
double cbenchGetWallTime();

/*
    Get's the CPU and System times for all children processes
    If either were failed to be retrieved returns -1 for the relevant time
*/
cbenchTime cbenchGetChildrenTime();

/*
    Returns the CPU and System time if it could retrieve it else -1
*/
cbenchTime cbenchGetTime();

double cbenchGetWallTime() {
    // attempts to use highest accuracy timers
#if defined _MSC_VER || defined __MINGW32
    LARGE_INTENGER t, f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency(&f);
    return (double)t.QuadPart/f.QuadPart;
#else
#if defined _POSIX_TIMERS && _POSIX_TIMERS > 0
    {
        // CLOCK_REALTIME is significantly more available than other clocks
        // Much more than CLOCK_MONOTOMIC and while I could check them and do
        // some 'magic' I felt for the purposes of this library it'll be fine
        // if you really care give me a case where it would matter :). + PR
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) != -1) {
            return ts.tv_sec + ts.tv_nsec / 1000000000.0;
        }
    }
#else
    {
        struct timeval tv;
        if (gettimeofday(&tv, NULL) != -1) {
            return tv.tv_sec + tv.tv_usec / 1000000.0;
        }
    }
#endif
    return -1;
#endif
}

cbenchTime cbenchGetChildrenTime() {
#if defined _MSC_VER && !defined __MINGW32__


#else
    // Note: we don't really use high precision timers because
    //       They often have no way to get System time
    //       and for Cpath's purposes system time is almost more important
#if defined RUSAGE_SELF
    {
        struct rusage rusage;
        if (getrusage(RUSAGE_CHILDREN, &rusage) != -1) {
            cbenchTime time = {
                rusage.ru_utime.tv_sec + rusage.ru_utime.tv_usec / 1000000.0,
                rusage.ru_stime.tv_sec + rusage.ru_stime.tv_usec / 1000000.0
            };
            return time;
        }
    }
#endif

#if defined _SC_CLK_TCK
    {
        const double ticks = sysconf(_SC_CLK_TCK);
        struct tms tms;
        if (times(&tms) != (clock_t)-1) {
            cbenchTime time = { tms.tms_cutime / ticks, tms.tms_cstime / ticks };
            return time;
        }
    }
#endif

    // I don't think clock() will work here since it won't handle children

#endif

    cbenchTime time = { -1, -1 };
    return time;
}

cbenchTime cbenchGetTime() {
#if defined _MSC_VER || defined __MINGW32__
    FILETIME createTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;

    if (GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, &kernelTime,
                                            &userTime)) {
        SYSTEMTIME userSysTime;
        SYSTEMTIME kernelSysTime;
        double userRes = -1;
        double kernelRes = -1;
        if (FileTimeToSystemTime(&userTime, &userSysTime) != -1) {
            userRes = userSysTime.wHour * 3600.0 + userSysTime.wMinute * 60.0 +
                                userSysTime.wSecond + userSysTime.wMilliseconds / 1000.0;
        }
        if (FileTimeToSystemTime(&userTime, &kernelSysTime) != -1) {
            kernelRes = kernelSysTime.wHour * 3600.0 + kernelSysTime.wMinute * 60.0 +
                                    kernelSysTime.wSecond + kernelSysTime.wMilliseconds / 1000.0;
        }
        cbenchTime time = { userRes, kernelRes };
        return time;
    }
#else
    // Note: we don't really use high precision timers because
    //       They often have no way to get System time
    //       and for Cpath's purposes system time is almost more important
#if defined RUSAGE_SELF
    {
        struct rusage rusage;
        if (getrusage(RUSAGE_SELF, &rusage) != -1) {
            cbenchTime time = {
                rusage.ru_utime.tv_sec + rusage.ru_utime.tv_usec / 1000000.0,
                rusage.ru_stime.tv_sec + rusage.ru_stime.tv_usec / 1000000.0
            };
            return time;
        }
    }
#endif

#if defined _SC_CLK_TCK
    {
        const double ticks = sysconf(_SC_CLK_TCK);
        struct tms tms;
        if (times(&tms) != (clock_t)-1) {
            cbenchTime time = { tms.tms_utime / ticks, tms.tms_stime / ticks };
            return time;
        }
    }
#endif

#if defined CLOCKS_PER_SEC
    {
        // This is a rough point since it means we can't distinguish the user
        // from the IO time.  So we just report the user time.
        clock_t clk = clock();
        if (clk != (clock_t)-1) {
            cbenchTime time = { clk / (double)CLOCKS_PER_SEC, -1 };
            return time;
        }
    }
#endif

#endif

    cbenchTime time = { -1, -1 };
    return time;
}

#endif /* CBench.h */
