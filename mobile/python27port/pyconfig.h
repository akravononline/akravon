/* Cynex: pyconfig.h - android bionic, api 26, arm64 + x86_64 */
#ifndef Py_PYCONFIG_H
#define Py_PYCONFIG_H

/* --- tip boyutlari (LP64) --- */
#define SIZEOF_SHORT 2
#define SIZEOF_INT 4
#define SIZEOF_LONG 8
#define SIZEOF_LONG_LONG 8
#define SIZEOF_VOID_P 8
#define SIZEOF_SIZE_T 8
#define SIZEOF_UINTPTR_T 8
#define SIZEOF_FLOAT 4
#define SIZEOF_DOUBLE 8
#define SIZEOF_LONG_DOUBLE 16
#define SIZEOF_FPOS_T 8
#define SIZEOF_OFF_T 8
#define SIZEOF_PID_T 4
#define SIZEOF_TIME_T 8
#define SIZEOF_WCHAR_T 4
#define SIZEOF__BOOL 1
#define SIZEOF_PTHREAD_T 8

#define HAVE_LONG_LONG 1
#define HAVE_LONG_DOUBLE 1
#define HAVE_UINTPTR_T 1
#define HAVE_C99_BOOL 1
#define HAVE_STDINT_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_UINT32_T 1
#define HAVE_UINT64_T 1
#define HAVE_INT32_T 1
#define HAVE_INT64_T 1
#define HAVE_SSIZE_T 1

#define PY_FORMAT_LONG_LONG "ll"
#define PY_FORMAT_SIZE_T "z"

/* --- derleyici / ANSI --- */
#define STDC_HEADERS 1
#define HAVE_PROTOTYPES 1
#define HAVE_STDARG_PROTOTYPES 1
/* #undef HAVE_GCC_ASM_FOR_X87 */
/* #undef HAVE_ATTRIBUTE_FORMAT_PARSETUPLE */
#if defined(__x86_64__)
#define VA_LIST_IS_ARRAY 1
#endif
#define DOUBLE_IS_LITTLE_ENDIAN_IEEE754 1

/* --- basliklar --- */
#define HAVE_DIRENT_H 1
#define HAVE_ERRNO_H 1
#define HAVE_FCNTL_H 1
#define HAVE_LIMITS_H 1
#define HAVE_SIGNAL_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_STRINGS_H 1
#define HAVE_UNISTD_H 1
#define HAVE_UTIME_H 1
#define HAVE_WCHAR_H 1
#define HAVE_SYS_FILE_H 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_SYS_SELECT_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TIME_H 1
#define HAVE_SYS_TIMES_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_UTSNAME_H 1
#define HAVE_SYS_WAIT_H 1
#define HAVE_SYS_RESOURCE_H 1
#define HAVE_PTHREAD_H 1
#define HAVE_POLL_H 1
#define TIME_WITH_SYS_TIME 1
/* #undef HAVE_STAT_TSPEC */

/* --- zaman --- */
#define HAVE_CLOCK 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_MKTIME 1
#define HAVE_STRFTIME 1
#define HAVE_TIMES 1
#define HAVE_SELECT 1
#define HAVE_STRUCT_TM_TM_ZONE 1
#define HAVE_TM_ZONE 1
#define HAVE_TZNAME 1
#define HAVE_DECL_TZNAME 1

/* --- matematik --- */
#define HAVE_ACOSH 1
#define HAVE_ASINH 1
#define HAVE_ATANH 1
#define HAVE_COPYSIGN 1
#define HAVE_ERF 1
#define HAVE_ERFC 1
#define HAVE_EXPM1 1
#define HAVE_FINITE 1
#define HAVE_HYPOT 1
#define HAVE_LGAMMA 1
#define HAVE_LOG1P 1
#define HAVE_ROUND 1
#define HAVE_TGAMMA 1
#define HAVE_DECL_ISINF 1
#define HAVE_DECL_ISNAN 1
#define HAVE_DECL_ISFINITE 1
#define HAVE_FSEEKO 1
#define HAVE_FTELLO 1
#define HAVE_FTRUNCATE 1

/* --- posix fonksiyonlari (bionic API 26'da olanlar) --- */
#define HAVE_ALARM 1
#define HAVE_CHOWN 1
#define HAVE_DUP2 1
#define HAVE_FCHDIR 1
#define HAVE_FCHMOD 1
#ifndef __APPLE__
#define HAVE_FDATASYNC 1
#endif
#define HAVE_FSYNC 1
#ifndef __APPLE__
#define HAVE_FORK 1
#endif
#define HAVE_GETCWD 1
#define HAVE_GETPID 1
#define HAVE_GETPPID 1
#define HAVE_GETUID 1
#define HAVE_GETEUID 1
#define HAVE_GETGID 1
#define HAVE_GETEGID 1
#define HAVE_KILL 1
#define HAVE_LINK 1
#define HAVE_LSTAT 1
#define HAVE_MKFIFO 1
#define HAVE_PUTENV 1
#define HAVE_READLINK 1
#define HAVE_SETSID 1
#define HAVE_SETVBUF 1
#define HAVE_SIGACTION 1
#define HAVE_SIGINTERRUPT 1
/* #undef HAVE_SIGRELSE */
#define HAVE_STRDUP 1
#define HAVE_STRERROR 1
#define HAVE_SYMLINK 1
#define HAVE_SYSCONF 1
#define HAVE_UNAME 1
#define HAVE_UNSETENV 1
#define HAVE_UTIMES 1
#define HAVE_WAIT 1
#define HAVE_WAITPID 1
#define HAVE_WCSCOLL 1
#define HAVE_MEMMOVE 1
#define HAVE_DEVICE_MACROS 1
/* Cynex: major/minor/makedev icin */
#ifndef __APPLE__
#define HAVE_SYS_SYSMACROS_H 1
#endif
#define HAVE_MAKEDEV 1
#define HAVE_MKNOD 1
#define HAVE_DIRFD 1
#ifdef __APPLE__
#define HAVE_STAT_TV_NSEC2 1
#else
#define HAVE_STAT_TV_NSEC 1
#endif
#define HAVE_SYS_TIMES_H 1
/* #undef HAVE_BROKEN_PTHREAD_SIGMASK */

/* --- thread --- */
#define WITH_THREAD 1
#ifndef _POSIX_THREADS /* Cynex: bionic'te tanimli */
#define _POSIX_THREADS 1
#endif
#define HAVE_PTHREAD_SIGMASK 1
/* #undef HAVE_SEM_OPEN */

/* --- unicode (bionic wchar_t = 4 bayt) --- */
#define Py_USING_UNICODE 1
#define Py_UNICODE_SIZE 4
#define HAVE_USABLE_WCHAR_T 1
#define PY_UNICODE_TYPE wchar_t

/* --- diger --- */
#define WITH_DOC_STRINGS 1
#define WITH_PYMALLOC 1
/* #undef HAVE_DYNAMIC_LOADING */
/* #undef SIGNED_RIGHT_SHIFT_ZERO_FILLS */
/* #undef ENABLE_IPV6 */
#define _GNU_SOURCE 1
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE 1

#endif /* Py_PYCONFIG_H */
