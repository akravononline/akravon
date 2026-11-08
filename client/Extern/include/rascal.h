#ifndef RASCALAPI_H
#define RASCALAPI_H

#ifdef RASCAL_DLL_EXPORTS
#define RASCAL_API __declspec(dllexport)
#else
#define RASCAL_API __declspec(dllimport)
#endif

#pragma comment(linker, "/ALIGN:0x10000")
#pragma check_stack(off)
#include <Windows.h>
#include <vector>
#include <string>
#include <vector>
#include <intrin.h>
#include "skCrypter.h"
#include <psapi.h>
#include "C:\Users\PARS\Desktop\barboros2 pars çalişma alanı\src\client\source\EterLib\MSApplication.h"

#pragma comment(lib,"rascal.lib")

#define ImageNtHeaders(smodule) \
    (reinterpret_cast<PIMAGE_NT_HEADERS>(\
        reinterpret_cast<SIZE_T>(smodule) + \
        static_cast<SIZE_T>((reinterpret_cast<PIMAGE_DOS_HEADER>(smodule))->e_lfanew)\
    ))
	
static uintptr_t __fastcall getcalleraddr()
{
	void* caller_addr = NULL;
	ULONG frame_index = 1;
	HMODULE module = GetModuleHandle(NULL);
	PIMAGE_NT_HEADERS nt_header = ImageNtHeaders(module);
	PIMAGE_OPTIONAL_HEADER optional_header = &nt_header->OptionalHeader;
	PVOID exception_handler = (PVOID)GetProcAddress(module, skCrypt("ExceptionHandler"));
	const int max_frames = 62;
	void* frames[max_frames];

	int frame_count = CaptureStackBackTrace(1, max_frames, frames, NULL);
	if (frame_count > 1)
	{
		return reinterpret_cast<uintptr_t>(frames[1]);
	}

	return 0;
}

enum Protect
{
    ATTACK,
    ATTACKSPEED,
    SETATTACKKEY,
    MOVE,
    ATTACKINIT,
    ATTACKKEYINIT,
    GETUSERNAME,
    NOTHING
};

static int GetRascalID(DWORD threadId)
{
	CMSApplication::DThreadId = threadId;
	return threadId;
}

namespace rascal{
	extern "C" {
		RASCAL_API void FunctionProtect(void* Address);
	}
	
	extern "C" {
		RASCAL_API void GetRascalThreadID(void* thisptr);
	}
	
	extern "C" {
		RASCAL_API void WLog(std::string Content);
	}
	
	extern "C" {
		RASCAL_API void getunknown(int type);
	}
	
	extern "C" {
		RASCAL_API void RascalProtectRTypes(Protect atype, void* addr, int previousvalue);
	}
	
	extern "C" {
		RASCAL_API void InitializeRascal(std::string ServerIP, int MaxClient, std::string key);
	}

	static bool CheckModules(HMODULE hModule) {
		MODULEINFO modInfo;
		if (!GetModuleInformation((HANDLE)-1, hModule, &modInfo, sizeof(modInfo))) {
			return FALSE;
		}
		PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)modInfo.lpBaseOfDll;
		if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE) {
			return FALSE;
		}
		PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)modInfo.lpBaseOfDll + pDOSHeader->e_lfanew);
		if (pNTHeaders->Signature != IMAGE_NT_SIGNATURE) {
			return FALSE;
		}
		PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNTHeaders);
		for (int i = 0; i < pNTHeaders->FileHeader.NumberOfSections; i++) {
			if (strstr((char*)pSectionHeader[i].Name, skCrypt(".vmp"))) {
				return TRUE;
			}
		}

		return FALSE;
	}

	static void ListModulesForTheVMPSections() {
		HMODULE hMods[1024];
		DWORD cbNeeded;
		if (EnumProcessModules((HANDLE)-1, hMods, sizeof(hMods), &cbNeeded)) {
			for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
				if (CheckModules(hMods[i])) {
					char szModName[MAX_PATH];
					if (GetModuleFileNameExA((HANDLE)-1, hMods[i], szModName, sizeof(szModName) / sizeof(char))) {
						if (std::string(szModName).find(std::string(skCrypt("rascal.dll"))) == std::string::npos) {
							if (std::string(szModName).find(std::string(skCrypt("metin2client.bin"))) == std::string::npos) {
								__fastfail(0);
								exit(0);
								std::terminate();
								*((unsigned int*)0) = 0xDEAD;
							}
						}
					}
				}
			}
		}
	}

	static bool CheckRascalExports(void* address) {
		HMODULE hModule = GetModuleHandleA(skCrypt("rascal.dll"));
		PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);

		PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uint8_t*>(hModule) + dosHeader->e_lfanew);

		uintptr_t startAddr = reinterpret_cast<uintptr_t>(hModule);
		uintptr_t endAddr = startAddr + ntHeaders->OptionalHeader.SizeOfImage;
		uintptr_t addr = reinterpret_cast<uintptr_t>(address);
		return (addr >= startAddr && addr < endAddr);
	}

	template<typename A>
	static void __forceinline EncryptDecryptPointer(A& ptr) {
		uint32_t key = 0xDEADBEEF;
		uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
		addr ^= key;
		ptr = reinterpret_cast<A>(addr);
	}

	template<typename T>
	static void __forceinline EncryptDecryptPointerVector(std::vector<T>& vec) {
		uint32_t key = 0xDEADBEEF;
		for (auto& ptr : vec) {
			uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
			addr ^= key;
			ptr = reinterpret_cast<T>(addr);
		}
	}

	static float gamerecievedAtkSpeed = 0.0f;
}

static void WINAPI initialiseRascal(LPARAM lParam) {
	HMODULE hRascal = LoadLibraryA(skCrypt("rascal.dll"));
	if(hRascal != NULL){
		rascal::getunknown(99);
		rascal::InitializeRascal(std::string(skCrypt("185.128.114.130")),3,std::string(skCrypt("user=tugrabarbaros")));
		rascal::GetRascalThreadID(reinterpret_cast<void*>(GetRascalID));
	} else {
		printf("%d", *(DWORD*)*(DWORD*)*(DWORD*)*(DWORD*)0x400U);
		__fastfail(0);
		std::terminate();
		*((unsigned int*)0) = 0xDEAD;
		atexit(0);
		abort();
	}
}

#endif