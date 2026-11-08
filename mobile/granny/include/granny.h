// Cynex: granny 2.11.8 public header'i icin platform ayari (cynex_granny)
// Cynex: orijinal header'a dokunmadan android arm64'u 64bit pointer olarak tanitir
#ifndef CYNEX_GRANNY_PLATFORM_H
#define CYNEX_GRANNY_PLATFORM_H

#if defined(__aarch64__) && !defined(ANDROID_ARM64)
#define ANDROID_ARM64 1
#endif

#include <granny2.11.8.0.h>

#endif
