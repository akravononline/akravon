// Cynex: ip/mac tablosu - mobilde tek satir, cihaza ozel kalici adres (CynexDevice)
#pragma once
#include <windows.h>

#define MAXLEN_PHYSADDR 8
typedef struct _MIB_IPNETROW
{
	DWORD dwIndex;
	DWORD dwPhysAddrLen;
	BYTE bPhysAddr[MAXLEN_PHYSADDR];
	DWORD dwAddr;
	DWORD dwType;
} MIB_IPNETROW, *PMIB_IPNETROW;

typedef struct _MIB_IPNETTABLE
{
	DWORD dwNumEntries;
	MIB_IPNETROW table[1];
} MIB_IPNETTABLE, *PMIB_IPNETTABLE;

#ifdef __cplusplus
extern "C" {
#endif
DWORD GetIpNetTable(PMIB_IPNETTABLE IpNetTable, PULONG SizePointer, BOOL Order);
#ifdef __cplusplus
}
#endif
