#ifndef __INC_TERRAINLIB_TEXTURESET_H__
#define __INC_TERRAINLIB_TEXTURESET_H__

#include "../eterLib/GrpImageInstance.h"

typedef struct STerrainTexture
{
	STerrainTexture() :	pd3dTexture(NULL),
		UScale(4.0f),
		VScale(4.0f),
		UOffset(0.0f),
		VOffset(0.0f),
		bSplat(true),
		Begin(0),
		End(0)
	{
	}

	~STerrainTexture()
	{
	}

	std::string					stFilename;
	LPDIRECT3DTEXTURE8			pd3dTexture;
	CGraphicImageInstance 		ImageInstance;
	float						UScale;
	float						VScale;
	float						UOffset;
	float						VOffset;
	bool						bSplat;
	unsigned short				Begin, End;
	D3DXMATRIX					m_matTransform;
} TTerrainTexture;

class CTextureSet
{
	public:
		typedef std::vector<TTerrainTexture> TTextureVector;

		CTextureSet();
		virtual ~CTextureSet();

		void			Initialize();
		void			Clear();

		void			Create();

		bool			Load(const char * c_pszFileName, float fTerrainTexCoordBase);
		bool			Save(const char * c_pszFileName);

		DWORD	GetTextureCount();

		TTerrainTexture	& GetTexture(DWORD ulIndex);
		bool			RemoveTexture(DWORD ulIndex);

		bool			SetTexture(DWORD ulIndex,
								   const char * c_szFileName,
			 					   float fuScale,
							       float fvScale,
								   float fuOffset,
								   float fvOffset,
								   bool bSplat,
								   unsigned short usBegin,
								   unsigned short usEnd,
								   float fTerrainTexCoordBase);

		void			Reload(float fTerrainTexCoordBase);

		bool			AddTexture(const char * c_szFileName,
			 					   float fuScale,
							       float fvScale,
								   float fuOffset,
								   float fvOffset,
								   bool bSplat,
								   unsigned short usBegin,
								   unsigned short usEnd,
								   float fTerrainTexCoordBase);

		const char *	GetFileName()	{ return m_stFileName.c_str(); }

	protected:
		void			AddEmptyTexture();

	protected:
		TTextureVector			m_Textures;
		TTerrainTexture			m_ErrorTexture;
		std::string				m_stFileName;
};

#endif
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
