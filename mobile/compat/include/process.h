// Cynex: msvc process.h
#pragma once
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif
uintptr_t _beginthreadex(void* security, unsigned stackSize, unsigned (*startAddress)(void*), void* argList, unsigned initFlag, unsigned* thrdAddr);
uintptr_t _beginthread(void (*startAddress)(void*), unsigned stackSize, void* argList);
void _endthreadex(unsigned retval);
void _endthread(void);
#ifdef __cplusplus
}
#endif
