///////////////////////////////////////////////////////////////////////
//	CynexSptModel.h
//
//	Cynex: CSpeedTreeRT ic modeli. Dosya verisi + bir agacin tum orneklerinin
//	Cynex: paylastigi durum (ruzgar, sallanma, LOD sinirlari, yaprak kose onbellegi).
//	Cynex: kare basina yollar (GetGeometry) bellek ayirmaz; tum tamponlar Compute'ta kurulur.

#pragma once

#include <SpeedTreeRT.h>

#include <string>
#include <vector>

#include "CynexSptFormat.h"

struct SCynexSptIndexed
{
	unsigned int						m_uiVertexCount;
	uint32_t							m_uiMask;
	std::vector<float>					m_vecCoords;
	std::vector<float>					m_vecNormals;
	std::vector<float>					m_vecBinormals;
	std::vector<float>					m_vecTangents;
	std::vector<float>					m_vecTexCoords0;		// dosyadaki (cevrilmemis)
	std::vector<float>					m_vecTexCoords1;
	std::vector<unsigned long>			m_vecColors;
	std::vector<float>					m_vecWindWeights;
	std::vector<unsigned char>			m_vecWindIndices;
	std::vector<float>					m_vecLocalUV;
	std::vector<unsigned char>			m_vecFrondMap;

	// Cynex: LOD strip tablolari
	std::vector<unsigned short>			m_vecIndices;
	std::vector<unsigned short>			m_vecLodStripCount;		// lod basina strip sayisi
	std::vector<unsigned int>			m_vecLodStripStart;		// lod basina ilk strip (m_vecStripLengths icinde)
	std::vector<unsigned short>			m_vecStripLengths;
	std::vector<unsigned int>			m_vecStripOffsets;		// m_vecIndices icinde
	std::vector<const unsigned short*>	m_vecStripPointers;
	std::vector<std::vector<unsigned char> > m_vecLodMapIndexes;
	std::vector<std::vector<unsigned short> > m_vecLodVertices;	// CPU ruzgar: LOD'un kullandigi vertexler

	// Cynex: Compute sonrasi
	std::vector<float>					m_vecTexCoords0Out;		// texture flip uygulanmis
	std::vector<float>					m_vecTexCoords1Out;
	std::vector<float>					m_vecCoordsWind;		// CPU ruzgar
	bool								m_bDeleted;

	SCynexSptIndexed() : m_uiVertexCount(0), m_uiMask(0), m_bDeleted(false) {}
	unsigned int	GetLodCount() const	{ return (unsigned int) m_vecLodStripCount.size(); }
	unsigned int	GetTriangleCount(int iLod) const;
};

struct SCynexSptLeafLod
{
	unsigned int						m_uiCount;
	uint32_t							m_uiMask;
	std::vector<unsigned char>			m_vecMapIndices;
	std::vector<unsigned char>			m_vecClusterIndices;
	std::vector<float>					m_vecCenters;
	std::vector<unsigned long>			m_vecColorsStatic;
	std::vector<unsigned long>			m_vecColorsDynamic;
	std::vector<float>					m_vecNormals;
	std::vector<float>					m_vecBinormals;
	std::vector<float>					m_vecTangents;
	std::vector<float>					m_vecWindWeights;
	std::vector<unsigned char>			m_vecWindIndices;

	// Cynex: Compute sonrasi
	std::vector<const float*>			m_vecTexPointers;		// m_pLeafMapTexCoords
	std::vector<const float*>			m_vecCoordPointers;		// m_pLeafMapCoords
	std::vector<float>					m_vecCorners;			// kume basina 16 float (x,y,z,0)
	std::vector<float>					m_vecCentersWind;

	SCynexSptLeafLod() : m_uiCount(0), m_uiMask(0) {}
};

struct SCynexSptModel
{
	unsigned int						m_uiRefCount;
	bool								m_bLoaded;
	bool								m_bComputed;
	SCynexSptModel*						m_pPrev;
	SCynexSptModel*						m_pNext;

	// Cynex: dosya verisi
	CynexSpt::SHead						m_sHead;
	std::vector<unsigned char>			m_vecFile;
	std::vector<float>					m_vecLeafLodSizes;
	std::string							m_strBranchTexture;
	std::string							m_strComposite;
	std::string							m_strSelfShadow;
	std::string							m_strUserData;
	bool								m_bHasComposite;
	bool								m_bHasSelfShadow;
	bool								m_bHasUserData;
	std::vector<std::string>			m_vecLeafTextures;
	std::vector<std::string>			m_vecFrondTextures;
	std::vector<const char*>			m_vecLeafTexturePointers;
	std::vector<const char*>			m_vecFrondTexturePointers;
	std::vector<CynexSpt::SCollision>	m_vecCollisions;
	SCynexSptIndexed					m_sBranches;
	SCynexSptIndexed					m_sFronds;
	std::vector<CynexSpt::SLeafCluster>	m_vecClusters;
	std::vector<float>					m_vecLeafMapTexCoords;	// harita basina 8 (dosyadaki)
	std::vector<unsigned char>			m_vecLeafMapNoFlip;
	std::vector<SCynexSptLeafLod>		m_vecLeafLods;
	CynexSpt::SBillboard				m_sBillboard;
	std::vector<float>					m_vec360TexCoords;

	// Cynex: Compute oncesi ayarlar (Compute'tan sonra degistirilemez)
	CSpeedTreeRT::ELightingMethod		m_eBranchLighting;
	CSpeedTreeRT::ELightingMethod		m_eLeafLighting;
	CSpeedTreeRT::ELightingMethod		m_eFrondLighting;
	CSpeedTreeRT::EStaticLightingStyle	m_eStaticStyle;
	CSpeedTreeRT::EWindMethod			m_eBranchWind;
	CSpeedTreeRT::EWindMethod			m_eLeafWind;
	CSpeedTreeRT::EWindMethod			m_eFrondWind;
	float								m_fLeafLightingAdjustment;
	float								m_afBranchMaterial[13];
	float								m_afLeafMaterial[13];
	float								m_afFrondMaterial[13];
	unsigned int						m_uiLocalStart;
	unsigned int						m_uiLocalSpan;
	unsigned int						m_uiRockingGroups;
	float								m_fTreeSize;
	float								m_fTreeSizeVariance;
	float								m_fGeometryScale;
	float								m_fTargetAlpha;

	// Cynex: orneklerin paylastigi durum
	float								m_fLodNear;
	float								m_fLodFar;
	float								m_fWindStrength;		// GetWindStrength
	float								m_fRockStrength;		// sallanmayi suren guc
	float								m_fRockOffset;			// faz kaydirma (SetWindStrength donusu)
	bool								m_bRocking;
	bool								m_bLeavesDirty;
	bool								m_bTextureFlip;			// Compute anindaki deger

	// Cynex: Compute sonrasi tamponlar
	std::vector<float>					m_vecLeafMapTexCoordsOut;
	std::vector<float>					m_vecBillboardTable;	// GetLeafBillboardTable
	float								m_afBillboardTexCoords[8];
	float								m_afHorizontalTexCoords[8];
	std::vector<float>					m_vec360TexCoordsOut;
	float								m_afBillboardCoords0[12];
	float								m_afBillboardCoords1[12];

	SCynexSptModel();
	~SCynexSptModel();

	bool	Load(const unsigned char* pBlock, unsigned int uiSize, std::string& strError);
	void	ResetSettingsFromFile();
	bool	Compute(const float* pTransform);
	void	UpdateLeafCorners();
	bool	HasBillboard() const	{ return (m_sHead.uiFlags & CynexSpt::HEAD_HAS_BILLBOARD) != 0; }

	// Cynex: SetTime / SetCamera bildirimleri icin global liste
	static SCynexSptModel*				ms_pFirst;
	void	Link();
	void	Unlink();
};

///////////////////////////////////////////////////////////////////////
//	global durum (CSpeedTreeRT static'leri)

struct SCynexSptGlobals
{
	enum { MAX_LIGHTS = 8, MAX_WIND_MATRICES = 32 };

	float			m_afCameraPos[3];
	float			m_afCameraDir[3];
	float			m_fCameraAzimuth;
	float			m_fCameraPitch;
	float			m_fTime;
	bool			m_bTextureFlip;
	bool			m_bDropToBillboard;
	bool			m_abLightState[MAX_LIGHTS];
	float			m_aafLights[MAX_LIGHTS][16];
	unsigned int	m_uiNumWindMatrices;
	float			m_aafWindMatrices[MAX_WIND_MATRICES][16];
	std::string		m_strError;

	SCynexSptGlobals();
};

SCynexSptGlobals& CynexSptGlobals();
void CynexSptSetError(const char* pError);
