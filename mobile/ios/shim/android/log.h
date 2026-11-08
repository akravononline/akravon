// Cynex iOS: android/log.h yerine - ayni makrolar, stderr'e yazar
#pragma once
#include <stdio.h>
#include <stdarg.h>
enum { ANDROID_LOG_UNKNOWN = 0, ANDROID_LOG_DEFAULT, ANDROID_LOG_VERBOSE, ANDROID_LOG_DEBUG, ANDROID_LOG_INFO, ANDROID_LOG_WARN, ANDROID_LOG_ERROR, ANDROID_LOG_FATAL };
#ifdef __cplusplus
extern "C" {
#endif
static inline int __android_log_vprint(int prio, const char* tag, const char* fmt, va_list ap)
{
	static const char* const s_names = "?-VDIWEF";
	fprintf(stderr, "[%c] %s: ", s_names[prio < 0 || prio > 7 ? 0 : prio], tag ? tag : "");
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
	return 0;
}
static inline int __android_log_print(int prio, const char* tag, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	__android_log_vprint(prio, tag, fmt, ap);
	va_end(ap);
	return 0;
}
static inline int __android_log_write(int prio, const char* tag, const char* text)
{
	return __android_log_print(prio, tag, "%s", text ? text : "");
}
#ifdef __cplusplus
}
#endif
