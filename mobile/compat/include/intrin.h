// Cynex: msvc intrin.h
#pragma once
#include <windows.h>
#if defined(__x86_64__) || defined(__i386__)
#include <x86intrin.h>
#else
inline unsigned long long __rdtsc() { unsigned long long v; asm volatile("mrs %0, cntvct_el0" : "=r"(v)); return v; }
#endif
#define _ReadWriteBarrier() __atomic_signal_fence(__ATOMIC_SEQ_CST)
#define _mm_pause() ((void)0)
