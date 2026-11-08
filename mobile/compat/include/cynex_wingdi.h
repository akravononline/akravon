// Cynex: gdi karsiliklari - yalnizca bellek DC + yazi (freetype, compat/src/wingdi.cpp)
#pragma once

#define LF_FACESIZE 32

#define ANSI_CHARSET 0
#define DEFAULT_CHARSET 1
#define SYMBOL_CHARSET 2
#define SHIFTJIS_CHARSET 128
#define HANGEUL_CHARSET 129
#define HANGUL_CHARSET 129
#define GB2312_CHARSET 134
#define CHINESEBIG5_CHARSET 136
#define OEM_CHARSET 255
#define JOHAB_CHARSET 130
#define HEBREW_CHARSET 177
#define ARABIC_CHARSET 178
#define GREEK_CHARSET 161
#define TURKISH_CHARSET 162
#define VIETNAMESE_CHARSET 163
#define THAI_CHARSET 222
#define EASTEUROPE_CHARSET 238
#define RUSSIAN_CHARSET 204
#define MAC_CHARSET 77
#define BALTIC_CHARSET 186

#define FW_DONTCARE 0
#define FW_THIN 100
#define FW_LIGHT 300
#define FW_NORMAL 400
#define FW_REGULAR 400
#define FW_MEDIUM 500
#define FW_SEMIBOLD 600
#define FW_BOLD 700
#define FW_HEAVY 900

#define OUT_DEFAULT_PRECIS 0
#define OUT_TT_PRECIS 4
#define OUT_TT_ONLY_PRECIS 7
#define CLIP_DEFAULT_PRECIS 0
#define DEFAULT_QUALITY 0
#define DRAFT_QUALITY 1
#define PROOF_QUALITY 2
#define NONANTIALIASED_QUALITY 3
#define ANTIALIASED_QUALITY 4
#define CLEARTYPE_QUALITY 5
#define DEFAULT_PITCH 0
#define FIXED_PITCH 1
#define VARIABLE_PITCH 2
#define FF_DONTCARE (0 << 4)
#define FF_SWISS (2 << 4)

#define TRANSPARENT 1
#define OPAQUE 2
#define BI_RGB 0L
#define DIB_RGB_COLORS 0
#define SRCCOPY ((DWORD)0x00CC0020)
#define PATCOPY ((DWORD)0x00F00021)
#define BLACKNESS ((DWORD)0x00000042)
#define WHITENESS ((DWORD)0x00FF0062)
#define WHITE_BRUSH 0
#define BLACK_BRUSH 4
#define NULL_BRUSH 5
#define LOGPIXELSX 88
#define LOGPIXELSY 90
#define BITSPIXEL 12
#define HORZRES 8
#define VERTRES 10
#define VREFRESH 116
#define TA_LEFT 0
#define TA_TOP 0
#define TA_BASELINE 24
#define MM_TEXT 1
#define GDI_ERROR (0xFFFFFFFFL)
#define CLR_INVALID 0xFFFFFFFF

typedef struct tagLOGFONTA
{
	LONG lfHeight;
	LONG lfWidth;
	LONG lfEscapement;
	LONG lfOrientation;
	LONG lfWeight;
	BYTE lfItalic;
	BYTE lfUnderline;
	BYTE lfStrikeOut;
	BYTE lfCharSet;
	BYTE lfOutPrecision;
	BYTE lfClipPrecision;
	BYTE lfQuality;
	BYTE lfPitchAndFamily;
	CHAR lfFaceName[LF_FACESIZE];
} LOGFONTA, LOGFONT, *PLOGFONT, *LPLOGFONT, *LPLOGFONTA;

typedef struct tagTEXTMETRICA
{
	LONG tmHeight;
	LONG tmAscent;
	LONG tmDescent;
	LONG tmInternalLeading;
	LONG tmExternalLeading;
	LONG tmAveCharWidth;
	LONG tmMaxCharWidth;
	LONG tmWeight;
	LONG tmOverhang;
	LONG tmDigitizedAspectX;
	LONG tmDigitizedAspectY;
	BYTE tmFirstChar;
	BYTE tmLastChar;
	BYTE tmDefaultChar;
	BYTE tmBreakChar;
	BYTE tmItalic;
	BYTE tmUnderlined;
	BYTE tmStruckOut;
	BYTE tmPitchAndFamily;
	BYTE tmCharSet;
} TEXTMETRICA, TEXTMETRIC, TEXTMETRICW, *LPTEXTMETRIC, *LPTEXTMETRICW;

typedef struct _POINTFLOAT { FLOAT x; FLOAT y; } POINTFLOAT, *PPOINTFLOAT;
typedef struct _GLYPHMETRICSFLOAT { FLOAT gmfBlackBoxX; FLOAT gmfBlackBoxY; POINTFLOAT gmfptGlyphOrigin; FLOAT gmfCellIncX; FLOAT gmfCellIncY; } GLYPHMETRICSFLOAT, *PGLYPHMETRICSFLOAT, *LPGLYPHMETRICSFLOAT;

typedef struct _ABC { int abcA; UINT abcB; int abcC; } ABC, *LPABC;
typedef struct _ABCFLOAT { FLOAT abcfA; FLOAT abcfB; FLOAT abcfC; } ABCFLOAT, *LPABCFLOAT;

typedef struct tagRGBQUAD { BYTE rgbBlue; BYTE rgbGreen; BYTE rgbRed; BYTE rgbReserved; } RGBQUAD;

#pragma pack(push, 2)
typedef struct tagBITMAPFILEHEADER
{
	WORD bfType;
	DWORD bfSize;
	WORD bfReserved1;
	WORD bfReserved2;
	DWORD bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER;
#pragma pack(pop)

typedef struct tagBITMAPINFOHEADER
{
	DWORD biSize;
	LONG biWidth;
	LONG biHeight;
	WORD biPlanes;
	WORD biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG biXPelsPerMeter;
	LONG biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagBITMAPINFO { BITMAPINFOHEADER bmiHeader; RGBQUAD bmiColors[1]; } BITMAPINFO, *LPBITMAPINFO, *PBITMAPINFO;

typedef struct tagBITMAP
{
	LONG bmType;
	LONG bmWidth;
	LONG bmHeight;
	LONG bmWidthBytes;
	WORD bmPlanes;
	WORD bmBitsPixel;
	LPVOID bmBits;
} BITMAP, *LPBITMAP;

#ifdef __cplusplus
extern "C" {
#endif

HDC GetDC(HWND hWnd);
int ReleaseDC(HWND hWnd, HDC hDC);
HDC CreateCompatibleDC(HDC hdc);
BOOL DeleteDC(HDC hdc);
HBITMAP CreateDIBSection(HDC hdc, const BITMAPINFO* pbmi, UINT usage, void** ppvBits, HANDLE hSection, DWORD offset);
HBITMAP CreateCompatibleBitmap(HDC hdc, int cx, int cy);
HGDIOBJ SelectObject(HDC hdc, HGDIOBJ h);
BOOL DeleteObject(HGDIOBJ ho);
HGDIOBJ GetStockObject(int i);
int GetObjectA(HANDLE h, int c, LPVOID pv);
#define GetObject GetObjectA
HFONT CreateFontIndirectA(const LOGFONT* lplf);
#define CreateFontIndirect CreateFontIndirectA
HFONT CreateFontA(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic, DWORD bUnderline,
	DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCSTR pszFaceName);
#define CreateFont CreateFontA
COLORREF SetTextColor(HDC hdc, COLORREF color);
COLORREF SetBkColor(HDC hdc, COLORREF color);
int SetBkMode(HDC hdc, int mode);
UINT SetTextAlign(HDC hdc, UINT align);
int SetMapMode(HDC hdc, int iMode);
BOOL TextOutA(HDC hdc, int x, int y, LPCSTR lpString, int c);
BOOL TextOutW(HDC hdc, int x, int y, LPCWSTR lpString, int c);
#define TextOut TextOutA
BOOL GetTextExtentPoint32A(HDC hdc, LPCSTR lpString, int c, LPSIZE psizl);
BOOL GetTextExtentPoint32W(HDC hdc, LPCWSTR lpString, int c, LPSIZE psizl);
#define GetTextExtentPoint32 GetTextExtentPoint32A
#define GetTextExtentPoint GetTextExtentPoint32A
BOOL GetCharABCWidthsFloatW(HDC hdc, UINT iFirst, UINT iLast, LPABCFLOAT lpABC);
BOOL GetCharABCWidthsW(HDC hdc, UINT wFirst, UINT wLast, LPABC lpABC);
BOOL GetTextMetricsA(HDC hdc, LPTEXTMETRIC lptm);
BOOL GetTextMetricsW(HDC hdc, LPTEXTMETRICW lptm);
#define GetTextMetrics GetTextMetricsA
int GetDeviceCaps(HDC hdc, int index);
int FillRect(HDC hDC, const RECT* lprc, HBRUSH hbr);
HBRUSH CreateSolidBrush(COLORREF color);
BOOL PatBlt(HDC hdc, int x, int y, int w, int h, DWORD rop);
BOOL BitBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop);
BOOL StretchBlt(HDC hdcDest, int xDest, int yDest, int wDest, int hDest, HDC hdcSrc, int xSrc, int ySrc, int wSrc, int hSrc, DWORD rop);
BOOL GdiFlush(void);
int SetDIBitsToDevice(HDC hdc, int xDest, int yDest, DWORD w, DWORD h, int xSrc, int ySrc, UINT StartScan, UINT cLines, const VOID* lpvBits, const BITMAPINFO* lpbmi, UINT ColorUse);
int StretchDIBits(HDC hdc, int xDest, int yDest, int DestWidth, int DestHeight, int xSrc, int ySrc, int SrcWidth, int SrcHeight, const VOID* lpBits, const BITMAPINFO* lpbmi, UINT iUsage, DWORD rop);
typedef struct tagENUMLOGFONTEXA { LOGFONTA elfLogFont; BYTE elfFullName[64]; BYTE elfStyle[32]; BYTE elfScript[32]; } ENUMLOGFONTEXA, ENUMLOGFONTEX, *LPENUMLOGFONTEX;
typedef struct tagNEWTEXTMETRICEXA { TEXTMETRICA ntmTm; DWORD ntmFontSig[6]; } NEWTEXTMETRICEXA, NEWTEXTMETRICEX;
typedef int (*FONTENUMPROCA)(const LOGFONTA*, const TEXTMETRICA*, DWORD, LPARAM);
#define FONTENUMPROC FONTENUMPROCA
int EnumFontFamiliesExA(HDC hdc, LPLOGFONTA lpLogfont, FONTENUMPROCA lpProc, LPARAM lParam, DWORD dwFlags);
#define EnumFontFamiliesEx EnumFontFamiliesExA

// Cynex: yazi tipi klasoru (oyun klasoru/fonts) - android_main ayarlar
void CynexGdi_SetFontDirectory(const char* path);

#ifdef __cplusplus
}
#endif
