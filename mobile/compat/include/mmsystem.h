// Cynex: winmm - timeGetTime windows.h icinde
#pragma once
#include <windows.h>
typedef UINT MMRESULT;
#define TIMERR_NOERROR 0
typedef struct timecaps_tag { UINT wPeriodMin; UINT wPeriodMax; } TIMECAPS, *PTIMECAPS, *LPTIMECAPS;
inline MMRESULT timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc) { if (ptc) { ptc->wPeriodMin = 1; ptc->wPeriodMax = 1000000; } return TIMERR_NOERROR; }
