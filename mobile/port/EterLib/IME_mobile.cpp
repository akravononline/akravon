// Cynex: EterLib/IME.cpp mobil karsiligi - tools/gen_ime_mobile.py uretti

#include "../../../client/source/EterLib/StdAfx.h"
#include "../../platform/CynexPlatform.h"
#include "../../../client/source/EterLib/IME.h"
#include "../../../client/source/EterLib/TextTag.h"
#include "../../../client/source/EterLib/../eterBase/Utils.h"

#define COUNTOF(a)						( sizeof( a ) / sizeof( ( a )[0] ) )

int CIME::ms_compLen;
int CIME::ms_curpos;
int CIME::ms_lastpos;
wchar_t	CIME::m_wText[IMESTR_MAXLEN];

#define MAKEIMEVERSION(major, minor)	((DWORD)(((BYTE)(major) << 24) | ((BYTE)(minor) << 16)))
#define IMEID_VER(dwId)					((dwId) & 0xffff0000)
#define IMEID_LANG(dwId)				((dwId) & 0x0000ffff)

#define GETLANG()						LOWORD(CIME::ms_hklCurrent)
#define GETPRIMLANG()					((WORD)PRIMARYLANGID(GETLANG()))
#define GETSUBLANG()					SUBLANGID(GETLANG())

#define LANG_CHT						MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL)
#define LANG_CHS						MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED)

// Chinese Traditional
#define _CHT_HKL_DAYI					((HKL)0xE0060404)	// DaYi
#define _CHT_HKL_NEW_PHONETIC			((HKL)0xE0080404)	// New Phonetic
#define _CHT_HKL_NEW_CHANG_JIE			((HKL)0xE0090404)	// New Chang Jie
#define _CHT_HKL_NEW_QUICK				((HKL)0xE00A0404)	// New Quick
#define _CHT_HKL_HK_CANTONESE			((HKL)0xE00B0404)	// Hong Kong Cantonese

#define CHT_IMEFILENAME1				"TINTLGNT.IME" // New Phonetic
#define CHT_IMEFILENAME2				"CINTLGNT.IME" // New Chang Jie
#define CHT_IMEFILENAME3				"MSTCIPHA.IME" // Phonetic 5.1

#define IMEID_CHT_VER42					(LANG_CHT | MAKEIMEVERSION(4, 2))	// New(Phonetic/ChanJie)IME98  : 4.2.x.x // Win98
#define IMEID_CHT_VER43					(LANG_CHT | MAKEIMEVERSION(4, 3))	// New(Phonetic/ChanJie)IME98a : 4.3.x.x // Win2k
#define IMEID_CHT_VER44					(LANG_CHT | MAKEIMEVERSION(4, 4))	// New ChanJie IME98b          : 4.4.x.x // WinXP
#define IMEID_CHT_VER50					(LANG_CHT | MAKEIMEVERSION(5, 0))	// New(Phonetic/ChanJie)IME5.0 : 5.0.x.x // WinME
#define IMEID_CHT_VER51					(LANG_CHT | MAKEIMEVERSION(5, 1))	// New(Phonetic/ChanJie)IME5.1 : 5.1.x.x // IME2002(w/OfficeXP)
#define IMEID_CHT_VER52					(LANG_CHT | MAKEIMEVERSION(5, 2))	// New(Phonetic/ChanJie)IME5.2 : 5.2.x.x // IME2002a(w/Whistler)
#define IMEID_CHT_VER60					(LANG_CHT | MAKEIMEVERSION(6, 0))	// New(Phonetic/ChanJie)IME6.0 : 6.0.x.x // IME XP(w/WinXP SP1)
#define IMEID_CHT_VER_VISTA				(LANG_CHT | MAKEIMEVERSION(7, 0))	// All TSF TIP under Cicero UI-less mode: a hack to make GetImeId() return non-zero value

// Chinese Simplized
#define _CHS_HKL						((HKL)0xE00E0804) // MSPY
#define _CHS_HKL_QQPINYIN				((HKL)0xE0210804) // QQ PinYin
#define _CHS_HKL_SOGOU					((HKL)0xE0220804) // Sougou PinYin
#define _CHS_HKL_GOOGLEPINYIN			((HKL)0xE0230804) // Google PinYin

#define CHS_IMEFILENAME1			    "PINTLGNT.IME"		// MSPY1.5/2/3
#define CHS_IMEFILENAME2			    "MSSCIPYA.IME"		// MSPY3 for OfficeXP
#define CHS_IMEFILENAME_QQPINYIN		"QQPINYIN.IME"		// QQ PinYin
#define CHS_IMEFILENAME_SOGOUPY			"SOGOUPY.IME"		// Sougou PinYin
#define CHS_IMEFILENAME_GOOGLEPINYIN2	"GOOGLEPINYIN2.IME"	// Google PinYin 2

#define IMEID_CHS_VER41					(LANG_CHS | MAKEIMEVERSION(4, 1))	// MSPY1.5	// SCIME97 or MSPY1.5 (w/Win98, Office97)
#define IMEID_CHS_VER42					(LANG_CHS | MAKEIMEVERSION(4, 2))	// MSPY2	// Win2k/WinME
#define IMEID_CHS_VER53					(LANG_CHS | MAKEIMEVERSION(5, 3))	// MSPY3	// WinXP

enum { INDICATOR_NON_IME, INDICATOR_CHS, INDICATOR_CHT, INDICATOR_KOREAN, INDICATOR_JAPANESE };
enum { IMEUI_STATE_OFF, IMEUI_STATE_ON, IMEUI_STATE_ENGLISH };

#define LCID_INVARIANT MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

wchar_t s_aszIndicator[5][3] =
{
	L"En",
	L"\x7B80",
	L"\x7E41",
	L"\xAC00",
	L"\x3042",
};

INPUTCONTEXT*	(WINAPI * CIME::_ImmLockIMC)( HIMC );
BOOL			(WINAPI * CIME::_ImmUnlockIMC)( HIMC );
LPVOID			(WINAPI * CIME::_ImmLockIMCC)( HIMCC );
BOOL			(WINAPI * CIME::_ImmUnlockIMCC)( HIMCC );

UINT			(WINAPI * CIME::_GetReadingString)( HIMC, UINT, LPWSTR, PINT, BOOL*, PUINT );
BOOL			(WINAPI * CIME::_ShowReadingWindow)( HIMC, BOOL );

bool CIME::ms_bInitialized = false;
bool CIME::ms_bDisableIMECompletely = false;
bool CIME::ms_bImeEnabled = false;
bool CIME::ms_bUILessMode = false;
bool CIME::ms_bCaptureInput = false;
bool CIME::ms_bChineseIME = false;
bool CIME::ms_bUseIMMCandidate = false;

HWND CIME::ms_hWnd;
HKL	CIME::ms_hklCurrent;
char CIME::ms_szKeyboardLayout[KL_NAMELENGTH+1];
OSVERSIONINFOA CIME::ms_stOSVI;

HINSTANCE CIME::ms_hImm32Dll;
HINSTANCE CIME::ms_hCurrentImeDll;
DWORD CIME::ms_dwImeState;

DWORD CIME::ms_adwId[2] = { 0, 0 };

// IME Level
DWORD CIME::ms_dwIMELevel;
DWORD CIME::ms_dwIMELevelSaved;

// Candidate List
bool CIME::ms_bCandidateList;
DWORD CIME::ms_dwCandidateCount;
bool CIME::ms_bVerticalCandidate;
int CIME::ms_iCandListIndexBase;
WCHAR CIME::ms_wszCandidate[CIME::MAX_CANDLIST][MAX_CANDIDATE_LENGTH];
DWORD CIME::ms_dwCandidateSelection;
DWORD CIME::ms_dwCandidatePageSize;

// Reading Window
bool CIME::ms_bReadingInformation;
int CIME::ms_iReadingError = 0;
bool CIME::ms_bHorizontalReading;
std::vector<wchar_t>	CIME::ms_wstrReading;

// Indicator
wchar_t* CIME::ms_wszCurrentIndicator;

IIMEEventSink* CIME::ms_pEvent;

int CIME::ms_ulbegin;
int CIME::ms_ulend;

UINT CIME::ms_uOutputCodePage = 0;
UINT CIME::ms_uInputCodePage = 0;

extern DWORD gs_codePage=0;
extern DWORD GetDefaultCodePage();
extern int ReadToken(const char* token);
extern const char* FindToken(const char* begin, const char* end);

///////////////////////////////////////////////////////////////////////////////
// Cynex: mobilde TSF yok - ayni arayuzle bos siniflar
class CTsfUiLessMode
{
public:
	static BOOL SetupSinks() { return FALSE; }
	static void ReleaseSinks() {}
	static BOOL CurrentInputLocaleIsIme() { return FALSE; }
	static void UpdateImeState(BOOL bResetCompartmentEventSink = FALSE) {}
	static void EnableUiUpdates(bool bEnable) {}
};

class CDisableCicero
{
public:
	void Initialize() {}
	void Uninitialize() {}
	void DisableCiceroOnThisWnd(HWND hwnd) {}
};
static CDisableCicero g_disableCicero;

/*---------------------------------------------------------------------------*/ /* Public */
CIME::CIME()
{
	ms_hWnd = NULL;

	ms_bCandidateList = false;
	ms_bReadingInformation = false;

	Clear();

	m_max = 0;
	m_userMax = 0;

	m_bOnlyNumberMode = FALSE;
	m_hOrgIMC = NULL;

	m_bEnablePaste = false;
	m_bUseDefaultIME = false;
}

CIME::~CIME()
{
	SAFE_FREE_LIBRARY(ms_hCurrentImeDll);
	SAFE_FREE_LIBRARY(ms_hImm32Dll);
}

bool CIME::Initialize(HWND hWnd)
{
	if(ms_bInitialized)
		return true;
	ms_hWnd = hWnd;

	g_disableCicero.Initialize();

	ms_stOSVI.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
	GetVersionExA(&ms_stOSVI);

	bool bUnicodeImm = false;
	// IMM in NT or Win98 supports Unicode
	if ( ms_stOSVI.dwPlatformId == VER_PLATFORM_WIN32_NT ||
		( ms_stOSVI.dwMajorVersion > 4 ) ||
		( ms_stOSVI.dwMajorVersion == 4 ) && ( ms_stOSVI.dwMinorVersion > 0 ) ) {
		bUnicodeImm = true;
	}

	// Load ImmLock/ImmUnlock Function Proc
    CHAR szPath[MAX_PATH+1];
	ms_bDisableIMECompletely = false;

	if(GetSystemDirectoryA(szPath, MAX_PATH+1)) {
		strcat(szPath, "\\imm32.dll");
		ms_hImm32Dll = LoadLibraryA(szPath);
		if(ms_hImm32Dll)
		{
			_ImmLockIMC		= (INPUTCONTEXT*(WINAPI *)(HIMC))	GetProcAddress(ms_hImm32Dll, "ImmLockIMC");
			_ImmUnlockIMC	= (BOOL(WINAPI *)(HIMC))			GetProcAddress(ms_hImm32Dll, "ImmUnlockIMC");
			_ImmLockIMCC	= (LPVOID(WINAPI *)(HIMCC))			GetProcAddress(ms_hImm32Dll, "ImmLockIMCC");
			_ImmUnlockIMCC	= (BOOL(WINAPI *)(HIMCC))			GetProcAddress(ms_hImm32Dll, "ImmUnlockIMCC");
			BOOL (WINAPI* _ImmDisableTextFrameService)(DWORD) = (BOOL (WINAPI*)(DWORD))GetProcAddress(ms_hImm32Dll, "ImmDisableTextFrameService");
			if ( _ImmDisableTextFrameService )
				_ImmDisableTextFrameService( (DWORD)-1 );
		} else {
			ms_bDisableIMECompletely = true;
		}
	}

	ms_bInitialized = true;

	m_hOrgIMC = ImmGetContext( ms_hWnd );
	ImmReleaseContext( ms_hWnd, m_hOrgIMC );

	CheckInputLocale();
	ChangeInputLanguageWorker();
	SetSupportLevel(2);

	ms_bUILessMode = CTsfUiLessMode::SetupSinks() != FALSE;
	CheckToggleState();
	if ( ms_bUILessMode )
	{
		ms_bChineseIME = ( GETPRIMLANG() == LANG_CHINESE ) && CTsfUiLessMode::CurrentInputLocaleIsIme();
		CTsfUiLessMode::UpdateImeState();
	}

	return true;
}

void CIME::Uninitialize()
{
	if ( !ms_bInitialized )
		return;
	CTsfUiLessMode::ReleaseSinks();
	if ( ms_hWnd )
		ImmAssociateContext(ms_hWnd, m_hOrgIMC);
	ms_hWnd = NULL;
	m_hOrgIMC = NULL;
	SAFE_FREE_LIBRARY(ms_hCurrentImeDll);
	SAFE_FREE_LIBRARY(ms_hImm32Dll);
	g_disableCicero.Uninitialize();
	ms_bInitialized = false;
}

void CIME::UseDefaultIME()
{
	m_bUseDefaultIME = true;
}

bool CIME::IsIMEEnabled()
{
	return ms_bImeEnabled;
}

void CIME::EnableIME(bool bEnable)
{
	if (!ms_bInitialized || !ms_hWnd)
		return;
	if (ms_bDisableIMECompletely)
		bEnable = false;
	ImmAssociateContext(ms_hWnd, bEnable ? m_hOrgIMC : NULL);
	ms_bImeEnabled = bEnable;
	if (bEnable)
		CheckToggleState();
	CTsfUiLessMode::EnableUiUpdates(bEnable);
}

void CIME::DisableIME()
{
	EnableIME(false);
}

void CIME::EnableCaptureInput()
{
	CynexPlatform_ShowKeyboard(1);
	ms_bCaptureInput = true;
}

void CIME::DisableCaptureInput()
{
	CynexPlatform_ShowKeyboard(0);
	ms_bCaptureInput = false;
}

bool CIME::IsCaptureEnabled()
{
	return ms_bCaptureInput;
}

void CIME::Clear()
{
	ms_lastpos = 0;
	ms_curpos = 0;

	ms_compLen = 0;
	ms_ulbegin = 0;
	ms_ulend = 0;
}

int CIME::GetReading(std::string & rstrText)
{
	char reading[IMEREADING_MAXLEN];

	if(ms_wstrReading.size() == 0)
		return 0;
	int readingLen = WideCharToMultiByte(ms_uOutputCodePage, 0, &ms_wstrReading[0], ms_wstrReading.size(), reading, sizeof(reading), NULL, NULL);

	rstrText.append(GetCodePageText());
	rstrText.append(reading, reading + readingLen);

	return rstrText.size();
}

int CIME::GetReadingError()
{
	return ms_iReadingError;
}

void CIME::SetMax(int iMax)
{
	m_max = iMax;
}

void CIME::SetUserMax(int iMax)
{
	m_userMax = iMax;
}

void CIME::SetText(const char* szText, int len)
{
	ms_compLen = 0;
	ms_ulbegin = 0;
	ms_ulend = 0;

	const char* begin = szText;
	const char* end = begin + len;
	const char* iter = FindToken(begin, end);

	int m_wTextLen = sizeof(m_wText)/sizeof(wchar_t);

	ms_lastpos = MultiByteToWideChar(ms_uInputCodePage, 0, begin, iter-begin, m_wText, m_wTextLen);

	if (iter < end)
		ms_lastpos += MultiByteToWideChar(ReadToken(iter), 0, (iter+5), end-(iter+5), m_wText+ms_lastpos, m_wTextLen-ms_lastpos);

	ms_curpos = min(ms_curpos, ms_lastpos);
}

int  CIME::GetText(std::string & rstrText, bool addCodePage)
{
	int outCodePage = ms_uOutputCodePage;
	int dataCodePage;
	switch (outCodePage)
	{
		//case 1256: // ARABIC
		case 1268: // VIETNAM
			dataCodePage = CP_UTF8;
			break;
		default:
			dataCodePage = outCodePage;
	}

	int len = 0;
	char text[IMESTR_MAXLEN];

	len += WideCharToMultiByte(dataCodePage, 0, m_wText, ms_curpos, text, sizeof(text)-len, NULL, NULL);
	len += WideCharToMultiByte(dataCodePage, 0, m_wszComposition, ms_compLen, text+len, sizeof(text)-len, NULL, NULL);
	len += WideCharToMultiByte(dataCodePage, 0, m_wText+ms_curpos, ms_lastpos-ms_curpos, text+len, sizeof(text)-len, NULL, NULL);

	int i;
	for(i=0; i<len; ++i)
		if((BYTE)text[i] > 0x7F) break;

	if(i == len)
	{
		rstrText.append(text, text+len);
	}
	else
	{
		rstrText.append(text, text+i);

		//if (addCodePage)
		//	rstrText.append(GetCodePageText());

		rstrText.append(text+i, text+len);
	}

	return rstrText.size();
}

const char* CIME::GetCodePageText()
{
	static char szCodePage[16];

	const int defCodePage = GetDefaultCodePage();
	const int outCodePage = ms_uOutputCodePage;

	if (outCodePage != defCodePage)
	{
		sprintf(szCodePage, "@%04d", outCodePage);
	}
	else
	{
		szCodePage[0] = 0;
	}

	return szCodePage;
}

int CIME::GetCodePage()
{
	return ms_uOutputCodePage;
}

int CIME::GetCandidatePageCount()
{
	return ms_dwCandidatePageSize;
}

int CIME::GetCandidateCount()
{
	return ms_dwCandidateCount;
}

int  CIME::GetCandidate(DWORD index, std::string & rstrText)
{
	if(index >= MAX_CANDLIST)
		return 0;

	LPCWSTR wszText = ms_wszCandidate[index];
	if(wszText == NULL)
		return 0;

	int wTextLen = wcslen(wszText);
	if(wTextLen == 0)
		return 0;

	char text[IMESTR_MAXLEN];
	int len = ::WideCharToMultiByte(CP_UTF8, 0, wszText, wTextLen, text, sizeof(text), 0, 0);

	rstrText.append("@9999");
	rstrText.append(text, text+len);

	return wTextLen;
}

int CIME::GetCandidateSelection()
{
	return ms_dwCandidateSelection;
}

void CIME::SetInputMode(DWORD dwMode)
{
	HIMC hImc = ImmGetContext(ms_hWnd);

	ImmSetConversionStatus(hImc, dwMode, IME_SMODE_AUTOMATIC);

	ImmReleaseContext(ms_hWnd, hImc);
}

DWORD CIME::GetInputMode()
{
	DWORD dwCMode, dwSMode;

	HIMC hImc = ImmGetContext(ms_hWnd);

	ImmGetConversionStatus(hImc, &dwCMode, &dwSMode);

	ImmReleaseContext(ms_hWnd, hImc);

	return dwCMode;
}

void CIME::SetNumberMode()
{
	m_bOnlyNumberMode = TRUE;
}

void CIME::SetStringMode()
{
	m_bOnlyNumberMode = FALSE;
}

void CIME::AddExceptKey(wchar_t key)
{
	m_exceptKey.push_back(key);
}

void CIME::ClearExceptKey()
{
	m_exceptKey.clear();
}

bool CIME::__IsWritable(wchar_t key)
{
	if ( m_exceptKey.end() == std::find(m_exceptKey.begin(),m_exceptKey.end(),key) )
		return true;
	else
		return false;
}

void CIME::EnablePaste(bool bFlag)
{
	m_bEnablePaste = bFlag;
}

void CIME::PasteTextFromClipBoard()
{
	if (!m_bEnablePaste)
		return;

	if (!OpenClipboard(NULL))
		return;

	HANDLE handle = GetClipboardData(CF_TEXT);
	if (!handle) // @fixme008
	{
		CloseClipboard();
		return;
	}
	char * buffer = (char*)GlobalLock(handle);
	if (!buffer) // @fixme008
	{
		GlobalUnlock(handle);
		CloseClipboard();
		return;
	}
	std::string strClipboard = buffer;
	GlobalUnlock(handle);
	CloseClipboard();

	if (strClipboard.empty())
		return;

	const char* begin = strClipboard.c_str();
	const char* end = begin + strClipboard.length();
	wchar_t m_wText[IMESTR_MAXLEN];
	int wstrLen = MultiByteToWideChar(ms_uInputCodePage, 0, begin, end-begin, m_wText, IMESTR_MAXLEN);
	if (wstrLen <= 0) // @fixme008
		return;

	InsertString(m_wText, wstrLen);
	if(ms_pEvent)
		ms_pEvent->OnUpdate();
}

void CIME::FinalizeString(bool bSend)
{
	HIMC himc;
	static bool s_bProcessing = false; // to avoid infinite recursion
	if ( !ms_bInitialized || s_bProcessing || NULL == ( himc = ImmGetContext( ms_hWnd ) ) )
		return;
	s_bProcessing = true;

	if (ms_dwIMELevel == 2 && bSend)
	{
		//// Send composition string to app.
		//LONG lRet = lstrlenW( m_wszComposition );
		////assert( lRet >= 2);
		//// In case of CHT IME, don't send the trailing double byte space, if it exists.
		//if ( GETLANG() == LANG_CHT && (lRet >= 1)
		//	&& m_wszComposition[lRet - 1] == 0x3000 )
		//{
		//	lRet--;
		//}
		//SendCompString();
	}

	//InitCompStringData();
	// clear composition string in IME
	ImmNotifyIME(himc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
	if (ms_bUILessMode)
	{
		// For some reason ImmNotifyIME doesn't work on DaYi and Array CHT IMEs. Cancel composition string by setting zero-length string.
		ImmSetCompositionStringW(himc, SCS_SETSTR, nullptr, sizeof(wchar_t), nullptr, sizeof(wchar_t));
	}
	// the following line is necessary as Korean IME doesn't close cand list when comp string is cancelled.
	ImmNotifyIME( himc, NI_CLOSECANDIDATE, 0, 0 );
	ImmReleaseContext(ms_hWnd, himc);
	// Zooty2 RAID #4759: Sometimes application doesn't receive IMN_CLOSECANDIDATE on Alt+Tab
	// So the same code for IMN_CLOSECANDIDATE is replicated here.
	CloseCandidateList();
	s_bProcessing = false;
}

int CIME::GetCompLen()
{
	return ms_compLen;
}

int CIME::GetULBegin()
{
	return ms_ulbegin;
}

int CIME::GetULEnd()
{
	return ms_ulend;
}

void CIME::CloseCandidateList()
{
	ms_bCandidateList = false;
	ms_dwCandidateCount = 0;
	memset(&ms_wszCandidate, 0, sizeof(ms_wszCandidate));
	if(ms_pEvent)
		ms_pEvent->OnCloseCandidateList();
}

void CIME::CloseReadingInformation()
{
	CIME::ms_bReadingInformation = false;
	if(CIME::ms_pEvent)
		CIME::ms_pEvent->OnCloseReadingWnd();
}

void CIME::ChangeInputLanguage()
{
	UINT uLanguage = (UINT) GETLANG();
	CheckToggleState();
	ChangeInputLanguageWorker();
	if (uLanguage != GETLANG())
	{
		// Korean IME always uses level 3 support.
		// Other languages use the level that is specified by ImeUi_SetSupportLevel()
		SetSupportLevel( ( GETPRIMLANG() == LANG_KOREAN ) ? 3 : ms_dwIMELevelSaved );
	}

	if(ms_pEvent)
		ms_pEvent->OnChangeCodePage();

	//HWND hwndImeDef = ImmGetDefaultIMEWnd(ms_hWnd);
	//if ( hwndImeDef )
	//{
	//	// Fix for Zooty #3995: prevent CHT IME toobar from showing up
	//	SendMessageA(hwndImeDef, WM_IME_CONTROL, IMC_OPENSTATUSWINDOW, 0);
	//	SendMessageA(hwndImeDef, WM_IME_CONTROL, IMC_CLOSESTATUSWINDOW, 0);
	//}
}

void CIME::ChangeInputLanguageWorker()
{
	if ( !ms_bUILessMode )
		ms_iCandListIndexBase = ( ms_hklCurrent == _CHT_HKL_DAYI ) ? 0 : 1;
	SetupImeApi();
}

void CIME::SetSupportLevel( DWORD dwImeLevel )
{
	if ( dwImeLevel < 2 || 3 < dwImeLevel )
		return;
	if ( GETPRIMLANG() == LANG_KOREAN )
	{
		dwImeLevel = 3;
	}
	ms_dwIMELevel = dwImeLevel;
	// cancel current composition string.
	FinalizeString();
	//SetCompStringColor();
}

/*---------------------------------------------------------------------------*/ /* Protected */
void CIME::IncCurPos()
{
	if (ms_curpos < ms_lastpos)
	{
		int pos = FindColorTagEndPosition(m_wText + ms_curpos, ms_lastpos - ms_curpos);

		if (pos > 0)
			ms_curpos = min(ms_lastpos, max(0, ms_curpos + (pos + 1)));
		else
			++ms_curpos;
		//++ms_curpos;
	}
}

void CIME::DecCurPos()
{
	if (ms_curpos > 0)
	{
		int pos = FindColorTagStartPosition(m_wText + ms_curpos - 1, ms_curpos);

		if (pos > 0)
			ms_curpos = min(ms_lastpos, max(0, ms_curpos - (pos + 1)));
		else
			--ms_curpos;
		//--ms_curpos;
	}
}

int CIME::GetCurPos()
{
	int pos = GetTextTagOutputLen(m_wText, ms_curpos);
	return pos;
	//return ms_curpos;
}

void CIME::SetCurPos(int offset)
{
	if (offset < 0 || offset >= ms_lastpos) // @fixme048
	{
		ms_curpos = ms_lastpos;
		return;
	}
	else
	{
		//ms_curpos = min(ms_lastpos, offset);
		ms_curpos = min(ms_lastpos, GetTextTagInternalPosFromRenderPos(m_wText, ms_lastpos, offset));
	}
}

void CIME::DelCurPos()
{
	if (ms_curpos < ms_lastpos)
	{
		int eraseCount = FindColorTagEndPosition(m_wText + ms_curpos, ms_lastpos - ms_curpos) + 1;
		wcscpy(m_wText + ms_curpos, m_wText + ms_curpos + eraseCount);
		ms_lastpos -= eraseCount;
		ms_curpos = min(ms_lastpos, ms_curpos);
	}
}

void CIME::PasteString(const char * str)
{
	const char * begin = str;
	const char * end = str + strlen(str);
	wchar_t m_wText[IMESTR_MAXLEN];
	int wstrLen = MultiByteToWideChar(ms_uInputCodePage, 0, begin, end - begin, m_wText, IMESTR_MAXLEN);
	InsertString(m_wText, wstrLen);
	if(ms_pEvent)
		ms_pEvent->OnUpdate();
}

/*---------------------------------------------------------------------------*/ /* Private */
void CIME::InsertString(wchar_t* wString, int iSize)
{
	if (IsMax(wString, iSize))
		return;

	if (ms_curpos < ms_lastpos)
		memmove(m_wText+ms_curpos+iSize, m_wText+ms_curpos, sizeof(wchar_t)*(ms_lastpos-ms_curpos));

	memcpy(m_wText+ms_curpos, wString, sizeof(wchar_t)*iSize);

	ms_curpos += iSize;
	ms_lastpos += iSize;
}

void CIME::OnChar(wchar_t c)
{
	if (m_bOnlyNumberMode)
		if (!iswdigit(c))
			return;

	if ((c >= 0x00 && c<=0x1f) || (c == 0x7f)) // @fixme011
		return;

	if (!__IsWritable(c))
		return;

	InsertString(&c, 1);
}

UINT CIME::GetCodePageFromLang(LANGID langid)
{
	unsigned pri_langid = PRIMARYLANGID(langid);
	switch (pri_langid)
	{
		case LANG_JAPANESE:
			//setlocale(LC_ALL, ".932");
			return 932;
		case LANG_KOREAN:
			//setlocale(LC_ALL, ".949");
			return 949;
		case LANG_CHINESE:
			{
				switch (SUBLANGID(langid))
				{
					case SUBLANG_CHINESE_SIMPLIFIED:
					case SUBLANG_CHINESE_SINGAPORE:
						//setlocale(LC_ALL, ".936");
						return 936;
					case SUBLANG_CHINESE_TRADITIONAL:
					case SUBLANG_CHINESE_MACAU:
					case SUBLANG_CHINESE_HONGKONG:
						//setlocale(LC_ALL, ".950");
						return 950;
				}
			}
			//setlocale(LC_ALL, ".936");
			return 936;
		case LANG_ARABIC:
			return 1256;
		case LANG_GREEK:
			//setlocale(LC_ALL, ".1253");
			return 1253;
		case LANG_TURKISH:
			//setlocale(LC_ALL, ".1254");
			return 1254;
		case LANG_HEBREW:
			//setlocale(LC_ALL, ".1255");
			return 1255;
		case LANG_ESTONIAN:
		case LANG_LATVIAN:
		case LANG_LITHUANIAN:
			//setlocale(LC_ALL, ".1257");
			return 1257;
		case LANG_VIETNAMESE:
			return 1258;
		case LANG_THAI:
			//setlocale(LC_ALL, ".874");
			return 874;
		case LANG_CZECH:
		case LANG_HUNGARIAN:
		case LANG_POLISH:
		case LANG_CROATIAN:
		case LANG_MACEDONIAN:
		case LANG_ROMANIAN:
		case LANG_SLOVAK:
		case LANG_SLOVENIAN:
			//setlocale(LC_ALL, ".1250");
			return 1250;
		case LANG_RUSSIAN:
		case LANG_BELARUSIAN:
		case LANG_BULGARIAN:
		case LANG_UKRAINIAN:
			return 1251;
		case LANG_GERMAN:
			//_wsetlocale(LC_ALL, ".1252");
			return 1252;
		default:
			//TraceError("UNKNOWN IME[%d]\n", langid);
			//setlocale(LC_ALL, ".949");
			return 1252;
	}
}

void CIME::CompositionProcess(HIMC hImc)
{
	ms_compLen = ImmGetCompositionStringW(hImc, GCS_COMPSTR, m_wszComposition, sizeof(m_wszComposition))/sizeof(wchar_t);

	//OutputDebugStringW( L"Composition: " );
	//OutputDebugStringW( m_wszComposition );
	//for( int i=0; i < (int) ms_compLen * 2; i++ ) {
	//	LPBYTE pbyData = (LPBYTE) m_wszComposition;
	//	pbyData += i;
	//	WCHAR tszName[32];

	//	swprintf_s( tszName, L"%02X ", (unsigned int) *pbyData );
	//	OutputDebugStringW( tszName );
	//}
	//OutputDebugStringW( L"\n" );

	if (IsMax(m_wszComposition, ms_compLen))
	{
		ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
		ms_compLen = 0;
	}
}

void CIME::CompositionProcessBuilding(HIMC hImc)
{
	int textLen = WideCharToMultiByte(ms_uOutputCodePage, 0, m_wText, ms_lastpos, 0, 0, NULL, NULL);

	if (textLen >= m_max)
	{
		ImmNotifyIME(hImc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
		ms_compLen = 0;
		return;
	}

	ms_compLen = ImmGetCompositionStringW(hImc, GCS_COMPSTR, m_wszComposition, sizeof(m_wszComposition))/sizeof(wchar_t);

	//OutputDebugStringW( L"Composition: " );
	//OutputDebugStringW( m_wszComposition );
	//for( int i=0; i < (int) ms_compLen * 2; i++ ) {
	//	LPBYTE pbyData = (LPBYTE) m_wszComposition;
	//	pbyData += i;
	//	WCHAR tszName[32];

	//	swprintf_s( tszName, L"%02X ", (unsigned int) *pbyData );
	//	OutputDebugStringW( tszName );
	//}
	//OutputDebugStringW( L"\n" );
}

void CIME::ResultProcess(HIMC hImc)
{
	wchar_t temp[IMESTR_MAXLEN];

	int len = ImmGetCompositionStringW(hImc, GCS_RESULTSTR, temp, sizeof(temp))/sizeof(wchar_t);

	if (len <= 0)
		return;

	InsertString(temp, len);
}

void CIME::AttributeProcess(HIMC hImc)
{
	BYTE	attribute[IMESTR_MAXLEN];
	LONG	attributeLen = ImmGetCompositionStringW(hImc, GCS_COMPATTR, &attribute, sizeof(attribute)) / sizeof(BYTE);

	int start,end;
	for(start=0; start<attributeLen; ++start) if(attribute[start]==ATTR_TARGET_CONVERTED || attribute[start]==ATTR_TARGET_NOTCONVERTED) break;
	for(end=start; end<attributeLen; ++end) if(attribute[end]!=attribute[start]) break;

	ms_ulbegin = start;
	ms_ulend = end;
}

void CIME::CandidateProcess(HIMC hImc)
{
	std::vector<BYTE>	abyCandidate;
	DWORD dwCandidateLen = ImmGetCandidateListW(hImc, 0, NULL, 0);
	abyCandidate.resize(dwCandidateLen);
	if(dwCandidateLen > 0) {
		ms_bCandidateList = true;

		CANDIDATELIST* lpCandidateList = (CANDIDATELIST*)(&abyCandidate[0]);
		dwCandidateLen = ImmGetCandidateListW(hImc, 0, lpCandidateList, dwCandidateLen);

		ms_dwCandidateSelection	= lpCandidateList->dwSelection;
		ms_dwCandidateCount		= lpCandidateList->dwCount;

		int iStartOfPage = 0;

		if( GETLANG() == LANG_CHS ) {
			// MSPY (CHS IME) has variable number of candidates in candidate window find where current page starts, and the size of current page
			const int maxCandChar = 18 * (3 - sizeof(TCHAR));
			UINT cChars = 0;
			UINT i;
			for (i = 0; i < ms_dwCandidateCount; i++)
			{
				UINT uLen = lstrlenW((LPWSTR)((DWORD)lpCandidateList + lpCandidateList->dwOffset[i])) + (3 - sizeof(WCHAR));
				if (uLen + cChars > maxCandChar)
				{
					if (i > ms_dwCandidateSelection)
					{
						break;
					}
					iStartOfPage = i;
					cChars = uLen;
				}
				else
				{
					cChars += uLen;
				}
			}
			ms_dwCandidatePageSize	= i - iStartOfPage;
		} else {
			ms_dwCandidatePageSize	= MIN( lpCandidateList->dwPageSize, MAX_CANDLIST );
			iStartOfPage = ms_bUILessMode ? lpCandidateList->dwPageStart : (ms_dwCandidateSelection / (MAX_CANDLIST - 1)) * (MAX_CANDLIST - 1);
		}

		ms_dwCandidateSelection = ( GETLANG() == LANG_CHS && !GetImeId() ) ? (DWORD)-1 : ms_dwCandidateSelection - iStartOfPage;

		//printf( "SEL: %d, START: %d, PAGED: %d\n", ms_dwCandidateSelection, iStartOfPage, ms_dwCandidatePageSize );
	    memset(&ms_wszCandidate, 0, sizeof(ms_wszCandidate));
	    for(UINT i = iStartOfPage, j = 0; (DWORD)i < lpCandidateList->dwCount && j < ms_dwCandidatePageSize; i++, j++) {
			wcscpy( ms_wszCandidate[j], (LPWSTR)( (DWORD)lpCandidateList + lpCandidateList->dwOffset[i] ) );
	    }

		// don't display selection in candidate list in case of Korean and old Chinese IME.
		if ( GETPRIMLANG() == LANG_KOREAN || GETLANG() == LANG_CHT && !GetImeId() )
			ms_dwCandidateSelection = (DWORD) -1;
	}
}

void CIME::ReadingProcess(HIMC hImc)
{
    if (!ms_adwId[0])
	{
		return;
    }

    DWORD dwErr = 0;

    if (_GetReadingString)
	{
        UINT uMaxUiLen;
        BOOL bVertical;
        // Obtain the reading string size
        int wstrLen = _GetReadingString(hImc, 0, NULL, (PINT)&dwErr, &bVertical, &uMaxUiLen);

		if(wstrLen == 0) {
			ms_wstrReading.resize(0);
		} else {
			wchar_t *wstr = (wchar_t*)alloca(sizeof(wchar_t) * wstrLen);
            _GetReadingString(hImc, wstrLen, wstr, (PINT)&dwErr, &bVertical, &uMaxUiLen);
			ms_wstrReading.assign(wstr, wstr+wstrLen);
		}

		ms_bHorizontalReading = (bVertical == 0);

    } else {

        // IMEs that doesn't implement Reading String API
		wchar_t* temp = NULL;
		DWORD tempLen = 0;
	    bool bUnicodeIme = false;
		INPUTCONTEXT *lpIC = _ImmLockIMC(hImc);

		if (lpIC == NULL)
		{
			temp = NULL;
			tempLen = 0;
		}
		else
		{
			LPBYTE p = 0;
			switch(ms_adwId[0])
			{
				case IMEID_CHT_VER42: // New(Phonetic/ChanJie)IME98  : 4.2.x.x // Win98
				case IMEID_CHT_VER43: // New(Phonetic/ChanJie)IME98a : 4.3.x.x // WinMe, Win2k
				case IMEID_CHT_VER44: // New ChanJie IME98b          : 4.4.x.x // WinXP
					p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC(lpIC->hPrivate) + 24);
					if (!p) break;
					tempLen = *(DWORD *)(p + 7 * 4 + 32 * 4);
					dwErr = *(DWORD *)(p + 8 * 4 + 32 * 4);
					temp = (wchar_t *)(p + 56);
					bUnicodeIme = true;
					break;

				case IMEID_CHT_VER50: // 5.0.x.x // WinME
					p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC(lpIC->hPrivate) + 3 * 4);
					if(!p) break;
					p = *(LPBYTE *)((LPBYTE)p + 1*4 + 5*4 + 4*2);
					if(!p) break;
					tempLen = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16);
					dwErr = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 + 1*4);
					temp = (wchar_t *)(p + 1*4 + (16*2+2*4) + 5*4);
					bUnicodeIme = false;
					break;

				case IMEID_CHT_VER51: // 5.1.x.x // IME2002(w/OfficeXP)
				case IMEID_CHT_VER52: // 5.2.x.x // (w/whistler)
				case IMEID_CHS_VER53: // 5.3.x.x // SCIME2k or MSPY3 (w/OfficeXP and Whistler)
					p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC(lpIC->hPrivate) + 4);
					if(!p) break;
					p = *(LPBYTE *)((LPBYTE)p + 1*4 + 5*4);
					if(!p) break;
					tempLen = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 * 2);
					dwErr = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 * 2 + 1*4);
					temp  = (wchar_t *) (p + 1*4 + (16*2+2*4) + 5*4);
					bUnicodeIme = true;
					break;

				// the code tested only with Win 98 SE (MSPY 1.5/ ver 4.1.0.21)
				case IMEID_CHS_VER41:
					{
						int nOffset;
						nOffset = (ms_adwId[1] >= 0x00000002) ? 8 : 7;

						p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC(lpIC->hPrivate) + nOffset * 4);
						if(!p) break;
						tempLen = *(DWORD *)(p + 7*4 + 16*2*4);
						dwErr = *(DWORD *)(p + 8*4 + 16*2*4);
						dwErr = min(dwErr, tempLen);
						temp = (wchar_t *)(p + 6*4 + 16*2*1);
						bUnicodeIme = true;
					}
					break;

				case IMEID_CHS_VER42: // 4.2.x.x // SCIME98 or MSPY2 (w/Office2k, Win2k, WinME, etc)
					{
						OSVERSIONINFOA osi;
						osi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
						GetVersionExA(&osi);

						int nTcharSize = (osi.dwPlatformId == VER_PLATFORM_WIN32_NT) ? sizeof(wchar_t) : sizeof(char);
						p = *(LPBYTE *)((LPBYTE)_ImmLockIMCC(lpIC->hPrivate) + 1*4 + 1*4 + 6*4);
						if(!p) break;
						tempLen = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 * nTcharSize);
						dwErr = *(DWORD *)(p + 1*4 + (16*2+2*4) + 5*4 + 16 * nTcharSize + 1*4);
						temp  = (wchar_t *) (p + 1*4 + (16*2+2*4) + 5*4);
						bUnicodeIme = (osi.dwPlatformId == VER_PLATFORM_WIN32_NT) ? true : false;
					}
					break;

				default:
					temp = NULL;
					tempLen = 0;
					break;
			}
		}

		if(tempLen == 0) {
			ms_wstrReading.resize(0);
		} else {
			if(bUnicodeIme) {
				ms_wstrReading.assign(temp, temp+tempLen);
			} else {
				int wstrLen = MultiByteToWideChar(ms_uInputCodePage, 0, (char*)temp, tempLen, NULL, 0);
				wchar_t* wstr = (wchar_t*)alloca(sizeof(wchar_t)*wstrLen);
				MultiByteToWideChar(ms_uInputCodePage, 0, (char*)temp, tempLen, wstr, wstrLen);
				ms_wstrReading.assign(wstr, wstr+wstrLen);
			}
		}

		_ImmUnlockIMCC(lpIC->hPrivate);
		_ImmUnlockIMC(hImc);

		ms_bHorizontalReading = GetReadingWindowOrientation();
    }
	if (ms_wstrReading.size()) {
		ms_bReadingInformation = true;
		if(ms_pEvent)
			ms_pEvent->OnOpenReadingWnd();
	} else {
		CloseReadingInformation();
	}
}

bool CIME::IsMax(const wchar_t* wInput, int len)
{
	if (ms_lastpos + len > IMESTR_MAXLEN)
		return true;

	int textLen = WideCharToMultiByte(ms_uOutputCodePage, 0, m_wText, ms_lastpos, 0, 0, NULL, NULL);
	int inputLen = WideCharToMultiByte(ms_uOutputCodePage, 0, wInput, len, 0, 0, NULL, NULL);
	//return textLen + inputLen > m_max;

	if (textLen + inputLen > m_max)
		return true;
	else if (m_userMax != 0 && m_max != m_userMax)
	{
		std::wstring str = GetTextTagOutputString(m_wText, ms_lastpos);
		std::wstring input = GetTextTagOutputString(wInput, len);
		int textLen = WideCharToMultiByte(ms_uOutputCodePage, 0, str.c_str(), str.length(), 0, 0, NULL, NULL);
		int inputLen = WideCharToMultiByte(ms_uOutputCodePage, 0, input.c_str(), input.length(), 0, 0, NULL, NULL);
		return textLen + inputLen > m_userMax;
	}
	return false;
}

DWORD CIME::GetImeId( UINT uIndex )
{
	static HKL hklPrev = 0;
    char szTmp[1024];

	if (uIndex >= COUNTOF(ms_adwId))
		return 0;
	HKL hkl = ms_hklCurrent;
	if(hklPrev == hkl)
		return ms_adwId[uIndex];
	hklPrev = hkl;

	DWORD dwLang = ((DWORD)hkl & 0xffff);

	if ( ms_bUILessMode && GETLANG() == LANG_CHT ) {
		// In case of Vista, artifitial value is returned so that it's not considered as older IME.
		ms_adwId[0] = IMEID_CHT_VER_VISTA;
		ms_adwId[1] = 0;
		return ms_adwId[0];
	}

	if (!((ms_hklCurrent == _CHT_HKL_NEW_PHONETIC) || (ms_hklCurrent == _CHT_HKL_NEW_CHANG_JIE) || (ms_hklCurrent == _CHT_HKL_NEW_QUICK) || (ms_hklCurrent == _CHT_HKL_HK_CANTONESE) || (ms_hklCurrent == _CHS_HKL))) {
		ms_adwId[0] = ms_adwId[1] = 0;
        return 0;
	}

	if (!ImmGetIMEFileNameA(ms_hklCurrent, szTmp, (sizeof(szTmp) / sizeof(szTmp[0])) - 1)) {
		ms_adwId[0] = ms_adwId[1] = 0;
        return 0;
	}

    if (!_GetReadingString)
	{
        if ((CompareStringA(LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1, CHT_IMEFILENAME1, -1) != CSTR_EQUAL) &&
            (CompareStringA(LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1, CHT_IMEFILENAME2, -1) != CSTR_EQUAL) &&
            (CompareStringA(LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1, CHT_IMEFILENAME3, -1) != CSTR_EQUAL) &&
            (CompareStringA(LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1, CHS_IMEFILENAME1, -1) != CSTR_EQUAL) &&
            (CompareStringA(LCID_INVARIANT, NORM_IGNORECASE, szTmp, -1, CHS_IMEFILENAME2, -1) != CSTR_EQUAL))
		{
			ms_adwId[0] = ms_adwId[1] = 0;
	        return 0;
        }
    }

    DWORD   dwVerHandle;
    DWORD   dwVerSize = GetFileVersionInfoSize(szTmp, &dwVerHandle);
	LANGID	langId = LOWORD(ms_hklCurrent);

    if (dwVerSize)
	{
        LPVOID lpVerBuffer = alloca(dwVerSize);

        if (GetFileVersionInfo(szTmp, dwVerHandle, dwVerSize, lpVerBuffer))
		{
			LPVOID  lpVerData;
			UINT    cbVerData;

            if(VerQueryValue(lpVerBuffer, "\\", &lpVerData, &cbVerData))
			{
                DWORD dwVer = ((VS_FIXEDFILEINFO*) lpVerData)->dwFileVersionMS;
                dwVer = (dwVer & 0x00ff0000) << 8 | (dwVer & 0x000000ff) << 16;

                if (_GetReadingString
                    ||
                    (langId == LANG_CHT &&
                        (dwVer == MAKEIMEVERSION(4, 2) ||
                        dwVer == MAKEIMEVERSION(4, 3) ||
                        dwVer == MAKEIMEVERSION(4, 4) ||
                        dwVer == MAKEIMEVERSION(5, 0) ||
                        dwVer == MAKEIMEVERSION(5, 1) ||
                        dwVer == MAKEIMEVERSION(5, 2) ||
                        dwVer == MAKEIMEVERSION(6, 0)))
                    ||
                    (langId == LANG_CHS &&
                        (dwVer == MAKEIMEVERSION(4, 1) ||
                        dwVer == MAKEIMEVERSION(4, 2) ||
                        dwVer == MAKEIMEVERSION(5, 3))))
				{
                    ms_adwId[0] = dwVer | langId;
                    ms_adwId[1] = ((VS_FIXEDFILEINFO*)lpVerData)->dwFileVersionLS;
					return ms_adwId[uIndex];
                }
            }
        }
    }
	ms_adwId[0] = ms_adwId[1] = 0;
	return ms_adwId[0];
}

bool CIME::GetReadingWindowOrientation()
{
    bool bHorizontalReading = (ms_hklCurrent == _CHS_HKL) || (ms_hklCurrent == _CHT_HKL_NEW_CHANG_JIE) || (ms_adwId[0] == 0);
    if(!bHorizontalReading && (GETLANG() == LANG_CHT))
    {
        char szRegPath[MAX_PATH];
        HKEY hKey;
        DWORD dwVer = ms_adwId[0] & 0xFFFF0000;
        strcpy(szRegPath, "software\\microsoft\\windows\\currentversion\\");
        strcat(szRegPath, (dwVer >= MAKEIMEVERSION(5, 1)) ? "MSTCIPH" : "TINTLGNT");
        LONG lRc = RegOpenKeyExA(HKEY_CURRENT_USER, szRegPath, 0, KEY_READ, &hKey);
        if (lRc == ERROR_SUCCESS)
        {
            DWORD dwSize = sizeof(DWORD), dwMapping, dwType;
            lRc = RegQueryValueExA(hKey, "Keyboard Mapping", NULL, &dwType, (PBYTE)&dwMapping, &dwSize);
            if (lRc == ERROR_SUCCESS)
            {
                if ((dwVer <= MAKEIMEVERSION(5, 0) &&
                       ((BYTE)dwMapping == 0x22 || (BYTE)dwMapping == 0x23))
                     ||
                     ((dwVer == MAKEIMEVERSION(5, 1) || dwVer == MAKEIMEVERSION(5, 2)) &&
                       (BYTE)dwMapping >= 0x22 && (BYTE)dwMapping <= 0x24)
                  )
                {
                    bHorizontalReading = true;
                }
            }
            RegCloseKey(hKey);
        }
    }

	return bHorizontalReading;
}

void CIME::SetupImeApi()
{
    char szImeFile[MAX_PATH + 1];

	_GetReadingString = NULL;
    _ShowReadingWindow = NULL;
	ms_bUseIMMCandidate = false;

	if(ImmGetIMEFileNameA(ms_hklCurrent, szImeFile, COUNTOF(szImeFile) - 1) == 0)
		return;
	if(stricmp(szImeFile, CHS_IMEFILENAME_QQPINYIN) == 0 || stricmp(szImeFile, CHS_IMEFILENAME_SOGOUPY) == 0 || stricmp(szImeFile, CHS_IMEFILENAME_GOOGLEPINYIN2) == 0)
		ms_bUseIMMCandidate = true;
	if (ms_bUILessMode)
		return;
	SAFE_FREE_LIBRARY(ms_hCurrentImeDll);
	ms_hCurrentImeDll = LoadLibraryA(szImeFile);

	if (ms_hCurrentImeDll) {
		_GetReadingString = (UINT (WINAPI*)(HIMC, UINT, LPWSTR, PINT, BOOL*, PUINT)) (GetProcAddress(ms_hCurrentImeDll, "GetReadingString"));
		_ShowReadingWindow =(BOOL (WINAPI*)(HIMC, BOOL)) (GetProcAddress(ms_hCurrentImeDll, "ShowReadingWindow"));

		if(_ShowReadingWindow) {
			HIMC hImc = ImmGetContext(ms_hWnd);
			if(hImc) {
				_ShowReadingWindow(hImc, false);
				ImmReleaseContext(ms_hWnd, hImc);
			}
		}
	}
}

static DWORD _strtoul( LPCSTR psz, LPTSTR*, int )
{
	if ( !psz )
		return 0;

	ULONG ulRet = 0;
	if ( psz[0] == '0' && ( psz[1] == 'x' || psz[1] == 'X' ) )
	{
		psz += 2;
		ULONG ul = 0;
		while ( *psz )
		{
			if ( '0' <= *psz && *psz <= '9' )
				ul = *psz - '0';
			else if ( 'A' <= *psz && *psz <= 'F' )
				ul = *psz - 'A' + 10;
			else if ( 'a' <= *psz && *psz <= 'f' )
				ul = *psz - 'a' + 10;
			else
				break;
			ulRet = ulRet * 16 + ul;
			psz++;
		}
	}
	else {
		while ( *psz && ( '0' <= *psz && *psz <= '9' ) )
		{
			ulRet = ulRet * 10 + ( *psz - '0' );
			psz++;
		}
	}
	return ulRet;
}

void CIME::CheckInputLocale()
{
	static UINT s_uPrevCodePage = 0xFFFF;
	static HKL	s_hklPrev = NULL;

	ms_hklCurrent = GetKeyboardLayout( 0 );
	if ( s_hklPrev == ms_hklCurrent )
		return;
	s_hklPrev = ms_hklCurrent;

	char szCodePage[8];
	int iRc = GetLocaleInfoA( MAKELCID( GETLANG(), SORT_DEFAULT ), LOCALE_IDEFAULTANSICODEPAGE, szCodePage, COUNTOF( szCodePage ) ); iRc;
	ms_uInputCodePage = _strtoul( szCodePage, NULL, 0 );
	if ( s_uPrevCodePage == ms_uInputCodePage )
		return;
	s_uPrevCodePage = ms_uInputCodePage;

	GetKeyboardLayoutName(ms_szKeyboardLayout);

    switch (GETPRIMLANG())
	{
		case LANG_KOREAN:
			ms_bVerticalCandidate = false;
			ms_wszCurrentIndicator = s_aszIndicator[INDICATOR_KOREAN];
			break;

		case LANG_JAPANESE:
			ms_bVerticalCandidate = true;
			ms_wszCurrentIndicator = s_aszIndicator[INDICATOR_JAPANESE];
			break;

		case LANG_CHINESE:
			ms_bVerticalCandidate = true;

			switch(GETSUBLANG())
			{
				case SUBLANG_CHINESE_SIMPLIFIED:
				case SUBLANG_CHINESE_SINGAPORE:
					//ms_bVerticalCandidate = (GetImeId() == 0);
					ms_bVerticalCandidate = false;
					ms_wszCurrentIndicator = s_aszIndicator[INDICATOR_CHS];
					break;

				case SUBLANG_CHINESE_TRADITIONAL:
				case SUBLANG_CHINESE_HONGKONG:
				case SUBLANG_CHINESE_MACAU:
					ms_wszCurrentIndicator = s_aszIndicator[INDICATOR_CHT];
					break;

				default:	// unsupported sub-language
					ms_wszCurrentIndicator = s_aszIndicator[INDICATOR_NON_IME];
					break;
			}
			break;

		default:
			ms_wszCurrentIndicator = s_aszIndicator[INDICATOR_NON_IME];
			break;
    }

    if (ms_wszCurrentIndicator == s_aszIndicator[INDICATOR_NON_IME])
    {
        char szLang[10];
        GetLocaleInfoA(MAKELCID(GETLANG(), SORT_DEFAULT), LOCALE_SABBREVLANGNAME, szLang, sizeof(szLang));
        ms_wszCurrentIndicator[0] = szLang[0];
        ms_wszCurrentIndicator[1] = towlower(szLang[1]);
    }

	if(ms_uOutputCodePage != 1256) {
		ms_uOutputCodePage = ms_uInputCodePage;
		Clear();
	}

	//for ( int i = 0; i < 256; i++ )
	//{
	//	LeadByteTable[i] = (BYTE)IsDBCSLeadByteEx( g_uCodePage, (BYTE)i );
	//}
}

void CIME::CheckToggleState()
{
	CheckInputLocale();

	// In Vista, we have to use TSF since few IMM functions don't work as expected.
	// WARNING: Because of timing, g_dwState and g_bChineseIME may not be updated
	// immediately after the change on IME states by user.
	if ( ms_bUILessMode )
		return;

	/* Check Toggle State */
	bool bIme = ImmIsIME( ms_hklCurrent ) != 0
		&& ( ( 0xF0000000 & (DWORD)ms_hklCurrent ) == 0xE0000000 ); // Hack to detect IME correctly. When IME is running as TIP, ImmIsIME() returns true for CHT US keyboard.
	ms_bChineseIME = ( GETPRIMLANG() == LANG_CHINESE ) && bIme;

	HIMC himc;
	if (NULL != (himc = ImmGetContext(ms_hWnd))) {
		if (ms_bChineseIME) {
			DWORD dwConvMode, dwSentMode;
			ImmGetConversionStatus(himc, &dwConvMode, &dwSentMode);
			ms_dwImeState = ( dwConvMode & IME_CMODE_NATIVE ) ? IMEUI_STATE_ON : IMEUI_STATE_ENGLISH;
		}
		else
		{
			ms_dwImeState = ( bIme && ImmGetOpenStatus( himc ) != 0 ) ? IMEUI_STATE_ON : IMEUI_STATE_OFF;
		}
		ImmReleaseContext(ms_hWnd, himc);
	}
	else
		ms_dwImeState = IMEUI_STATE_OFF;
}

///////////////////////////////////////////////////////////////////////////////
//
//	CTsfUiLessMode methods
//
///////////////////////////////////////////////////////////////////////////////

//
//	SetupSinks()
//	Set up sinks. A sink is used to receive a Text Service Framework event.
//  CUIElementSink implements multiple sink interfaces to receive few different TSF events.
//
LRESULT CIME::WMInputLanguage(HWND hWnd, UINT /*uiMsg*/, WPARAM /*wParam*/, LPARAM lParam)
{
	ChangeInputLanguage();
	return 0;
}

LRESULT CIME::WMStartComposition(HWND /*hWnd*/, UINT /*uiMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return 1L;
}

LRESULT CIME::WMComposition(HWND hWnd, UINT /*uiMsg*/, WPARAM /*wParam*/, LPARAM lParam)
{
	LRESULT		result = 0;

	if(ms_bCaptureInput == false)
		return 0;

	HIMC hImc = ImmGetContext(hWnd);

	if(hImc == NULL)
		return 0;

	if(lParam&GCS_RESULTSTR)
		ResultProcess(hImc);
	if(lParam&GCS_COMPATTR)
		AttributeProcess(hImc);
	if(lParam&GCS_COMPSTR)
	{
		if (ms_uOutputCodePage == 950)
		{
			if (lParam&GCS_COMPATTR)
				CompositionProcessBuilding(hImc);
			else
				CompositionProcess(hImc);
		}
		else
		{
			CompositionProcess(hImc);
		}
	}

	ImmReleaseContext(hWnd, hImc);

	if(ms_pEvent)
		ms_pEvent->OnUpdate();

	return (result);
}

LRESULT CIME::WMEndComposition(HWND /*hWnd*/, UINT /*uiMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	ms_compLen = 0;
	ms_ulbegin = 0;
	ms_ulend = 0;

	if(ms_pEvent)
		ms_pEvent->OnUpdate();

	return 0L;
}

LRESULT CIME::WMNotify(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT		result = 0;

	if(ms_bCaptureInput == false)
		return 0;
	switch (wParam) {

	case IMN_OPENCANDIDATE:
	case IMN_CHANGECANDIDATE: {
		if (ms_bUILessMode && !ms_bUseIMMCandidate)
			break;
		HIMC hImc = ImmGetContext(hWnd);
		if (hImc == NULL)
			break;
		CandidateProcess(hImc);
		if (!m_bUseDefaultIME) {
			if(ms_pEvent)
				ms_pEvent->OnOpenCandidateList();
		} else
			result = ::DefWindowProc(hWnd, uiMsg, wParam, lParam);
		ImmReleaseContext(hWnd, hImc);
		break;
		}
	case IMN_CLOSECANDIDATE:
		if (ms_bUILessMode && !ms_bUseIMMCandidate)
			break;
		if (!m_bUseDefaultIME)
			CloseCandidateList();
		else
			result = DefWindowProc(hWnd, uiMsg, wParam, lParam);
		break;

	case IMN_SETCONVERSIONMODE:
	case IMN_SETOPENSTATUS:
		if (ms_bUILessMode)
			break;
		CheckToggleState();
		break;

	case IMN_PRIVATE: {
		if (ms_bUILessMode)
			break;
		HIMC hImc = ImmGetContext(hWnd);
		if (hImc == NULL)
			break;
        ReadingProcess(hImc);

		// Trap some messages to hide reading window
        switch(ms_adwId[0])
        {
            case IMEID_CHT_VER42:
            case IMEID_CHT_VER43:
            case IMEID_CHT_VER44:
            case IMEID_CHS_VER41:
            case IMEID_CHS_VER42:
				if ((lParam == 1)||(lParam == 2))
					return true;
                break;

            case IMEID_CHT_VER50:
            case IMEID_CHT_VER51:
            case IMEID_CHT_VER52:
            case IMEID_CHT_VER60:
            case IMEID_CHS_VER53:
                if ((lParam == 16)||(lParam == 17)||(lParam == 26)||(lParam == 27)||(lParam == 28))
					return true;
                break;
        }
		ImmReleaseContext(hWnd, hImc);
		break;
		}
	}

	if(ms_pEvent)
		ms_pEvent->OnUpdate();

	return result;
}

LRESULT CIME::WMChar(HWND /*hWnd*/, UINT /*uiMsg*/, WPARAM wParam, LPARAM lParam)
{
	unsigned char c = (unsigned char)(wParam & 0xff);

	switch (c)
	{
	case 8:
		if(ms_bCaptureInput == false)
			return 0;
		if (ms_curpos > 0)
		{
			DecCurPos();
			DelCurPos();
		}
		if(ms_pEvent)
			ms_pEvent->OnUpdate();
		return 0;
		break;

	default:
		if(ms_pEvent) {
			if (ms_pEvent->OnWM_CHAR(wParam, lParam))
				break;
		}
		if(ms_bCaptureInput == false)
			return 0;
		wchar_t w[10];
		MultiByteToWideChar(ms_uInputCodePage, 0, (char*)&c, 1, w, 1);

		OnChar(w[0]);
		if (w[0] == L'|')
			OnChar(w[0]);
		if(ms_pEvent)
			ms_pEvent->OnUpdate();
		break;
	}

	return 0;
}
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
