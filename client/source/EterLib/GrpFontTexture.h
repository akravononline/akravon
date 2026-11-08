#pragma once

#include "GrpTexture.h"
#include "GrpImageTexture.h"
#include "GrpDIB.h"

#include <vector>
#include <map>

class CGraphicFontTexture : public CGraphicTexture
{
public:
	typedef std::pair<WORD,wchar_t> TCharacterKey;

	typedef struct SCharacterInfomation
	{
		short index;
		short width;
		short height;
		float left;
		float top;
		float right;
		float bottom;
		float advance;
		float drawWidth;	// AKRAVON: dokudaki hucrenin mantiksal boyutu (cizim dortgeni)
		float drawHeight;
		float padLeft;		// AKRAVON: pisirilmis kontur icin hucre dolgusu (mantiksal px)
		float padTop;
	} TCharacterInfomation;

	typedef std::vector<TCharacterInfomation*>		TPCharacterInfomationVector;

public:
	CGraphicFontTexture();
	virtual ~CGraphicFontTexture();

	void Destroy();
	bool Create(const char* c_szFontName, int fontSize, bool bItalic);

	bool CreateDeviceObjects();
	void DestroyDeviceObjects();

	bool CheckTextureIndex(DWORD dwTexture);
	void SelectTexture(DWORD dwTexture);

	#ifdef ENABLE_FIX_MOBS_LAG
	CGraphicImageTexture* GetTexture(DWORD dwTexture);
	#endif

	bool UpdateTexture();

	TCharacterInfomation* GetCharacterInfomation(WORD codePage, wchar_t keyValue);
	TCharacterInfomation* UpdateCharacterInfomation(TCharacterKey code);

	bool IsEmpty() const;
	float GetScale() const { return m_scale; }
	bool HasBakedOutline() const { return m_bakedOutline; }

protected:
	bool m_bakedOutline = false;
	void Initialize();

	bool AppendTexture();

	HFONT GetFont(WORD codePage);

protected:
	typedef std::vector<CGraphicImageTexture*>				TGraphicImageTexturePointerVector;
	typedef std::map<TCharacterKey, TCharacterInfomation>	TCharacterInfomationMap;
	typedef std::map<WORD, HFONT>							TFontMap;

protected:
	CGraphicDib	m_dib;

	HFONT	m_hFontOld;
	HFONT	m_hFont;

	TGraphicImageTexturePointerVector m_pFontTextureVector;

	TCharacterInfomationMap m_charInfoMap;

	TFontMap m_fontMap;

	int m_x;
	int m_y;
	int m_step;
	bool m_isDirty;

	TCHAR	m_fontName[LF_FACESIZE];
	LONG	m_fontSize;
	bool	m_bItalic;
	float	m_scale;	// AKRAVON: harfler ekran olceginde cizilir, olculer mantiksal kalir
};
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
