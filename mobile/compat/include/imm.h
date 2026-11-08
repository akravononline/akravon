// Cynex: imm - mobilde ime yok, yazi WM_CHAR ile geliyor
#pragma once
#include <windows.h>

typedef struct tagCANDIDATELIST
{
	DWORD dwSize;
	DWORD dwStyle;
	DWORD dwCount;
	DWORD dwSelection;
	DWORD dwPageStart;
	DWORD dwPageSize;
	DWORD dwOffset[1];
} CANDIDATELIST, *PCANDIDATELIST, *LPCANDIDATELIST;

typedef struct tagCOMPOSITIONFORM { DWORD dwStyle; POINT ptCurrentPos; RECT rcArea; } COMPOSITIONFORM, *LPCOMPOSITIONFORM;
typedef struct tagCANDIDATEFORM { DWORD dwIndex; DWORD dwStyle; POINT ptCurrentPos; RECT rcArea; } CANDIDATEFORM, *LPCANDIDATEFORM;
typedef DWORD HIMCC;
typedef struct tagINPUTCONTEXT { HWND hWnd; BOOL fOpen; POINT ptStatusWndPos; POINT ptSoftKbdPos; DWORD fdwConversion; DWORD fdwSentence;
	HIMCC hCompStr; HIMCC hCandInfo; HIMCC hGuideLine; HIMCC hPrivate; DWORD dwNumMsgBuf; HIMCC hMsgBuf; DWORD fdwInit; } INPUTCONTEXT, *LPINPUTCONTEXT;

#define CFS_DEFAULT 0x0000
#define CFS_RECT 0x0001
#define CFS_POINT 0x0002
#define CFS_FORCE_POSITION 0x0020
#define CFS_CANDIDATEPOS 0x0040
#define CFS_EXCLUDE 0x0080
#define GCS_COMPREADSTR 0x0001
#define GCS_COMPREADATTR 0x0002
#define GCS_COMPREADCLAUSE 0x0004
#define GCS_COMPSTR 0x0008
#define GCS_COMPATTR 0x0010
#define GCS_COMPCLAUSE 0x0020
#define GCS_CURSORPOS 0x0080
#define GCS_DELTASTART 0x0100
#define GCS_RESULTREADSTR 0x0200
#define GCS_RESULTREADCLAUSE 0x0400
#define GCS_RESULTSTR 0x0800
#define GCS_RESULTCLAUSE 0x1000
#define IMN_CLOSESTATUSWINDOW 0x0001
#define IMN_OPENSTATUSWINDOW 0x0002
#define IMN_CHANGECANDIDATE 0x0003
#define IMN_CLOSECANDIDATE 0x0004
#define IMN_OPENCANDIDATE 0x0005
#define IMN_SETCONVERSIONMODE 0x0006
#define IMN_SETSENTENCEMODE 0x0007
#define IMN_SETOPENSTATUS 0x0008
#define IMN_SETCANDIDATEPOS 0x0009
#define IMN_SETCOMPOSITIONFONT 0x000A
#define IMN_SETCOMPOSITIONWINDOW 0x000B
#define IMN_SETSTATUSWINDOWPOS 0x000C
#define IMN_GUIDELINE 0x000D
#define IMN_PRIVATE 0x000E
#define NI_OPENCANDIDATE 0x0010
#define NI_CLOSECANDIDATE 0x0011
#define NI_SELECTCANDIDATESTR 0x0012
#define NI_CHANGECANDIDATELIST 0x0013
#define NI_COMPOSITIONSTR 0x0015
#define CPS_COMPLETE 0x0001
#define CPS_CONVERT 0x0002
#define CPS_REVERT 0x0003
#define CPS_CANCEL 0x0004
#define SCS_SETSTR (GCS_COMPREADSTR | GCS_COMPSTR)
#define ISC_SHOWUICANDIDATEWINDOW 0x00000001
#define ISC_SHOWUICOMPOSITIONWINDOW 0x80000000
#define ISC_SHOWUIGUIDELINE 0x40000000
#define ISC_SHOWUIALLCANDIDATEWINDOW 0x0000000F
#define IME_CMODE_ALPHANUMERIC 0x0000
#define IME_CMODE_NATIVE 0x0001
#define IME_CMODE_HANGUL IME_CMODE_NATIVE
#define IME_SMODE_NONE 0x0000
#define IME_SMODE_AUTOMATIC 0x0004
#define ATTR_INPUT 0x00
#define ATTR_TARGET_CONVERTED 0x01
#define ATTR_CONVERTED 0x02
#define ATTR_TARGET_NOTCONVERTED 0x03
#define ATTR_INPUT_ERROR 0x04
#define KL_NAMELENGTH 9

#define LANG_NEUTRAL 0x00
#define LANG_ARABIC 0x01
#define LANG_BULGARIAN 0x02
#define LANG_CHINESE 0x04
#define LANG_CZECH 0x05
#define LANG_GERMAN 0x07
#define LANG_GREEK 0x08
#define LANG_ENGLISH 0x09
#define LANG_SPANISH 0x0a
#define LANG_FRENCH 0x0c
#define LANG_HEBREW 0x0d
#define LANG_HUNGARIAN 0x0e
#define LANG_ITALIAN 0x10
#define LANG_JAPANESE 0x11
#define LANG_KOREAN 0x12
#define LANG_POLISH 0x15
#define LANG_PORTUGUESE 0x16
#define LANG_ROMANIAN 0x18
#define LANG_RUSSIAN 0x19
#define LANG_CROATIAN 0x1a
#define LANG_SLOVAK 0x1b
#define LANG_THAI 0x1e
#define LANG_TURKISH 0x1f
#define LANG_UKRAINIAN 0x22
#define LANG_BELARUSIAN 0x23
#define LANG_SLOVENIAN 0x24
#define LANG_ESTONIAN 0x25
#define LANG_LATVIAN 0x26
#define LANG_LITHUANIAN 0x27
#define LANG_VIETNAMESE 0x2a
#define LANG_MACEDONIAN 0x2f
#define SUBLANG_NEUTRAL 0x00
#define SUBLANG_DEFAULT 0x01
#define SUBLANG_ENGLISH_US 0x01
#define SUBLANG_CHINESE_TRADITIONAL 0x01
#define SUBLANG_CHINESE_SIMPLIFIED 0x02
#define SUBLANG_CHINESE_HONGKONG 0x03
#define SUBLANG_CHINESE_SINGAPORE 0x04
#define SUBLANG_CHINESE_MACAU 0x05
#define SORT_DEFAULT 0x0
#define MAKELANGID(p, s) ((((WORD)(s)) << 10) | (WORD)(p))
#define PRIMARYLANGID(lgid) ((WORD)(lgid) & 0x3ff)
#define SUBLANGID(lgid) ((WORD)(lgid) >> 10)
#define MAKELCID(lgid, srtid) ((DWORD)((((DWORD)((WORD)(srtid))) << 16) | ((DWORD)((WORD)(lgid)))))
#define LANGIDFROMLCID(lcid) ((WORD)(lcid))
#define LOCALE_SABBREVLANGNAME 0x00000003
#define LOCALE_IDEFAULTANSICODEPAGE 0x00001004
#define LOCALE_IDEFAULTCODEPAGE 0x0000000B
#define LOCALE_SYSTEM_DEFAULT 0x0800
#define LOCALE_USER_DEFAULT 0x0400

inline HIMC ImmGetContext(HWND) { return NULL; }
inline BOOL ImmReleaseContext(HWND, HIMC) { return TRUE; }
inline HIMC ImmAssociateContext(HWND, HIMC) { return NULL; }
inline LONG ImmGetCompositionStringW(HIMC, DWORD, LPVOID, DWORD) { return 0; }
inline BOOL ImmSetCompositionStringW(HIMC, DWORD, LPVOID, DWORD, LPVOID, DWORD) { return FALSE; }
inline DWORD ImmGetCandidateListW(HIMC, DWORD, LPCANDIDATELIST, DWORD) { return 0; }
inline BOOL ImmNotifyIME(HIMC, DWORD, DWORD, DWORD) { return FALSE; }
inline BOOL ImmGetConversionStatus(HIMC, LPDWORD c, LPDWORD s) { if (c) *c = 0; if (s) *s = 0; return FALSE; }
inline BOOL ImmSetConversionStatus(HIMC, DWORD, DWORD) { return FALSE; }
inline BOOL ImmGetOpenStatus(HIMC) { return FALSE; }
inline BOOL ImmSetOpenStatus(HIMC, BOOL) { return FALSE; }
inline BOOL ImmIsIME(HKL) { return FALSE; }
inline UINT ImmGetIMEFileNameA(HKL, LPSTR s, UINT n) { if (s && n) s[0] = 0; return 0; }
inline HWND ImmGetDefaultIMEWnd(HWND) { return NULL; }
inline BOOL ImmDisableTextFrameService(DWORD) { return TRUE; }
inline BOOL ImmSetCompositionWindow(HIMC, LPCOMPOSITIONFORM) { return FALSE; }
inline BOOL ImmSetCandidateWindow(HIMC, LPCANDIDATEFORM) { return FALSE; }
inline LPINPUTCONTEXT ImmLockIMC(HIMC) { return NULL; }
inline BOOL ImmUnlockIMC(HIMC) { return FALSE; }
inline LPVOID ImmLockIMCC(HIMCC) { return NULL; }
inline BOOL ImmUnlockIMCC(HIMCC) { return FALSE; }
inline LPVOID ImmLock(HIMC) { return NULL; }
inline BOOL ImmUnlock(HIMC) { return FALSE; }

#ifdef __cplusplus
extern "C" {
#endif
BOOL GetKeyboardLayoutNameA(LPSTR pwszKLID);
#define GetKeyboardLayoutName GetKeyboardLayoutNameA
int GetLocaleInfoA(LCID Locale, LCTYPE LCType, LPSTR lpLCData, int cchData);
#define GetLocaleInfo GetLocaleInfoA
#ifdef __cplusplus
}
#endif
