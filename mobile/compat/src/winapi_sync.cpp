// Cynex: kernel32 senkronizasyon + thread - pthread uzerinde
#include "cynex_handle.h"
#include <process.h>
#include <sched.h>
#include <sys/resource.h>
#ifdef __APPLE__
#include <pthread.h>
#else
#include <sys/syscall.h>
#endif
#include <time.h>
#include <mutex>
#include <condition_variable>
#include <unordered_set>
#include <chrono>

namespace cynex
{
	namespace
	{
		std::mutex s_handleMutex;
		std::unordered_set<SHandle*> s_handles;

		struct SWaitable : SHandle
		{
			explicit SWaitable(EHandleType t) : SHandle(t) {}
			std::mutex m;
			std::condition_variable cv;
		};

		struct SEvent : SWaitable
		{
			SEvent() : SWaitable(HANDLE_EVENT) {}
			bool manual = false;
			bool signaled = false;
		};

		struct SMutexObj : SWaitable
		{
			SMutexObj() : SWaitable(HANDLE_MUTEX) {}
			pthread_t owner = 0;
			int count = 0;
		};

		struct SSemaphore : SWaitable
		{
			SSemaphore() : SWaitable(HANDLE_SEMAPHORE) {}
			LONG count = 0;
			LONG max = 0;
		};

		struct SThread : SWaitable
		{
			SThread() : SWaitable(HANDLE_THREAD) {}
			pthread_t tid = 0;
			pid_t ltid = 0;
			bool finished = false;
			DWORD exitCode = STILL_ACTIVE;
			unsigned (*startEx)(void*) = nullptr;
			void (*start)(void*) = nullptr;
			LPTHREAD_START_ROUTINE startWin = nullptr;
			void* arg = nullptr;
		};

		bool Wait(SWaitable* w, DWORD ms, const std::function<bool()>& ready, const std::function<void()>& consume)
		{
			std::unique_lock<std::mutex> lock(w->m);
			if (ms == INFINITE)
				w->cv.wait(lock, ready);
			else if (!w->cv.wait_for(lock, std::chrono::milliseconds(ms), ready))
				return false;
			consume();
			return true;
		}
	}

	void RegisterHandle(SHandle* p)
	{
		std::lock_guard<std::mutex> lock(s_handleMutex);
		s_handles.insert(p);
	}

	SHandle* GetHandle(HANDLE h, EHandleType type)
	{
		if (!h || h == INVALID_HANDLE_VALUE)
		{
			SetLastError(ERROR_INVALID_HANDLE);
			return nullptr;
		}
		SHandle* p = (SHandle*)h;
		std::lock_guard<std::mutex> lock(s_handleMutex);
		if (!s_handles.count(p) || p->type != type)
		{
			SetLastError(ERROR_INVALID_HANDLE);
			return nullptr;
		}
		return p;
	}

	void ReleaseHandle(SHandle* p)
	{
		if (!p)
			return;
		if (--p->refs == 0)
		{
			{
				std::lock_guard<std::mutex> lock(s_handleMutex);
				s_handles.erase(p);
			}
			delete p;
		}
	}

	DWORD WaitHandle(SHandle* p, DWORD ms)
	{
		switch (p->type)
		{
			case HANDLE_EVENT:
			{
				SEvent* e = (SEvent*)p;
				return Wait(e, ms, [e] { return e->signaled; }, [e] { if (!e->manual) e->signaled = false; }) ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
			}
			case HANDLE_SEMAPHORE:
			{
				SSemaphore* s = (SSemaphore*)p;
				return Wait(s, ms, [s] { return s->count > 0; }, [s] { --s->count; }) ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
			}
			case HANDLE_MUTEX:
			{
				SMutexObj* m = (SMutexObj*)p;
				const pthread_t self = pthread_self();
				return Wait(m, ms, [m, self] { return m->count == 0 || pthread_equal(m->owner, self); },
					[m, self] { m->owner = self; ++m->count; }) ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
			}
			case HANDLE_THREAD:
			{
				SThread* t = (SThread*)p;
				return Wait(t, ms, [t] { return t->finished; }, [] {}) ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
			}
			default:
				return WAIT_OBJECT_0;
		}
	}
}

using namespace cynex;

// Cynex: cekirdek thread kimligi (Linux gettid / Apple pthread_threadid_np)
static inline pid_t CynexGetTid()
{
#ifdef __APPLE__
	uint64_t id = 0;
	pthread_threadid_np(nullptr, &id);
	return (pid_t)id;
#else
	return (pid_t)syscall(SYS_gettid);
#endif
}


namespace
{
	void* ThreadEntry(void* p)
	{
		SThread* t = (SThread*)p;
		t->ltid = CynexGetTid();
		DWORD code = 0;
		if (t->startEx)
			code = t->startEx(t->arg);
		else if (t->startWin)
			code = t->startWin(t->arg);
		else if (t->start)
			t->start(t->arg);
		{
			std::lock_guard<std::mutex> lock(t->m);
			t->finished = true;
			t->exitCode = code;
		}
		t->cv.notify_all();
		ReleaseHandle(t);	// Cynex: thread'in kendi referansi
		return nullptr;
	}

	SThread* StartThread(SThread* t, size_t stack)
	{
		RegisterHandle(t);
		t->refs = 2;	// Cynex: biri cagiran (CloseHandle), biri thread
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_attr_setstacksize(&attr, stack ? std::max<size_t>(stack, 256 * 1024) : 1024 * 1024);
		if (pthread_create(&t->tid, &attr, ThreadEntry, t) != 0)
		{
			pthread_attr_destroy(&attr);
			t->refs = 1;
			ReleaseHandle(t);
			return nullptr;
		}
		pthread_attr_destroy(&attr);
		return t;
	}
}

extern "C" {

BOOL CloseHandle(HANDLE hObject)
{
	if (!hObject || hObject == INVALID_HANDLE_VALUE)
		return FALSE;
	ReleaseHandle((SHandle*)hObject);
	return TRUE;
}

HANDLE CreateEventA(LPSECURITY_ATTRIBUTES, BOOL bManualReset, BOOL bInitialState, LPCSTR)
{
	SEvent* e = new SEvent;
	e->manual = bManualReset != FALSE;
	e->signaled = bInitialState != FALSE;
	RegisterHandle(e);
	return e;
}

BOOL SetEvent(HANDLE hEvent)
{
	SEvent* e = (SEvent*)GetHandle(hEvent, HANDLE_EVENT);
	if (!e)
		return FALSE;
	{
		std::lock_guard<std::mutex> lock(e->m);
		e->signaled = true;
	}
	if (e->manual)
		e->cv.notify_all();
	else
		e->cv.notify_one();
	return TRUE;
}

BOOL ResetEvent(HANDLE hEvent)
{
	SEvent* e = (SEvent*)GetHandle(hEvent, HANDLE_EVENT);
	if (!e)
		return FALSE;
	std::lock_guard<std::mutex> lock(e->m);
	e->signaled = false;
	return TRUE;
}

HANDLE CreateMutexA(LPSECURITY_ATTRIBUTES, BOOL bInitialOwner, LPCSTR)
{
	SMutexObj* m = new SMutexObj;
	if (bInitialOwner)
	{
		m->owner = pthread_self();
		m->count = 1;
	}
	RegisterHandle(m);
	SetLastError(0);
	return m;
}

HANDLE OpenMutexA(DWORD, BOOL, LPCSTR)
{
	return NULL;
}

BOOL ReleaseMutex(HANDLE hMutex)
{
	SMutexObj* m = (SMutexObj*)GetHandle(hMutex, HANDLE_MUTEX);
	if (!m)
		return FALSE;
	{
		std::lock_guard<std::mutex> lock(m->m);
		if (m->count == 0 || !pthread_equal(m->owner, pthread_self()))
			return FALSE;
		--m->count;
	}
	m->cv.notify_one();
	return TRUE;
}

HANDLE CreateSemaphoreA(LPSECURITY_ATTRIBUTES, LONG lInitialCount, LONG lMaximumCount, LPCSTR)
{
	SSemaphore* s = new SSemaphore;
	s->count = lInitialCount;
	s->max = lMaximumCount;
	RegisterHandle(s);
	return s;
}

BOOL ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LPLONG lpPreviousCount)
{
	SSemaphore* s = (SSemaphore*)GetHandle(hSemaphore, HANDLE_SEMAPHORE);
	if (!s)
		return FALSE;
	{
		std::lock_guard<std::mutex> lock(s->m);
		if (lpPreviousCount)
			*lpPreviousCount = s->count;
		if (s->count + lReleaseCount > s->max)
			return FALSE;
		s->count += lReleaseCount;
	}
	s->cv.notify_all();
	return TRUE;
}

DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds)
{
	if (!hHandle || hHandle == INVALID_HANDLE_VALUE)
		return WAIT_FAILED;
	SHandle* p = (SHandle*)hHandle;
	{
		std::lock_guard<std::mutex> lock(cynex::s_handleMutex);
		if (!cynex::s_handles.count(p))
			return WAIT_FAILED;
		++p->refs;
	}
	const DWORD r = WaitHandle(p, dwMilliseconds);
	ReleaseHandle(p);
	return r;
}

DWORD WaitForMultipleObjects(DWORD nCount, const HANDLE* lpHandles, BOOL bWaitAll, DWORD dwMilliseconds)
{
	// Cynex: oyunda nadir - kisa araliklarla yokla
	const auto start = std::chrono::steady_clock::now();
	for (;;)
	{
		DWORD done = 0;
		for (DWORD i = 0; i < nCount; ++i)
		{
			if (WaitForSingleObject(lpHandles[i], 0) == WAIT_OBJECT_0)
			{
				if (!bWaitAll)
					return WAIT_OBJECT_0 + i;
				++done;
			}
		}
		if (bWaitAll && done == nCount)
			return WAIT_OBJECT_0;
		if (dwMilliseconds != INFINITE && std::chrono::steady_clock::now() - start >= std::chrono::milliseconds(dwMilliseconds))
			return WAIT_TIMEOUT;
		usleep(1000);
	}
}

HANDLE CreateThread(LPSECURITY_ATTRIBUTES, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD, LPDWORD lpThreadId)
{
	SThread* t = new SThread;
	t->startWin = lpStartAddress;
	t->arg = lpParameter;
	if (!StartThread(t, dwStackSize))
		return NULL;
	if (lpThreadId)
		*lpThreadId = (DWORD)(uintptr_t)t;
	return t;
}

uintptr_t _beginthreadex(void*, unsigned stackSize, unsigned (*startAddress)(void*), void* argList, unsigned, unsigned* thrdAddr)
{
	SThread* t = new SThread;
	t->startEx = startAddress;
	t->arg = argList;
	if (!StartThread(t, stackSize))
		return 0;
	if (thrdAddr)
		*thrdAddr = (unsigned)(uintptr_t)t;
	return (uintptr_t)t;
}

uintptr_t _beginthread(void (*startAddress)(void*), unsigned stackSize, void* argList)
{
	SThread* t = new SThread;
	t->start = startAddress;
	t->arg = argList;
	if (!StartThread(t, stackSize))
		return (uintptr_t)-1;
	// Cynex: _beginthread tutamagi kapatilmaz
	ReleaseHandle(t);
	return (uintptr_t)t;
}

void _endthreadex(unsigned retval)
{
	pthread_exit(nullptr);
}

void _endthread(void)
{
	pthread_exit(nullptr);
}

BOOL SetThreadPriority(HANDLE hThread, int nPriority)
{
	SThread* t = (SThread*)GetHandle(hThread, HANDLE_THREAD);
	// Cynex: nice degeri - yuksek oncelik = dusuk nice
	const int nice = nPriority >= THREAD_PRIORITY_TIME_CRITICAL ? -8 : -nPriority * 2;
	const pid_t tid = t ? t->ltid : CynexGetTid();
#ifdef __APPLE__
	(void)tid; (void)nice;	// ios: thread basina nice yok
	return TRUE;
#else
	return setpriority(PRIO_PROCESS, tid, nice) == 0;
#endif
}

int GetThreadPriority(HANDLE)
{
	return THREAD_PRIORITY_NORMAL;
}

DWORD ResumeThread(HANDLE)
{
	return 0;
}

DWORD SuspendThread(HANDLE)
{
	return (DWORD)-1;
}

BOOL TerminateThread(HANDLE, DWORD)
{
	return FALSE;
}

BOOL GetExitCodeThread(HANDLE hThread, LPDWORD lpExitCode)
{
	SThread* t = (SThread*)GetHandle(hThread, HANDLE_THREAD);
	if (!t)
		return FALSE;
	std::lock_guard<std::mutex> lock(t->m);
	*lpExitCode = t->finished ? t->exitCode : STILL_ACTIVE;
	return TRUE;
}

HANDLE GetCurrentThread(void)
{
	return (HANDLE)(intptr_t)-2;
}

DWORD GetCurrentThreadId(void)
{
	return (DWORD)CynexGetTid();
}

HANDLE GetCurrentProcess(void)
{
	return (HANDLE)(intptr_t)-1;
}

DWORD GetCurrentProcessId(void)
{
	return (DWORD)getpid();
}

extern "C" void CynexLogBacktrace(const char* why);

void ExitProcess(UINT uExitCode)
{
	CynexLogBacktrace("ExitProcess");
	_exit((int)uExitCode);
}

BOOL TerminateProcess(HANDLE, UINT uExitCode)
{
	CynexLogBacktrace("TerminateProcess");
	_exit((int)uExitCode);
}

void InitializeCriticalSection(LPCRITICAL_SECTION cs)
{
	pthread_mutex_t* m = new pthread_mutex_t;
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(m, &attr);
	pthread_mutexattr_destroy(&attr);
	cs->impl = m;
}

BOOL InitializeCriticalSectionAndSpinCount(LPCRITICAL_SECTION cs, DWORD)
{
	InitializeCriticalSection(cs);
	return TRUE;
}

void DeleteCriticalSection(LPCRITICAL_SECTION cs)
{
	if (!cs->impl)
		return;
	pthread_mutex_destroy((pthread_mutex_t*)cs->impl);
	delete (pthread_mutex_t*)cs->impl;
	cs->impl = nullptr;
}

void EnterCriticalSection(LPCRITICAL_SECTION cs)
{
	if (!cs->impl)
		InitializeCriticalSection(cs);
	pthread_mutex_lock((pthread_mutex_t*)cs->impl);
}

BOOL TryEnterCriticalSection(LPCRITICAL_SECTION cs)
{
	if (!cs->impl)
		InitializeCriticalSection(cs);
	return pthread_mutex_trylock((pthread_mutex_t*)cs->impl) == 0;
}

void LeaveCriticalSection(LPCRITICAL_SECTION cs)
{
	if (cs->impl)
		pthread_mutex_unlock((pthread_mutex_t*)cs->impl);
}

LONG InterlockedIncrement(volatile LONG* p)
{
	return __atomic_add_fetch(p, 1, __ATOMIC_SEQ_CST);
}

LONG InterlockedDecrement(volatile LONG* p)
{
	return __atomic_sub_fetch(p, 1, __ATOMIC_SEQ_CST);
}

LONG InterlockedExchange(volatile LONG* p, LONG v)
{
	return __atomic_exchange_n(p, v, __ATOMIC_SEQ_CST);
}

LONG InterlockedExchangeAdd(volatile LONG* p, LONG v)
{
	return __atomic_fetch_add(p, v, __ATOMIC_SEQ_CST);
}

LONG InterlockedCompareExchange(volatile LONG* p, LONG exchange, LONG comparand)
{
	__atomic_compare_exchange_n(p, &comparand, exchange, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
	return comparand;
}

}	// extern "C"
