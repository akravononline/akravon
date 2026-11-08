#pragma once

#include <windows.h>
#include "Singleton.h"

class CTimer : public CSingleton<CTimer>
{
	public:
		CTimer();
		virtual ~CTimer();

		void	Advance();
		void	Adjust(int iTimeGap);
		void	SetBaseTime();

		float	GetCurrentSecond();
		DWORD	GetCurrentMillisecond();

		float	GetElapsedSecond();
		DWORD	GetElapsedMilliecond();

		void	UseCustomTime();
#ifdef ENABLE_FPS
	void	SetUpdateTime(BYTE first, BYTE sec);
#endif
	protected:
		bool	m_bUseRealTime;
		DWORD	m_dwBaseTime;
		DWORD	m_dwCurrentTime;
		float	m_fCurrentTime;
		DWORD	m_dwElapsedTime;
		int		m_index;
#ifdef ENABLE_FPS
	BYTE	m_firstArg;
	BYTE	m_SecondArg;
#endif

};

BOOL	ELTimer_Init();

DWORD	ELTimer_GetMSec();

VOID	ELTimer_SetServerMSec(DWORD dwServerTime);
DWORD	ELTimer_GetServerMSec();

VOID	ELTimer_SetFrameMSec();
DWORD	ELTimer_GetFrameMSec();
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
