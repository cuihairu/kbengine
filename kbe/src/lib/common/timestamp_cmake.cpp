// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "common/timestamp.h"

#if KBE_PLATFORM == PLATFORM_WIN32
#include <windows.h>
#endif

namespace KBEngine {

#if KBE_PLATFORM == PLATFORM_WIN32
KBETimingMethod g_timingMethod = NO_TIMING_METHOD;
#elif KBE_PLATFORM == PLATFORM_UNIX
KBETimingMethod g_timingMethod = RDTSC_TIMING_METHOD;
#else
KBETimingMethod g_timingMethod = GET_TIME_TIMING_METHOD;
#endif

const char* getTimingMethodName()
{
  switch (g_timingMethod)
  {
  case NO_TIMING_METHOD:
    return "none";
  case RDTSC_TIMING_METHOD:
    return "rdtsc";
  case GET_TIME_OF_DAY_TIMING_METHOD:
    return "gettimeofday";
  case GET_TIME_TIMING_METHOD:
    return "gettime";
  default:
    return "Unknown";
  }
}

uint64 stampsPerSecond_rdtsc()
{
#if KBE_PLATFORM == PLATFORM_UNIX
  static const uint64 value = []() -> uint64 {
    const uint64 before = timestamp();
    const uint64 time_before = timestamp_gettimeofday();

    struct timeval sleep_time = {0, 500000};
    select(0, NULL, NULL, NULL, &sleep_time);

    const uint64 after = timestamp();
    const uint64 time_after = timestamp_gettimeofday();
    const uint64 elapsed_us = time_after - time_before;
    return elapsed_us > 0 ? ((after - before) * 1000000ULL) / elapsed_us : 0ULL;
  }();

  return value;
#else
  return stampsPerSecond_gettimeofday();
#endif
}

double stampsPerSecondD_rdtsc()
{
  return static_cast<double>(stampsPerSecond_rdtsc());
}

uint64 stampsPerSecond_gettimeofday()
{
  return 1000000ULL;
}

double stampsPerSecondD_gettimeofday()
{
  return static_cast<double>(stampsPerSecond_gettimeofday());
}

uint64 stampsPerSecond()
{
#if KBE_PLATFORM == PLATFORM_WIN32
  static const uint64 value = []() -> uint64 {
    LARGE_INTEGER rate;
    QueryPerformanceFrequency(&rate);
    return static_cast<uint64>(rate.QuadPart);
  }();
  return value;
#elif KBE_PLATFORM == PLATFORM_UNIX
  if (g_timingMethod == RDTSC_TIMING_METHOD)
  {
    return stampsPerSecond_rdtsc();
  }

  if (g_timingMethod == GET_TIME_OF_DAY_TIMING_METHOD)
  {
    return stampsPerSecond_gettimeofday();
  }

  return 1000000000ULL;
#else
  if (g_timingMethod == GET_TIME_OF_DAY_TIMING_METHOD)
  {
    return stampsPerSecond_gettimeofday();
  }

  return 1000000000ULL;
#endif
}

double stampsPerSecondD()
{
  return static_cast<double>(stampsPerSecond());
}

} // namespace KBEngine
