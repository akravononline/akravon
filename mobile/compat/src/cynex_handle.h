// Cynex: HANDLE nesneleri ve yol cozumleme (compat ici)
#pragma once
#include <windows.h>
#include <pthread.h>
#include <string>
#include <atomic>

namespace cynex
{
	enum EHandleType
	{
		HANDLE_FILE,
		HANDLE_MAPPING,
		HANDLE_EVENT,
		HANDLE_MUTEX,
		HANDLE_SEMAPHORE,
		HANDLE_THREAD,
		HANDLE_FIND,
		HANDLE_PROCESS,
	};

	struct SHandle
	{
		explicit SHandle(EHandleType t) : type(t) {}
		virtual ~SHandle() {}
		const EHandleType type;
		std::atomic<int> refs{ 1 };
	};

	void RegisterHandle(SHandle* p);
	SHandle* GetHandle(HANDLE h, EHandleType type);
	void ReleaseHandle(SHandle* p);

	// Cynex: windows yolu -> android yolu ('\' -> '/', buyuk/kucuk harf duyarsiz cozum)
	std::string ResolvePath(const char* path, bool forCreate = false);
	std::string NormalizeSlashes(const char* path);

	void SetLastErrorFromErrno(int e);

	// bekleme - event/mutex/semaphore/thread icin ortak
	DWORD WaitHandle(SHandle* p, DWORD ms);
}
