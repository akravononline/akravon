// Cynex: msvc io.h
#pragma once
#include <windows.h>
#include <fcntl.h>

struct _finddata_t
{
	unsigned attrib;
	long long time_create;
	long long time_access;
	long long time_write;
	uint32_t size;
	char name[260];
};
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_SUBDIR 0x10
#define _A_ARCH 0x20

#ifdef __cplusplus
extern "C" {
#endif
intptr_t _findfirst(const char* filespec, struct _finddata_t* fileinfo);
int _findnext(intptr_t handle, struct _finddata_t* fileinfo);
int _findclose(intptr_t handle);
#ifdef __cplusplus
}
#endif
#define _open open
#define _close close
#define _read read
#define _write write
#define _lseek lseek
