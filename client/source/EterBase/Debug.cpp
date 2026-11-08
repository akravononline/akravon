#include "StdAfx.h"

#include <time.h>
#include <stdio.h>
#include "Debug.h"
#include "Singleton.h"
#include "Timer.h"

const DWORD DEBUG_STRING_MAX_LEN = 1024;

static int isLogFile = false;
HWND g_PopupHwnd = NULL;

class CLogFile : public CSingleton<CLogFile>
{
	public:
		CLogFile() : m_fp(NULL)
		{
		}

		virtual ~CLogFile()
		{
			if (m_fp)
				fclose(m_fp);

			m_fp = NULL;
		}

		void Initialize()
		{
			m_fp = fopen("log.txt", "w");
		}

		void Write(const char * c_pszMsg)
		{
			if (!m_fp)
				return;

			time_t ct = time(0);
			struct tm ctm = *localtime(&ct);

			fprintf(m_fp, "%02d%02d %02d:%02d:%05d :: %s",
				ctm.tm_mon + 1,
				ctm.tm_mday,
				ctm.tm_hour,
				ctm.tm_min,
				ELTimer_GetMSec() % 60000,
				c_pszMsg);

			fflush(m_fp);
		}

	protected:
		FILE *	m_fp;
};

static CLogFile gs_logfile;

static UINT gs_uLevel=0;

void SetLogLevel(UINT uLevel)
{
	gs_uLevel=uLevel;
}

void LogBoxf(const char* c_szFormat, ...)
{
	va_list args;
	va_start(args, c_szFormat);

	char szBuf[2048];
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);

	LogBox(szBuf);
}

void LogBox(const char* c_szMsg, const char * c_szCaption, HWND hWnd)
{
	if (!hWnd)
		hWnd = g_PopupHwnd;

	MessageBox(hWnd, c_szMsg, c_szCaption ? c_szCaption : "LOG", MB_OK);
	Tracen(c_szMsg);
}

void LogFile(const char * c_szMsg)
{
	CLogFile::Instance().Write(c_szMsg);
}

void LogFilef(const char * c_szMessage, ...)
{
	va_list args;
	va_start(args, c_szMessage);
	char szBuf[DEBUG_STRING_MAX_LEN+1];
	_vsnprintf(szBuf, sizeof(szBuf), c_szMessage, args);

	CLogFile::Instance().Write(szBuf);
}

void OpenLogFile(bool bUseLogFIle)
{
#ifndef _DISTRIBUTE
#ifndef CYNEX_MOBILE
	freopen("data/syserr.txt", "w", stderr);	// Cynex: mobilde stderr logcat'e gidiyor
#endif

	if (bUseLogFIle)
	{
		isLogFile = true;
		CLogFile::Instance().Initialize();
	}
#endif
}

void OpenConsoleWindow()
{
#ifdef CYNEX_MOBILE
	return;	// Cynex: konsol yok, stdout logcat'te
#endif
	AllocConsole();

	freopen("CONOUT$", "a", stdout);
	freopen("CONIN$", "r", stdin);
}
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
