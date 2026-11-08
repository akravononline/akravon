// Cynex: gdi karsiliklari - bellek DC, DIB ve freetype ile yazi
#include <windows.h>
#include <android/log.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#include FT_SYNTHESIS_H
#include <dirent.h>
#include <sys/stat.h>
#include <math.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace
{
	enum EGdiType { GDI_DC = 1, GDI_BITMAP, GDI_FONT, GDI_BRUSH };

	struct SGdiObject
	{
		int type;
		explicit SGdiObject(int t) : type(t) {}
		virtual ~SGdiObject() {}
	};

	struct SBitmap : SGdiObject
	{
		SBitmap() : SGdiObject(GDI_BITMAP) {}
		~SBitmap() override { if (owned) free(bits); }
		uint32_t* bits = nullptr;
		int width = 0;
		int height = 0;
		bool topDown = true;
		bool owned = true;

		uint32_t* Row(int y) { return bits + (size_t)(topDown ? y : height - 1 - y) * width; }
	};

	struct SGlyph
	{
		int left = 0;		// Cynex: kalem -> bitmap sol
		int top = 0;		// Cynex: taban cizgisi -> bitmap ust
		int width = 0;
		int rows = 0;
		int advance = 0;
		std::vector<unsigned char> mask;	// 0/1
	};

	struct SFont : SGdiObject
	{
		SFont() : SGdiObject(GDI_FONT) {}
		~SFont() override { if (face) FT_Done_Face(face); }
		LOGFONTA log = {};
		FT_Face face = nullptr;
		bool emboldenSynth = false;
		bool italicSynth = false;
		int ascent = 0;
		int descent = 0;
		int internalLeading = 0;
		int aveWidth = 0;
		int maxWidth = 0;
		std::unordered_map<uint32_t, SGlyph> glyphs;
	};

	struct SBrush : SGdiObject
	{
		SBrush() : SGdiObject(GDI_BRUSH) {}
		COLORREF color = 0;
		bool stock = false;
	};

	struct SDC : SGdiObject
	{
		SDC() : SGdiObject(GDI_DC) {}
		SBitmap* bitmap = nullptr;
		SFont* font = nullptr;
		SBrush* brush = nullptr;
		COLORREF textColor = 0;
		COLORREF bkColor = RGB(255, 255, 255);
		int bkMode = OPAQUE;
		UINT align = TA_LEFT;
		bool screen = false;
	};

	std::recursive_mutex g_lock;
	FT_Library g_ft = nullptr;
	std::string g_fontDir;
	SBitmap g_stockBitmap;	// Cynex: 1x1 bos, yeni DC'lerin varsayilan bitmap'i
	SFont* g_stockFont = nullptr;
	SBrush g_stockBrushes[6];
	SDC g_screenDC;

	template <class T> T* As(HGDIOBJ h, int type)
	{
		SGdiObject* o = (SGdiObject*)h;
		return (o && o->type == type) ? (T*)o : nullptr;
	}

	SDC* DC(HDC h)
	{
		return As<SDC>((HGDIOBJ)h, GDI_DC);
	}

	bool InitFreeType()
	{
		if (g_ft)
			return true;
		if (FT_Init_FreeType(&g_ft))
		{
			__android_log_print(ANDROID_LOG_ERROR, "CynexMobil", "freetype acilamadi");
			g_ft = nullptr;
			return false;
		}
		return true;
	}

	std::string Lower(const char* s)
	{
		std::string r = s ? s : "";
		for (char& c : r)
			c = (char)tolower((unsigned char)c);
		return r;
	}

	bool Exists(const std::string& path)
	{
		struct stat st;
		return !path.empty() && stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
	}

	// Cynex: fonts klasorunde buyuk/kucuk harf duyarsiz ara
	std::string FindInFontDir(const std::string& fileLower)
	{
		if (g_fontDir.empty())
			return std::string();
		DIR* d = opendir(g_fontDir.c_str());
		if (!d)
			return std::string();
		std::string found;
		while (dirent* e = readdir(d))
		{
			if (Lower(e->d_name) == fileLower)
			{
				found = g_fontDir + "/" + e->d_name;
				break;
			}
		}
		closedir(d);
		return found;
	}

	// Cynex: once oyunun fonts klasoru (tahoma.ttf vb.), yoksa android sistem yazisi
	std::string ResolveFontFile(const char* faceName, bool bold, bool& boldFound)
	{
		std::string face = Lower(faceName);
		std::string compact;
		for (char c : face)
			if (c != ' ')
				compact += c;

		// Cynex: arial yerine serbest lisansli metrik uyumlu liberation sans
		const bool arial = compact == "arial";

		boldFound = false;
		std::vector<std::string> names;
		if (bold)
		{
			names.push_back(compact + "bd.ttf");
			names.push_back(compact + "b.ttf");
			names.push_back(compact + "-bold.ttf");
			names.push_back(face + " bold.ttf");
			if (arial)
				names.push_back("liberationsans-bold.ttf");
		}
		const size_t boldCount = names.size();
		names.push_back(compact + ".ttf");
		if (arial)
			names.push_back("liberationsans-regular.ttf");
		names.push_back(face + ".ttf");
		names.push_back(compact + ".otf");
		if (compact.size() > 7 && compact.compare(compact.size() - 7, 7, "regular") == 0)
			names.push_back(compact.substr(0, compact.size() - 7) + ".ttf");

		for (size_t i = 0; i < names.size(); ++i)
		{
			std::string p = FindInFontDir(names[i]);
			if (!p.empty())
			{
				boldFound = bold && i < boldCount;
				return p;
			}
		}

		std::string p = FindInFontDir("default.ttf");
		if (!p.empty())
			return p;
#ifdef __APPLE__
		// Cynex iOS: /system/fonts yok - oyunla gelen yazi tiplerine dus
		if (bold)
		{
			p = FindInFontDir("liberationsans-bold.ttf");
			if (!p.empty())
			{
				boldFound = true;
				return p;
			}
		}
		for (const char* f : { "liberationsans-regular.ttf", "tahoma.ttf" })
		{
			p = FindInFontDir(f);
			if (!p.empty())
				return p;
		}
#endif

		static const char* s_system[] = {
			"/system/fonts/Roboto-Regular.ttf",
			"/system/fonts/NotoSans-Regular.ttf",
			"/system/fonts/DroidSans.ttf",
		};
		static const char* s_systemBold[] = {
			"/system/fonts/Roboto-Bold.ttf",
			"/system/fonts/NotoSans-Bold.ttf",
			"/system/fonts/DroidSans-Bold.ttf",
		};
		if (bold)
			for (const char* f : s_systemBold)
				if (Exists(f))
				{
					boldFound = true;
					return f;
				}
		for (const char* f : s_system)
			if (Exists(f))
				return f;
		return std::string();
	}

	void SetupFontMetrics(SFont* f)
	{
		FT_Face face = f->face;
		const TT_OS2* os2 = (const TT_OS2*)FT_Get_Sfnt_Table(face, FT_SFNT_OS2);
		const double upem = face->units_per_EM ? face->units_per_EM : 2048;
		double winAscent = os2 && os2->usWinAscent ? os2->usWinAscent : face->ascender;
		double winDescent = os2 && os2->usWinDescent ? os2->usWinDescent : -face->descender;
		if (winAscent + winDescent <= 0)
		{
			winAscent = upem * 0.8;
			winDescent = upem * 0.2;
		}

		// Cynex: gdi kurali - pozitif yukseklik hucre (ascent+descent), negatif em boyu
		LONG h = f->log.lfHeight;
		double ppem;
		if (h > 0)
			ppem = h * upem / (winAscent + winDescent);
		else if (h < 0)
			ppem = -h;
		else
			ppem = 12.0;
		int ippem = (int)floor(ppem + 0.5);
		if (ippem < 1)
			ippem = 1;
		FT_Set_Pixel_Sizes(face, 0, ippem);

		const double scale = ippem / upem;
		f->ascent = (int)floor(winAscent * scale + 0.5);
		f->descent = (int)floor(winDescent * scale + 0.5);
		if (h > 0 && f->ascent + f->descent != h)
			f->descent = h - f->ascent;	// Cynex: hucre yuksekligi istenenle ayni kalsin
		f->internalLeading = std::max(0, f->ascent + f->descent - ippem);

		const double avg = os2 && os2->xAvgCharWidth ? os2->xAvgCharWidth : upem * 0.5;
		f->aveWidth = (int)floor(avg * scale + 0.5);
		f->maxWidth = (int)ceil((face->bbox.xMax - face->bbox.xMin) * scale);
	}

	const SGlyph& GetGlyph(SFont* f, uint32_t ch)
	{
		auto it = f->glyphs.find(ch);
		if (it != f->glyphs.end())
			return it->second;

		SGlyph& g = f->glyphs[ch];
		if (!f->face)
			return g;

		FT_UInt index = FT_Get_Char_Index(f->face, ch);
		// Cynex: kucuk boyda windows gibi keskin - tek renk ipuclu cizim
		// Cynex: gomulu bitmap'ler kullanilmaz (wine tahoma'da 'ı' bozuk)
		// AKRAVON: yumusatilmis (gri tonlu) cizim - 1280x720 olceklenmis ekranda kenarlar basamakli gorunmesin
		if (FT_Load_Glyph(f->face, index, FT_LOAD_NO_BITMAP | FT_LOAD_TARGET_LIGHT))
			return g;
		FT_GlyphSlot slot = f->face->glyph;
		if (f->emboldenSynth)
			FT_GlyphSlot_Embolden(slot);
		if (f->italicSynth)
			FT_GlyphSlot_Oblique(slot);
		g.advance = (int)((slot->advance.x + 32) >> 6);

		if (FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL))
			return g;

		const FT_Bitmap& bm = slot->bitmap;
		g.left = slot->bitmap_left;
		g.top = slot->bitmap_top;
		g.width = (int)bm.width;
		g.rows = (int)bm.rows;
		g.mask.resize((size_t)g.width * g.rows);
		for (int y = 0; y < g.rows; ++y)
		{
			const unsigned char* src = bm.buffer + (ptrdiff_t)y * bm.pitch;
			for (int x = 0; x < g.width; ++x)
			{
				unsigned char v;
				// AKRAVON: maske artik 0-255 kaplama degeri
				if (bm.pixel_mode == FT_PIXEL_MODE_MONO)
					v = ((src[x >> 3] >> (7 - (x & 7))) & 1) ? 255 : 0;
				else
					v = src[x];
				g.mask[(size_t)y * g.width + x] = v;
			}
		}
		return g;
	}

	uint32_t ToDibColor(COLORREF c)
	{
		// Cynex: COLORREF 0x00BBGGRR -> DIB 0x00RRGGBB
		return ((c & 0xFF) << 16) | (c & 0xFF00) | ((c >> 16) & 0xFF);
	}

	void FillDib(SBitmap* bmp, int x0, int y0, int x1, int y1, uint32_t color)
	{
		x0 = std::max(x0, 0);
		y0 = std::max(y0, 0);
		x1 = std::min(x1, bmp->width);
		y1 = std::min(y1, bmp->height);
		for (int y = y0; y < y1; ++y)
		{
			uint32_t* row = bmp->Row(y);
			for (int x = x0; x < x1; ++x)
				row[x] = color;
		}
	}

	int TextWidth(SFont* f, const wchar_t* s, int n)
	{
		int w = 0;
		for (int i = 0; i < n; ++i)
			w += GetGlyph(f, (uint32_t)s[i]).advance;
		return w;
	}

	void DrawText(SDC* dc, int x, int y, const wchar_t* s, int n)
	{
		SFont* f = dc->font ? dc->font : g_stockFont;
		SBitmap* bmp = dc->bitmap;
		if (!f || !bmp || !bmp->bits || dc->screen)
			return;

		const int cellH = f->ascent + f->descent;
		int baseline = y + f->ascent;
		if ((dc->align & TA_BASELINE) == TA_BASELINE)
		{
			baseline = y;
			y -= f->ascent;
		}

		if (dc->bkMode == OPAQUE)
			FillDib(bmp, x, y, x + TextWidth(f, s, n), y + cellH, ToDibColor(dc->bkColor));

		const uint32_t color = ToDibColor(dc->textColor);
		int pen = x;
		for (int i = 0; i < n; ++i)
		{
			const SGlyph& g = GetGlyph(f, (uint32_t)s[i]);
			const int gx = pen + g.left;
			const int gy = baseline - g.top;
			for (int row = 0; row < g.rows; ++row)
			{
				const int py = gy + row;
				if (py < 0 || py >= bmp->height)
					continue;
				uint32_t* dst = bmp->Row(py);
				const unsigned char* m = &g.mask[(size_t)row * g.width];
				for (int col = 0; col < g.width; ++col)
				{
					const int px = gx + col;
					if (m[col] && px >= 0 && px < bmp->width)
					{
						// AKRAVON: kaplama oraninda zemine karistir
						const uint32_t a = m[col];
						if (a >= 255)
							dst[px] = color;
						else
						{
							const uint32_t d = dst[px];
							uint32_t out = 0;
							for (int shift = 0; shift <= 16; shift += 8)
							{
								const uint32_t cs = (color >> shift) & 0xFF;
								const uint32_t ds = (d >> shift) & 0xFF;
								out |= ((cs * a + ds * (255 - a) + 127) / 255) << shift;
							}
							dst[px] = out;
						}
					}
				}
			}
			pen += g.advance;
		}
	}

	std::vector<wchar_t> ToWide(LPCSTR s, int n)
	{
		std::vector<wchar_t> w;
		if (!s || n <= 0)
			return w;
		const int len = MultiByteToWideChar(CP_ACP, 0, s, n, nullptr, 0);
		if (len > 0)
		{
			w.resize(len);
			MultiByteToWideChar(CP_ACP, 0, s, n, w.data(), len);
		}
		return w;
	}

	void FillTextMetric(SFont* f, TEXTMETRICA* tm)
	{
		memset(tm, 0, sizeof(*tm));
		if (!f)
			return;
		tm->tmHeight = f->ascent + f->descent;
		tm->tmAscent = f->ascent;
		tm->tmDescent = f->descent;
		tm->tmInternalLeading = f->internalLeading;
		tm->tmAveCharWidth = f->aveWidth;
		tm->tmMaxCharWidth = f->maxWidth;
		tm->tmWeight = f->log.lfWeight ? f->log.lfWeight : FW_NORMAL;
		tm->tmDigitizedAspectX = 96;
		tm->tmDigitizedAspectY = 96;
		tm->tmFirstChar = 0x20;
		tm->tmLastChar = 0xFF;
		tm->tmDefaultChar = 0x1F;
		tm->tmBreakChar = 0x20;
		tm->tmItalic = f->log.lfItalic;
		tm->tmPitchAndFamily = 0x04 /*TMPF_TRUETYPE*/ | VARIABLE_PITCH;
		tm->tmCharSet = f->log.lfCharSet;
	}
}

extern "C" {

void CynexGdi_SetFontDirectory(const char* path)
{
	std::lock_guard<std::recursive_mutex> l(g_lock);
	g_fontDir = path ? path : "";
	while (!g_fontDir.empty() && (g_fontDir.back() == '/' || g_fontDir.back() == '\\'))
		g_fontDir.pop_back();
}

HDC GetDC(HWND)
{
	g_screenDC.screen = true;
	return (HDC)&g_screenDC;
}

int ReleaseDC(HWND, HDC) { return 1; }

HDC CreateCompatibleDC(HDC)
{
	SDC* dc = new SDC;
	dc->bitmap = &g_stockBitmap;
	return (HDC)dc;
}

BOOL DeleteDC(HDC h)
{
	SDC* dc = DC(h);
	if (!dc || dc == &g_screenDC)
		return FALSE;
	delete dc;
	return TRUE;
}

HBITMAP CreateDIBSection(HDC, const BITMAPINFO* bmi, UINT, void** bits, HANDLE, DWORD)
{
	if (!bmi || bmi->bmiHeader.biBitCount != 32 || bmi->bmiHeader.biWidth <= 0 || bmi->bmiHeader.biHeight == 0)
	{
		// Cynex: oyun yalnizca 32 bit kullaniyor
		if (bits)
			*bits = nullptr;
		return nullptr;
	}
	SBitmap* b = new SBitmap;
	b->width = bmi->bmiHeader.biWidth;
	b->height = abs(bmi->bmiHeader.biHeight);
	b->topDown = bmi->bmiHeader.biHeight < 0;
	b->bits = (uint32_t*)calloc((size_t)b->width * b->height, 4);
	if (!b->bits)
	{
		delete b;
		return nullptr;
	}
	if (bits)
		*bits = b->bits;
	return (HBITMAP)b;
}

HBITMAP CreateCompatibleBitmap(HDC, int cx, int cy)
{
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = cx;
	bmi.bmiHeader.biHeight = -cy;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	return CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, nullptr, nullptr, 0);
}

HGDIOBJ SelectObject(HDC h, HGDIOBJ obj)
{
	std::lock_guard<std::recursive_mutex> l(g_lock);
	SDC* dc = DC(h);
	SGdiObject* o = (SGdiObject*)obj;
	if (!dc || !o)
		return nullptr;
	switch (o->type)
	{
		case GDI_BITMAP:
		{
			SBitmap* old = dc->bitmap;
			dc->bitmap = (SBitmap*)o;
			return (HGDIOBJ)old;
		}
		case GDI_FONT:
		{
			SFont* old = dc->font ? dc->font : g_stockFont;
			dc->font = (SFont*)o;
			return (HGDIOBJ)old;
		}
		case GDI_BRUSH:
		{
			SBrush* old = dc->brush ? dc->brush : &g_stockBrushes[WHITE_BRUSH];
			dc->brush = (SBrush*)o;
			return (HGDIOBJ)old;
		}
	}
	return nullptr;
}

BOOL DeleteObject(HGDIOBJ obj)
{
	std::lock_guard<std::recursive_mutex> l(g_lock);
	SGdiObject* o = (SGdiObject*)obj;
	if (!o || o == &g_stockBitmap || o == g_stockFont)
		return FALSE;
	if (o->type == GDI_BRUSH && ((SBrush*)o)->stock)
		return TRUE;
	if (o->type != GDI_BITMAP && o->type != GDI_FONT && o->type != GDI_BRUSH)
		return FALSE;
	delete o;
	return TRUE;
}

HGDIOBJ GetStockObject(int i)
{
	if (i < 0 || i >= 6)
		i = WHITE_BRUSH;
	SBrush& b = g_stockBrushes[i];
	b.stock = true;
	b.color = i == BLACK_BRUSH ? RGB(0, 0, 0) : RGB(255, 255, 255);
	return (HGDIOBJ)&b;
}

int GetObjectA(HANDLE h, int c, LPVOID pv)
{
	SGdiObject* o = (SGdiObject*)h;
	if (!o)
		return 0;
	if (o->type == GDI_BITMAP && pv && c >= (int)sizeof(BITMAP))
	{
		SBitmap* b = (SBitmap*)o;
		BITMAP* out = (BITMAP*)pv;
		out->bmType = 0;
		out->bmWidth = b->width;
		out->bmHeight = b->height;
		out->bmWidthBytes = b->width * 4;
		out->bmPlanes = 1;
		out->bmBitsPixel = 32;
		out->bmBits = b->bits;
		return sizeof(BITMAP);
	}
	if (o->type == GDI_FONT && pv && c >= (int)sizeof(LOGFONTA))
	{
		*(LOGFONTA*)pv = ((SFont*)o)->log;
		return sizeof(LOGFONTA);
	}
	return 0;
}

HFONT CreateFontIndirectA(const LOGFONT* lf)
{
	std::lock_guard<std::recursive_mutex> l(g_lock);
	if (!lf || !InitFreeType())
		return nullptr;

	SFont* f = new SFont;
	f->log = *lf;
	const bool bold = lf->lfWeight >= FW_SEMIBOLD;
	bool boldFound = false;
	const std::string path = ResolveFontFile(lf->lfFaceName, bold, boldFound);
	if (path.empty() || FT_New_Face(g_ft, path.c_str(), 0, &f->face))
	{
		__android_log_print(ANDROID_LOG_ERROR, "CynexMobil", "yazi tipi bulunamadi: %s", lf->lfFaceName);
		f->face = nullptr;
		return (HFONT)f;
	}
	FT_Select_Charmap(f->face, FT_ENCODING_UNICODE);
	f->emboldenSynth = bold && !boldFound;
	f->italicSynth = lf->lfItalic && !(f->face->style_flags & FT_STYLE_FLAG_ITALIC);
	SetupFontMetrics(f);
	return (HFONT)f;
}

HFONT CreateFontA(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic, DWORD bUnderline,
	DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCSTR pszFaceName)
{
	LOGFONTA lf = {};
	lf.lfHeight = cHeight;
	lf.lfWidth = cWidth;
	lf.lfEscapement = cEscapement;
	lf.lfOrientation = cOrientation;
	lf.lfWeight = cWeight;
	lf.lfItalic = (BYTE)bItalic;
	lf.lfUnderline = (BYTE)bUnderline;
	lf.lfStrikeOut = (BYTE)bStrikeOut;
	lf.lfCharSet = (BYTE)iCharSet;
	lf.lfOutPrecision = (BYTE)iOutPrecision;
	lf.lfClipPrecision = (BYTE)iClipPrecision;
	lf.lfQuality = (BYTE)iQuality;
	lf.lfPitchAndFamily = (BYTE)iPitchAndFamily;
	if (pszFaceName)
		strncpy(lf.lfFaceName, pszFaceName, LF_FACESIZE - 1);
	return CreateFontIndirectA(&lf);
}

COLORREF SetTextColor(HDC h, COLORREF color)
{
	SDC* dc = DC(h);
	if (!dc)
		return CLR_INVALID;
	COLORREF old = dc->textColor;
	dc->textColor = color;
	return old;
}

COLORREF SetBkColor(HDC h, COLORREF color)
{
	SDC* dc = DC(h);
	if (!dc)
		return CLR_INVALID;
	COLORREF old = dc->bkColor;
	dc->bkColor = color;
	return old;
}

int SetBkMode(HDC h, int mode)
{
	SDC* dc = DC(h);
	if (!dc)
		return 0;
	int old = dc->bkMode;
	dc->bkMode = mode;
	return old;
}

UINT SetTextAlign(HDC h, UINT align)
{
	SDC* dc = DC(h);
	if (!dc)
		return GDI_ERROR;
	UINT old = dc->align;
	dc->align = align;
	return old;
}

int SetMapMode(HDC, int) { return MM_TEXT; }

BOOL TextOutW(HDC h, int x, int y, LPCWSTR s, int c)
{
	std::lock_guard<std::recursive_mutex> l(g_lock);
	SDC* dc = DC(h);
	if (!dc || !s)
		return FALSE;
	DrawText(dc, x, y, s, c);
	return TRUE;
}

BOOL TextOutA(HDC h, int x, int y, LPCSTR s, int c)
{
	std::vector<wchar_t> w = ToWide(s, c);
	return TextOutW(h, x, y, w.data(), (int)w.size());
}

BOOL GetTextExtentPoint32W(HDC h, LPCWSTR s, int c, LPSIZE size)
{
	std::lock_guard<std::recursive_mutex> l(g_lock);
	SDC* dc = DC(h);
	if (!dc || !size)
		return FALSE;
	SFont* f = dc->font ? dc->font : g_stockFont;
	if (!f)
	{
		size->cx = size->cy = 0;
		return FALSE;
	}
	size->cx = s ? TextWidth(f, s, c) : 0;
	size->cy = f->ascent + f->descent;
	return TRUE;
}

BOOL GetTextExtentPoint32A(HDC h, LPCSTR s, int c, LPSIZE size)
{
	std::vector<wchar_t> w = ToWide(s, c);
	return GetTextExtentPoint32W(h, w.data(), (int)w.size(), size);
}

BOOL GetCharABCWidthsFloatW(HDC h, UINT first, UINT last, LPABCFLOAT abc)
{
	std::lock_guard<std::recursive_mutex> l(g_lock);
	SDC* dc = DC(h);
	SFont* f = dc ? (dc->font ? dc->font : g_stockFont) : nullptr;
	if (!f || !abc || last < first)
		return FALSE;
	for (UINT ch = first; ch <= last; ++ch, ++abc)
	{
		const SGlyph& g = GetGlyph(f, ch);
		if (g.width == 0)
		{
			// Cynex: bosluk gibi gorunmez karakter - tum genislik B
			abc->abcfA = 0.0f;
			abc->abcfB = (FLOAT)g.advance;
			abc->abcfC = 0.0f;
		}
		else
		{
			abc->abcfA = (FLOAT)g.left;
			abc->abcfB = (FLOAT)g.width;
			abc->abcfC = (FLOAT)(g.advance - g.left - g.width);
		}

	}
	return TRUE;
}

BOOL GetCharABCWidthsW(HDC h, UINT first, UINT last, LPABC abc)
{
	if (!abc || last < first)
		return FALSE;
	for (UINT ch = first; ch <= last; ++ch, ++abc)
	{
		ABCFLOAT f;
		if (!GetCharABCWidthsFloatW(h, ch, ch, &f))
			return FALSE;
		abc->abcA = (int)f.abcfA;
		abc->abcB = (UINT)f.abcfB;
		abc->abcC = (int)f.abcfC;
	}
	return TRUE;
}

BOOL GetTextMetricsA(HDC h, LPTEXTMETRIC tm)
{
	SDC* dc = DC(h);
	if (!dc || !tm)
		return FALSE;
	FillTextMetric(dc->font ? dc->font : g_stockFont, tm);
	return TRUE;
}

BOOL GetTextMetricsW(HDC h, LPTEXTMETRICW tm)
{
	return GetTextMetricsA(h, tm);
}

int GetDeviceCaps(HDC, int index)
{
	switch (index)
	{
		case LOGPIXELSX:
		case LOGPIXELSY:
			return 96;
		case BITSPIXEL:
			return 32;
		case VREFRESH:
			return 60;
	}
	return 0;
}

int FillRect(HDC h, const RECT* rc, HBRUSH hbr)
{
	SDC* dc = DC(h);
	SBrush* br = As<SBrush>((HGDIOBJ)hbr, GDI_BRUSH);
	if (!dc || !rc || !dc->bitmap || !dc->bitmap->bits)
		return 0;
	FillDib(dc->bitmap, rc->left, rc->top, rc->right, rc->bottom, ToDibColor(br ? br->color : 0));
	return 1;
}

HBRUSH CreateSolidBrush(COLORREF color)
{
	SBrush* b = new SBrush;
	b->color = color;
	return (HBRUSH)b;
}

BOOL PatBlt(HDC h, int x, int y, int w, int hgt, DWORD rop)
{
	SDC* dc = DC(h);
	if (!dc || !dc->bitmap || !dc->bitmap->bits)
		return FALSE;
	uint32_t color;
	if (rop == BLACKNESS)
		color = 0;
	else if (rop == WHITENESS)
		color = 0x00FFFFFF;
	else
		color = ToDibColor(dc->brush ? dc->brush->color : RGB(255, 255, 255));
	FillDib(dc->bitmap, x, y, x + w, y + hgt, color);
	return TRUE;
}

BOOL BitBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD)
{
	SDC* dst = DC(hdc);
	SDC* src = DC(hdcSrc);
	if (!dst || !src || !dst->bitmap || !src->bitmap || !dst->bitmap->bits || !src->bitmap->bits)
		return TRUE;	// Cynex: ekrana kopyalama yok
	for (int j = 0; j < cy; ++j)
	{
		const int sy = y1 + j, dy = y + j;
		if (sy < 0 || dy < 0 || sy >= src->bitmap->height || dy >= dst->bitmap->height)
			continue;
		for (int i = 0; i < cx; ++i)
		{
			const int sx = x1 + i, dx = x + i;
			if (sx >= 0 && dx >= 0 && sx < src->bitmap->width && dx < dst->bitmap->width)
				dst->bitmap->Row(dy)[dx] = src->bitmap->Row(sy)[sx];
		}
	}
	return TRUE;
}

BOOL StretchBlt(HDC, int, int, int, int, HDC, int, int, int, int, DWORD) { return TRUE; }
BOOL GdiFlush(void) { return TRUE; }

int SetDIBitsToDevice(HDC, int, int, DWORD, DWORD h, int, int, UINT, UINT lines, const VOID*, const BITMAPINFO*, UINT)
{
	// Cynex: dogrudan ekrana cizim yok (d3d ile ciziliyor)
	return (int)(lines ? lines : h);
}

int StretchDIBits(HDC, int, int, int, int, int, int, int, int srcHeight, const VOID*, const BITMAPINFO*, UINT, DWORD)
{
	return srcHeight;
}

int EnumFontFamiliesExA(HDC, LPLOGFONTA lf, FONTENUMPROCA proc, LPARAM lParam, DWORD)
{
	// Cynex: tahoma/arial her zaman var sayilir (yoksa sistem yazisina duser)
	if (!proc)
		return 1;
	std::vector<std::string> faces = { "Tahoma", "Arial" };
	if (!g_fontDir.empty())
		if (DIR* d = opendir(g_fontDir.c_str()))
		{
			while (dirent* e = readdir(d))
			{
				std::string n = e->d_name;
				const std::string ln = Lower(n.c_str());
				if (ln.size() > 4 && (ln.compare(ln.size() - 4, 4, ".ttf") == 0 || ln.compare(ln.size() - 4, 4, ".otf") == 0))
					faces.push_back(n.substr(0, n.size() - 4));
			}
			closedir(d);
		}

	int ret = 1;
	for (const std::string& face : faces)
	{
		LOGFONTA out = {};
		out.lfCharSet = lf ? lf->lfCharSet : DEFAULT_CHARSET;
		out.lfWeight = FW_NORMAL;
		strncpy(out.lfFaceName, face.c_str(), LF_FACESIZE - 1);
		TEXTMETRICA tm = {};
		ret = proc(&out, &tm, 4 /*TRUETYPE_FONTTYPE*/, lParam);
		if (ret == 0)
			break;
	}
	return ret;
}

}	// extern "C"
