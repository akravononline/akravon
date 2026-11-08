// Cynex: error.cpp yerine - minidump yok, cokme sinyalinde logcat'e yigin izi
#include "../../../client/source/EterBase/StdAfx.h"
#include "../../../client/source/EterBase/error.h"

#include <signal.h>
#include <unwind.h>
#include <dlfcn.h>
#include <android/log.h>

namespace
{
	struct SBacktrace
	{
		void** cur;
		void** end;
	};

	_Unwind_Reason_Code UnwindCallback(struct _Unwind_Context* ctx, void* arg)
	{
		SBacktrace* bt = static_cast<SBacktrace*>(arg);
		uintptr_t pc = _Unwind_GetIP(ctx);
		if (pc)
		{
			if (bt->cur == bt->end)
				return _URC_END_OF_STACK;
			*bt->cur++ = reinterpret_cast<void*>(pc);
		}
		return _URC_NO_REASON;
	}

}

// Cynex: cokme / exit sirasinda yigin izi logcat'e
extern "C" void CynexLogBacktrace(const char* why)
{
	void* frames[48];
	SBacktrace bt = { frames, frames + 48 };
	_Unwind_Backtrace(UnwindCallback, &bt);

	__android_log_print(ANDROID_LOG_FATAL, "CynexMobil", "%s", why);
	for (void** p = frames; p < bt.cur; ++p)
	{
		Dl_info dl = {};
		if (dladdr(*p, &dl) && dl.dli_fname)
		{
			const uintptr_t rel = (uintptr_t)*p - (uintptr_t)dl.dli_fbase;
			__android_log_print(ANDROID_LOG_FATAL, "CynexMobil", "  #%02d %s+0x%zx %s", int(p - frames), dl.dli_fname, (size_t)rel, dl.dli_sname ? dl.dli_sname : "");
		}
		else
			__android_log_print(ANDROID_LOG_FATAL, "CynexMobil", "  #%02d %p", int(p - frames), *p);
	}
}

namespace
{
	struct sigaction s_oldActions[NSIG];

	void CrashHandler(int sig, siginfo_t* info, void* uctx)
	{
		char why[64];
		snprintf(why, sizeof(why), "COKME sinyal=%d adres=%p", sig, info ? info->si_addr : nullptr);
		CynexLogBacktrace(why);

		// Cynex: android'in kendi tombstone'u da yazilsin
		sigaction(sig, &s_oldActions[sig], nullptr);
		raise(sig);
	}
}

void SetEterExceptionHandler()
{
	struct sigaction sa = {};
	sa.sa_sigaction = CrashHandler;
	sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
	sigemptyset(&sa.sa_mask);
	for (int sig : { SIGSEGV, SIGABRT, SIGBUS, SIGFPE, SIGILL })
		sigaction(sig, &sa, &s_oldActions[sig]);
}

// Cynex: TraceError -> logcat
void CynexTraceError(const char* c_szFormat, ...)
{
	char buf[2048];
	va_list args;
	va_start(args, c_szFormat);
	vsnprintf(buf, sizeof(buf), c_szFormat, args);
	va_end(args);
	__android_log_write(ANDROID_LOG_ERROR, "CynexOyun", buf);
}
