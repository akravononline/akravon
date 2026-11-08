// Cynex: android giris noktasi (WinMain karsiligi)
#include <android_native_app_glue.h>
#include <android/log.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "../platform/CynexPlatform.h"

#define CYNEX_TAG "CynexMobil"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, CYNEX_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, CYNEX_TAG, __VA_ARGS__)

extern "C" void CynexGdi_SetFontDirectory(const char* path);
extern "C" void CynexLogBacktrace(const char* why);
extern "C" int Py_NoSiteFlag;
extern "C" int Py_IgnoreEnvironmentFlag;

// Cynex: oyunun giris noktasi (UserInterface.cpp)
struct HINSTANCE__;
int WinMain(HINSTANCE__* hInstance, HINSTANCE__* hPrevInstance, char* lpCmdLine, int nCmdShow);

namespace
{
	int g_logPipe[2];

	// Cynex: printf/stderr ciktisi logcat'e
	void* LogThread(void*)
	{
		char buf[1024];
		std::string line;
		ssize_t n;
		while ((n = read(g_logPipe[0], buf, sizeof(buf))) > 0)
		{
			for (ssize_t i = 0; i < n; ++i)
			{
				if (buf[i] == '\n')
				{
					__android_log_write(ANDROID_LOG_INFO, "CynexOyun", line.c_str());
					line.clear();
				}
				else
					line += buf[i];
			}
		}
		return nullptr;
	}

	void OnExit()
	{
		// Cynex: oyun exit() cagirirsa nereden cagirdigi gorunsun
		CynexLogBacktrace("exit cagrildi");
		fflush(stdout);
		fflush(stderr);
		usleep(300 * 1000);	// Cynex: log ipligi son satirlari yazsin
		_exit(0);	// Cynex: statik yikicilar (ses akislari) sirasiz calisip cokmesin
	}

	void RedirectStdio()
	{
		setvbuf(stdout, nullptr, _IOLBF, 0);
		setvbuf(stderr, nullptr, _IONBF, 0);
		if (pipe(g_logPipe) != 0)
			return;
		dup2(g_logPipe[1], STDOUT_FILENO);
		dup2(g_logPipe[1], STDERR_FILENO);
		pthread_t t;
		pthread_create(&t, nullptr, LogThread, nullptr);
		pthread_detach(t);
	}
}


void android_main(android_app* app)
{
	RedirectStdio();
	atexit(OnExit);
	signal(SIGPIPE, SIG_IGN);	// Cynex: kopan sokete yazinca uygulama kapanmasin
	LOGI("Cynex Mobil basliyor (%s)", __DATE__);

	CynexPlatform_Init(app);
	if (app->destroyRequested)
		return;

	// Cynex: oyun klasoru = PC istemcisiyle ayni duzen (pack, lib, locale, metin2.cfg)
	const std::string dataPath = CynexPlatform_GetDataPath();
	if (chdir(dataPath.c_str()) != 0)
		LOGE("oyun klasorune gecilemedi: %s", dataPath.c_str());
	CynexGdi_SetFontDirectory((dataPath + "/fonts").c_str());
	LOGI("oyun klasoru: %s", dataPath.c_str());

	// Cynex: site.py yok (site-packages, sysconfig gereksiz)
	Py_NoSiteFlag = 1;
	Py_IgnoreEnvironmentFlag = 1;

	const int ret = WinMain(nullptr, nullptr, (char*)"", 1);
	LOGI("Cynex Mobil kapandi (%d)", ret);

	// Cynex: oyun dongusu bitti, activity'yi de kapat
	ANativeActivity_finish(app->activity);
	while (!app->destroyRequested)
		CynexPlatform_PumpEvents(-1);
	exit(0);
}
