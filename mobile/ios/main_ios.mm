// Cynex iOS: oyun ipligi giris noktasi (android_main karsiligi). UIKit tarafi CynexPlatform_ios.mm'de.
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <execinfo.h>
#include <dirent.h>
#include <sys/stat.h>
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

static void ListDir(const char* path)
{
	DIR* d = opendir(path);
	if (!d) { fprintf(stderr, "[D] %s: ACILAMADI\n", path); return; }
	int n = 0;
	for (dirent* e = readdir(d); e; e = readdir(d))
	{
		if (e->d_name[0] == '.') continue;
		std::string full = std::string(path) + "/" + e->d_name;
		struct stat st;
		stat(full.c_str(), &st);
		fprintf(stderr, "[D]   %s%s (%lld)\n", e->d_name, S_ISDIR(st.st_mode) ? "/" : "", (long long)st.st_size);
		++n;
	}
	closedir(d);
	fprintf(stderr, "[D] %s: %d oge\n", path, n);
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
	fprintf(stderr, "[I] AKRAVON: AKRAVON ONLINE (iOS) basliyor (%s)\n", __DATE__);

	// oyun klasoru = belgeler klasoru: Dosyalar uygulamasi / Finder ile "mobile", "lib", "fonts" vb. buraya konur
	const std::string dataPath = CynexPlatform_GetDataPath();
	if (chdir(dataPath.c_str()) != 0)
		fprintf(stderr, "[E] AKRAVON: oyun klasorune gecilemedi: %s\n", dataPath.c_str());
	CynexGdi_SetFontDirectory((dataPath + "/fonts").c_str());
	fprintf(stderr, "[I] AKRAVON: oyun klasoru: %s\n", dataPath.c_str());

	ListDir(".");
	ListDir("mobile");
	ListDir("config");
	Py_NoSiteFlag = 1;
	Py_IgnoreEnvironmentFlag = 1;

	return WinMain(nullptr, nullptr, (char*)"", 1);
}
