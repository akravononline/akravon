#include "StdAfx.h"
#include "GrpText.h"
#include "../eterBase/Stl.h"

#include "Util.h"

#ifdef CYNEX_MOBILE
float CynexD3D_UIScale();
#endif

CGraphicFontTexture::CGraphicFontTexture()
{
	Initialize();
}

CGraphicFontTexture::~CGraphicFontTexture()
{
	Destroy();
}

void CGraphicFontTexture::Initialize()
{
	CGraphicTexture::Initialize();
	m_hFontOld = NULL;
	m_hFont = NULL;
	m_isDirty = false;
	m_bItalic = false;
	m_scale = 1.0f;
}

bool CGraphicFontTexture::IsEmpty() const
{
	return m_fontMap.size() == 0;
}

void CGraphicFontTexture::Destroy()
{
	HDC hDC = m_dib.GetDCHandle();
	if (hDC)
		SelectObject(hDC, m_hFontOld);

	m_dib.Destroy();

	m_lpd3dTexture = NULL;
	CGraphicTexture::Destroy();
	stl_wipe(m_pFontTextureVector);
	m_charInfoMap.clear();

	if (m_fontMap.size())
	{
		TFontMap::iterator i = m_fontMap.begin();

		while(i != m_fontMap.end())
		{
			DeleteObject((HGDIOBJ)i->second);
			++i;
		}

		m_fontMap.clear();
	}

	Initialize();
}

bool CGraphicFontTexture::CreateDeviceObjects()
{
	return true;
}

void CGraphicFontTexture::DestroyDeviceObjects()
{
}

bool CGraphicFontTexture::Create(const char* c_szFontName, int fontSize, bool bItalic)
{
	Destroy();

	strncpy(m_fontName, c_szFontName, sizeof(m_fontName)-1);
	m_fontSize	= fontSize;
	m_bItalic	= bItalic;
	m_bakedOutline = (strstr(m_fontName, "NotoSansSb") != NULL);	// AKRAVON: isim etiketi yazisi - kontur harfin icine pisirilir

	m_x = 0;
	m_y = 0;
	m_step = 0;

	DWORD width = 256,height = 256;
	if (GetMaxTextureWidth() > 512)
		width = 512;
	if (GetMaxTextureHeight() > 512)
		height = 512;

#ifdef CYNEX_MOBILE
	m_scale = std::min(std::max(CynexD3D_UIScale(), 1.0f), 2.5f);
	if (m_scale > 1.0f)
	{
		width = 1024;
		height = 1024;
	}
#endif

	if (!m_dib.Create(ms_hDC, width, height))
		return false;

	HDC hDC = m_dib.GetDCHandle();

	m_hFont = GetFont(GetDefaultCodePage());

	m_hFontOld=(HFONT)SelectObject(hDC, m_hFont);
	SetTextColor(hDC, RGB(255, 255, 255));
	SetBkColor(hDC,	0);

	if (!AppendTexture())
		return false;

	return true;
}



HFONT CGraphicFontTexture::GetFont(WORD codePage)
{
	HFONT hFont = NULL;
	TFontMap::iterator i = m_fontMap.find(codePage);

	if(i != m_fontMap.end())
	{
		hFont = i->second;
	}
	else
	{
		LOGFONT logFont;

		memset(&logFont, 0, sizeof(LOGFONT));

		logFont.lfHeight			= m_fontSize;
#ifdef CYNEX_MOBILE
		logFont.lfHeight			= (LONG)floorf(m_fontSize * m_scale + (m_fontSize < 0 ? -0.5f : 0.5f));
#endif
		logFont.lfEscapement		= 0;
		logFont.lfOrientation		= 0;
		logFont.lfWeight			= FW_NORMAL;
		logFont.lfItalic			= (BYTE) m_bItalic;
		logFont.lfUnderline			= FALSE;
		logFont.lfStrikeOut			= FALSE;
		logFont.lfCharSet			= GetCharsetFromCodePage(codePage);
		logFont.lfOutPrecision		= OUT_DEFAULT_PRECIS;
		logFont.lfClipPrecision		= CLIP_DEFAULT_PRECIS;
		logFont.lfQuality			= ANTIALIASED_QUALITY;
		logFont.lfPitchAndFamily	= DEFAULT_PITCH;
		//Tracenf("font: %s", GetFontFaceFromCodePage(codePage));
		strcpy(logFont.lfFaceName, m_fontName); //GetFontFaceFromCodePage(codePage));
		//strcpy(logFont.lfFaceName, GetFontFaceFromCodePage(codePage));

		hFont = CreateFontIndirect(&logFont);

		m_fontMap.insert(TFontMap::value_type(codePage, hFont));
	}

	return hFont;
}

bool CGraphicFontTexture::AppendTexture()
{
	CGraphicImageTexture * pNewTexture = new CGraphicImageTexture;

#ifdef CYNEX_MOBILE
	const D3DFORMAT format = D3DFMT_A8R8G8B8;	// AKRAVON: 256 seviye kenar yumusatma
#else
	const D3DFORMAT format = D3DFMT_A4R4G4B4;
#endif
	if (!pNewTexture->Create(m_dib.GetWidth(), m_dib.GetHeight(), format))
	{
		delete pNewTexture;
		return false;
	}

	m_pFontTextureVector.push_back(pNewTexture);
	return true;
}

bool CGraphicFontTexture::UpdateTexture()
{
	if(!m_isDirty)
		return true;

	m_isDirty = false;

	CGraphicImageTexture * pFontTexture = m_pFontTextureVector.back();

	if (!pFontTexture)
		return false;

	WORD* pwDst;
	int pitch;

	if (!pFontTexture->Lock(&pitch, (void**)&pwDst))
		return false;

	int width = m_dib.GetWidth();
	int height = m_dib.GetHeight();

	DWORD * pdwSrc = (DWORD*)m_dib.GetPointer();

#ifdef CYNEX_MOBILE
	DWORD* pdwDst = (DWORD*)pwDst;
	pitch /= 4;
	for (int y = 0; y < height; ++y, pdwDst += pitch, pdwSrc += width)
		memcpy(pdwDst, pdwSrc, width * sizeof(DWORD));
#else
	pitch /= 2;
	for (int y = 0; y < height; ++y, pwDst += pitch, pdwSrc += width)
		for (int x = 0; x < width; ++x)
			pwDst[x]=pdwSrc[x];
#endif

	pFontTexture->Unlock();
	return true;
}

CGraphicFontTexture::TCharacterInfomation* CGraphicFontTexture::GetCharacterInfomation(WORD codePage, wchar_t keyValue)
{
	TCharacterKey code(codePage, keyValue);

	TCharacterInfomationMap::iterator f = m_charInfoMap.find(code);

	if (m_charInfoMap.end() == f)
	{
		return UpdateCharacterInfomation(code);
	}
	else
	{
		return &f->second;
	}
}

CGraphicFontTexture::TCharacterInfomation* CGraphicFontTexture::UpdateCharacterInfomation(TCharacterKey code)
{
	HDC hDC = m_dib.GetDCHandle();
	SelectObject(hDC, GetFont(code.first));

	wchar_t keyValue = code.second;

	if (keyValue == 0x08)
		keyValue = L' ';

	ABCFLOAT	stABC;
	SIZE		size;

	if (!GetTextExtentPoint32W(hDC, &keyValue, 1, &size) || !GetCharABCWidthsFloatW(hDC, keyValue, keyValue, &stABC))
		return NULL;

	size.cx = stABC.abcfB;
	if( stABC.abcfA > 0.0f )
		size.cx += ceilf(stABC.abcfA);
	if( stABC.abcfC > 0.0f )
		size.cx += ceilf(stABC.abcfC);
	size.cx++;

	LONG lAdvance = ceilf( stABC.abcfA + stABC.abcfB + stABC.abcfC );

	int width = m_dib.GetWidth();
	int height = m_dib.GetHeight();
	int bakePad = 0;	// fiziksel px
	const float kOutlineR = 0.25f;	// mantiksal px (en ince kontur)
	const float kOutlineAlpha = 0.6f;	// AKRAVON: kontur koyulugu (1 = tam siyah)

	// AKRAVON: harf ekran olceginde cizilir; mantiksal olcu = fiziksel / olcek, doku hucresi tam olcek katidir
	int logicalW = size.cx, logicalH = size.cy;
	int cellW = size.cx, cellH = size.cy;
#ifdef CYNEX_MOBILE
	if (m_scale > 1.0f)
	{
		logicalW = (int)ceilf(size.cx / m_scale);
		logicalH = (int)ceilf(size.cy / m_scale);
		cellW = (int)ceilf(logicalW * m_scale);
		cellH = (int)ceilf(logicalH * m_scale);
		lAdvance = (LONG)floorf(lAdvance / m_scale + 0.5f);
	}
	if (m_bakedOutline)
	{
		bakePad = (int)ceilf(kOutlineR * (m_scale > 1.0f ? m_scale : 1.0f)) + 1;
		cellW += bakePad * 2;
		cellH += bakePad * 2;
		logicalW = (int)ceilf(cellW / (m_scale > 1.0f ? m_scale : 1.0f));
		logicalH = (int)ceilf(cellH / (m_scale > 1.0f ? m_scale : 1.0f));
	}
#endif

	if (m_x + cellW >= (width - 1))
	{
		m_y += (m_step + 1);
		m_step = 0;
		m_x = 0;

		if (m_y + cellH >= (height - 1))
		{
			if (!UpdateTexture())
			{
				return NULL;
			}

			if (!AppendTexture())
				return NULL;

			m_y = 0;
		}
	}

	TextOutW(hDC, m_x + bakePad, m_y + bakePad, &keyValue, 1);

	int nChrX;
	int nChrY;
	int nChrWidth = size.cx;
	int nChrHeight = size.cy;
	int nDIBWidth = m_dib.GetWidth();


	DWORD*pdwDIBData=(DWORD*)m_dib.GetPointer();
	DWORD*pdwDIBBase=pdwDIBData+nDIBWidth*m_y+m_x;
	DWORD*pdwDIBRow;

	pdwDIBRow=pdwDIBBase;
#ifdef CYNEX_MOBILE
	nChrWidth = cellW;
	nChrHeight = cellH;
#endif
#ifdef CYNEX_MOBILE
	if (m_bakedOutline)
	{
		// AKRAVON: kontur harfin kendisinden hesaplanir (yuvarlak, kenar yumusatmali): siyah kontur + beyaz dolgu, tek dokuda
		const int cw = nChrWidth, ch = nChrHeight;
		std::vector<unsigned char> src((size_t)cw * ch, 0);
		for (int y = 0; y < ch; ++y)
			for (int x = 0; x < cw; ++x)
			{
				const int gx = x - bakePad, gy = y - bakePad;
				if (gx >= 0 && gy >= 0 && gx < size.cx && gy < size.cy)
					src[(size_t)y * cw + x] = (unsigned char)(pdwDIBBase[(size_t)y * nDIBWidth + x] & 0xff);
			}
		const float R = kOutlineR * (m_scale > 1.0f ? m_scale : 1.0f);
		const int ri = (int)ceilf(R) + 1;
		for (int y = 0; y < ch; ++y)
			for (int x = 0; x < cw; ++x)
			{
				const float v = src[(size_t)y * cw + x] / 255.0f;
				float o = v;
				for (int dy = -ri; dy <= ri; ++dy)
					for (int dx = -ri; dx <= ri; ++dx)
					{
						const int sx = x + dx, sy = y + dy;
						if (sx < 0 || sy < 0 || sx >= cw || sy >= ch)
							continue;
						const float sv = src[(size_t)sy * cw + sx] / 255.0f;
						if (sv <= o)
							continue;
						float cover = R + 0.5f - sqrtf(float(dx * dx + dy * dy));
						cover = cover < 0.0f ? 0.0f : (cover > 1.0f ? 1.0f : cover);
						const float t = sv * cover * kOutlineAlpha;
						if (t > o)
							o = t;
					}
				const float a = v + o * (1.0f - v);
				DWORD px = 0;
				if (a > 0.003f)
				{
					const float rgb = (v / a) * 255.0f;
					const DWORD A = (DWORD)(a * 255.0f + 0.5f);
					const DWORD C = (DWORD)(rgb + 0.5f);
					px = (A << 24) | (C << 16) | (C << 8) | C;
				}
				pdwDIBBase[(size_t)y * nDIBWidth + x] = px;
			}
	}
	else
#endif
	for (nChrY=0; nChrY<nChrHeight; ++nChrY, pdwDIBRow+=nDIBWidth)
	{
		for (nChrX=0; nChrX<nChrWidth; ++nChrX)
		{
#ifdef CYNEX_MOBILE
			// AKRAVON: beyaz yazinin parlakligi = saydamlik (A8R8G8B8, 256 seviye); hucre dolgusu temizlenir
			{
				const DWORD v = (nChrX < size.cx && nChrY < size.cy) ? (pdwDIBRow[nChrX] & 0xff) : 0;
				pdwDIBRow[nChrX] = v ? ((v << 24) | 0x00ffffff) : 0;
			}
#else
			pdwDIBRow[nChrX]=(pdwDIBRow[nChrX]&0xff) ? 0xffff : 0;
#endif
		}
	}

	float rhwidth = 1.0f / float(width);
	float rhheight = 1.0f / float(height);

	TCharacterInfomation& rNewCharInfo = m_charInfoMap[code];

	rNewCharInfo.index = m_pFontTextureVector.size() - 1;
	rNewCharInfo.width = logicalW;
	rNewCharInfo.height = logicalH;
	rNewCharInfo.left = float(m_x) * rhwidth;
	rNewCharInfo.top = float(m_y) * rhheight;
	rNewCharInfo.right = float(m_x + cellW) * rhwidth;
	rNewCharInfo.bottom = float(m_y + cellH) * rhheight;
	rNewCharInfo.advance = (float) lAdvance;
	rNewCharInfo.drawWidth = float(cellW) / m_scale;
	rNewCharInfo.drawHeight = float(cellH) / m_scale;
	rNewCharInfo.padLeft = rNewCharInfo.padTop = m_bakedOutline ? float(bakePad) / (m_scale > 1.0f ? m_scale : 1.0f) : 0.0f;

	static constexpr auto CHAR_SPACING = 2;	 // appending empty space between characters
	m_x += cellW + CHAR_SPACING;

	if (m_step < cellH + CHAR_SPACING)
		m_step = cellH + CHAR_SPACING;

	m_isDirty = true;

	return &rNewCharInfo;
}

bool CGraphicFontTexture::CheckTextureIndex(DWORD dwTexture)
{
	if (dwTexture >= m_pFontTextureVector.size())
		return false;

	return true;
}

void CGraphicFontTexture::SelectTexture(DWORD dwTexture)
{
	assert(CheckTextureIndex(dwTexture));
	m_lpd3dTexture = m_pFontTextureVector[dwTexture]->GetD3DTexture();
}

#ifdef ENABLE_FIX_MOBS_LAG
CGraphicImageTexture * CGraphicFontTexture::GetTexture(DWORD dwTexture)
{
	assert(CheckTextureIndex(dwTexture));
	return m_pFontTextureVector[dwTexture];
}
#endif

//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
