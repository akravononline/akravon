#ifndef __INC_GRPIMAGE_H__
#define __INC_GRPIMAGE_H__

#include "Ref.h"
#include "Resource.h"
#include "GrpImageTexture.h"

class CGraphicImage : public CResource
{
	public:
		typedef CRef<CGraphicImage> TRef;

	public:
		static TType Type();

	public:
		CGraphicImage(const char* c_szFileName, DWORD dwFilter = D3DX_FILTER_LINEAR);
		#ifdef ENABLE_GIF_SUPPORT
		CGraphicImage(const char* c_szFileName, const void* c_pvBuf, int32_t iSize, uint32_t dwFilter = D3DX_FILTER_LINEAR);
		#endif
		virtual ~CGraphicImage();

		virtual bool CreateDeviceObjects();
		virtual void DestroyDeviceObjects();

		int GetWidth() const;
		int GetHeight() const;

		const RECT & GetRectReference() const;

		const CGraphicTexture & GetTextureReference() const;
		CGraphicTexture * GetTexturePointer();

	protected:
		bool OnLoad(int iSize, const void * c_pvBuf);

		void OnClear();
		bool OnIsEmpty() const;
		bool OnIsType(TType type);

	protected:
		CGraphicImageTexture	m_imageTexture;
		RECT					m_rect;
		DWORD					m_dwFilter;
		int m_iLogicalDiv = 1;	// AKRAVON: yuksek cozunurluklu esya ikonu mantiksal bolen
};

#endif
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
