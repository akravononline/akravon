// Cynex iOS: <sys/sysinfo.h> yerine - yalnizca toplam/bos bellek (GlobalMemoryStatus icin)
#pragma once
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <os/proc.h>

struct sysinfo
{
	unsigned long totalram;
	unsigned long freeram;
	unsigned int mem_unit;
};

static inline int sysinfo(struct sysinfo* si)
{
	memset(si, 0, sizeof(*si));
	uint64_t total = 0;
	size_t len = sizeof(total);
	sysctlbyname("hw.memsize", &total, &len, NULL, 0);
	si->totalram = (unsigned long)total;
	// ios: uygulamanin kullanabilecegi bellek (iOS 13+)
	size_t avail = 0;
	if (__builtin_available(iOS 13.0, *))
		avail = os_proc_available_memory();
	si->freeram = (unsigned long)(avail ? avail : total / 4);
	si->mem_unit = 1;
	return 0;
}
