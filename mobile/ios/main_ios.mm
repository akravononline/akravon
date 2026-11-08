// Cynex iOS: oyun ipligi giris noktasi (android_main karsiligi). UIKit tarafi CynexPlatform_ios.mm'de.
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <execinfo.h>
#include "../platform/CynexPlatform.h"

extern "C" void CynexGdi_SetFontDirectory(const char* path);
extern "C" int Py_NoSiteFlag;
extern "C" int Py_IgnoreEnvironmentFlag;

struct HINSTANCE__;
int WinMain(HINSTANCE__* hInstance, HINSTANCE__* hPrevInstance, char* lpCmdLine, int nCmdShow);


static void CrashHandler(int sig)
{
	void* frames[48];
	const int n = backtrace(frames, 48);
	fprintf(stderr, "[F] CRASH sinyal=%d\n", sig);
	backtrace_symbols_fd(frames, n, fileno(stderr));
	fflush(stderr);
	signal(sig, SIG_DFL);
	raise(sig);
}
extern "C" int CynexIos_GameMain(void)
{
	// hata kaydi: Dosyalar > AKRAVON > akravon_log.txt
	{
		const std::string logPath = std::string(CynexPlatform_GetDataPath()) + "/akravon_log.txt";
		freopen(logPath.c_str(), "w", stderr);
		freopen(logPath.c_str(), "a", stdout);
	}
	signal(SIGSEGV, CrashHandler);
	signal(SIGABRT, CrashHandler);
	signal(SIGBUS, CrashHandler);
	signal(SIGILL, CrashHandler);
	setvbuf(stdout, nullptr, _IOLBF, 0);
	setvbuf(stderr, nullptr, _IONBF, 0);
	signal(SIGPIPE, SIG_IGN);
	fprintf(stderr, "[I] CynexMobil: Cynex Mobil (iOS) basliyor (%s)\n", __DATE__);

	// oyun klasoru = belgeler klasoru: Dosyalar uygulamasi / Finder ile "mobile", "lib", "fonts" vb. buraya konur
	const std::string dataPath = CynexPlatform_GetDataPath();
	if (chdir(dataPath.c_str()) != 0)
		fprintf(stderr, "[E] CynexMobil: oyun klasorune gecilemedi: %s\n", dataPath.c_str());
	CynexGdi_SetFontDirectory((dataPath + "/fonts").c_str());
	fprintf(stderr, "[I] CynexMobil: oyun klasoru: %s\n", dataPath.c_str());

	Py_NoSiteFlag = 1;
	Py_IgnoreEnvironmentFlag = 1;

	return WinMain(nullptr, nullptr, (char*)"", 1);
}
