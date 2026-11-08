///////////////////////////////////////////////////////////////////////
//	SpeedTreeRT.cpp
//
//	Cynex: CSpeedTreeRT 1.6 arayuzunun temiz-oda uygulamasi. Geometri sptconv
//	Cynex: dosyasindan gelir; kamera, zaman, ruzgar ve LOD'a bagli sonuclar
//	Cynex: burada hesaplanir (gercek dll ile karsilastirmali dogrulandi).

#include <SpeedTreeRT.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "CynexSptModel.h"

namespace
{
	inline int ClampInt(int v, int lo, int hi)
	{
		return v < lo ? lo : (v > hi ? hi : v);
	}

	inline float Clamp01(float v)
	{
		return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
	}

	// Cynex: dll'in mesafe icin kullandigi kaba karekok (us yarilama, mantis dogrusal)
	inline float ApproxSqrt(float x)
	{
		uint32_t i;
		memcpy(&i, &x, 4);
		i = (i + 0x3F800000u) >> 1;
		float r;
		memcpy(&r, &i, 4);
		return r;
	}

	struct STransition
	{
		bool	m_bActive;
		int		m_iHigh;
		int		m_iLow;
		float	m_fHigh;
		float	m_fLow;
	};

	// Cynex: LOD bantlari arasindaki gecis; bant sinirinin +-yaricapinda iki seviye birlikte.
	// Cynex: dll ters LOD'u (1 - f) float'a yazip gerisini x87 ile double hassasiyette hesapliyor; sinirlar buna gore
	void GetTransition(const SCynexSptModel& m, float fLod, int iCount, STransition& t)
	{
		const float fInv = 1.0f - Clamp01(fLod);
		const double u = fInv;
		const double dTarget = m.m_fTargetAlpha;
		t.m_bActive = false;
		t.m_iLow = -1;
		t.m_fHigh = m.m_fTargetAlpha;
		t.m_fLow = -1.0f;
		if (iCount <= 1)
		{
			t.m_iHigh = 0;
			return;
		}
		const double r = m.m_sHead.fLodRadius;
		if (r > 0.0)
		{
			// Cynex: en yakin sinir (esitlikte buyuk olan); bitisik bolgelerde dll boyle seciyor
			const int k = (int) floor(u * iCount + 0.5);
			if (k >= 1 && k < iCount)
			{
				const double ub = (double) k / (double) iCount;
				if (fabs(u - ub) <= r)
				{
					const double x = (ub + r - u) / (2.0 * r);
					const double tr = m.m_sHead.fLodTransition;
					const double e = m.m_sHead.fLodExponent;
					double uh = tr < 1.0 ? 1.0 - x / (1.0 - tr) : 0.0;
					double ul = tr < 1.0 ? (x - tr) / (1.0 - tr) : 1.0;
					uh = uh < 0.0 ? 0.0 : (uh > 1.0 ? 1.0 : uh);
					ul = ul < 0.0 ? 0.0 : (ul > 1.0 ? 1.0 : ul);
					t.m_bActive = true;
					t.m_iHigh = k - 1;
					t.m_iLow = k;
					t.m_fHigh = (float) (dTarget + (255.0 - dTarget) * pow(uh, e));
					t.m_fLow = (float) (dTarget + (255.0 - dTarget) * pow(ul, e));
					return;
				}
			}
		}
		t.m_iHigh = ClampInt((int) floor(u * iCount), 0, iCount - 1);
	}

	struct SLodState
	{
		float	m_fFadeAlpha;		// dal/frond alfasi (billboard'a gecis)
		bool	m_bBillboard;
		float	m_fBillboardAlpha;
	};

	void GetBillboardFade(const SCynexSptModel& m, float fLod, SLodState& s)
	{
		const int nL = m.m_sHead.usLeafLods;
		s.m_fFadeAlpha = m.m_fTargetAlpha;
		s.m_bBillboard = false;
		s.m_fBillboardAlpha = -1.0f;
		if (!CynexSptGlobals().m_bDropToBillboard || !m.HasBillboard())
			return;
		STransition t;
		GetTransition(m, fLod, nL + 1, t);
		if (t.m_bActive && t.m_iLow == nL)
		{
			s.m_fFadeAlpha = t.m_fHigh;
			s.m_bBillboard = true;
			s.m_fBillboardAlpha = t.m_fLow;
		}
		else if (!t.m_bActive && t.m_iHigh == nL)
		{
			s.m_fFadeAlpha = 255.0f;
			s.m_bBillboard = true;
			s.m_fBillboardAlpha = m.m_fTargetAlpha;
		}
	}

	bool DropActive(const SCynexSptModel& m)
	{
		return CynexSptGlobals().m_bDropToBillboard && m.HasBillboard();
	}

	int DiscreteBranch(const SCynexSptModel& m, float fLod)
	{
		const int n = m.m_sHead.usBranchLods;
		if (n <= 0)
			return -1;
		const double dInv = 1.0 - (double) Clamp01(fLod);
		return ClampInt((int) floor(dInv * n), 0, n - 1);
	}

	int DiscreteFrond(const SCynexSptModel& m, float fLod)
	{
		const int n = m.m_sHead.usFrondLods;
		if (n <= 0)
			return -1;
		const int iCount = n + (DropActive(m) ? 1 : 0);
		// Cynex: ayrik seviye hesabinda (1 - f) float'a yazilmiyor (bant sinirinda gecis kodundan farkli)
		const double dInv = 1.0 - (double) Clamp01(fLod);
		const int l = ClampInt((int) floor(dInv * iCount), 0, iCount - 1);
		return l >= n ? -1 : l;
	}

	int DiscreteLeaf(const SCynexSptModel& m, float fLod)
	{
		const int n = m.m_sHead.usLeafLods;
		if (n <= 0)
			return 0;
		const int iCount = n + (DropActive(m) ? 1 : 0);
		const double dInv = 1.0 - (double) Clamp01(fLod);
		return ClampInt((int) floor(dInv * iCount), 0, iCount - 1);
	}

	void FillIndexed(const SCynexSptIndexed& s, int iLodForStrips, int iDiscrete,
		CSpeedTreeRT::ELightingMethod eLight, CSpeedTreeRT::EWindMethod eWind, bool bFronds, CSpeedTreeRT::SGeometry::SIndexed& out)
	{
		out.m_nDiscreteLodLevel = iDiscrete;
		if (iLodForStrips >= 0 && iLodForStrips < (int) s.m_vecLodStripCount.size() && !s.m_bDeleted)
		{
			const unsigned int start = s.m_vecLodStripStart[iLodForStrips];
			out.m_usNumStrips = s.m_vecLodStripCount[iLodForStrips];
			out.m_pStripLengths = out.m_usNumStrips ? &s.m_vecStripLengths[start] : NULL;
			out.m_pStrips = out.m_usNumStrips ? (const unsigned short**) &s.m_vecStripPointers[start] : NULL;
		}
		else
		{
			out.m_usNumStrips = 0;
			out.m_pStripLengths = NULL;
			out.m_pStrips = NULL;
		}

		const unsigned int n = s.m_bDeleted ? 0 : s.m_uiVertexCount;
		out.m_usVertexCount = (unsigned short) n;
		if (n == 0)
		{
			out.m_pColors = NULL;
			out.m_pNormals = out.m_pBinormals = out.m_pTangents = out.m_pCoords = NULL;
			out.m_pTexCoords0 = out.m_pTexCoords1 = out.m_pWindWeights = NULL;
			out.m_pWindMatrixIndices = NULL;
			return;
		}
		const bool bStatic = eLight == CSpeedTreeRT::LIGHT_STATIC;
		const bool bWind = eWind != CSpeedTreeRT::WIND_NONE;
		// Cynex: dll kurali: renk sadece statikte, normal sadece dinamikte; dal binormal/tangent her zaman
		out.m_pColors = (bStatic && !s.m_vecColors.empty()) ? &s.m_vecColors[0] : NULL;
		out.m_pNormals = (!bStatic && !s.m_vecNormals.empty()) ? &s.m_vecNormals[0] : NULL;
		const bool bBinTan = !bStatic || (!bFronds && (s.m_uiMask & CynexSpt::ARRAY_BINTAN_STATIC));
		out.m_pBinormals = (bBinTan && !s.m_vecBinormals.empty()) ? &s.m_vecBinormals[0] : NULL;
		out.m_pTangents = (bBinTan && !s.m_vecTangents.empty()) ? &s.m_vecTangents[0] : NULL;
		if (eWind == CSpeedTreeRT::WIND_CPU && !s.m_vecCoordsWind.empty())
			out.m_pCoords = &s.m_vecCoordsWind[0];
		else
			out.m_pCoords = s.m_vecCoords.empty() ? NULL : &s.m_vecCoords[0];
		out.m_pTexCoords0 = s.m_vecTexCoords0Out.empty() ? NULL : &s.m_vecTexCoords0Out[0];
		out.m_pTexCoords1 = s.m_vecTexCoords1Out.empty() ? NULL : &s.m_vecTexCoords1Out[0];
		out.m_pWindWeights = (bWind && !s.m_vecWindWeights.empty()) ? &s.m_vecWindWeights[0] : NULL;
		out.m_pWindMatrixIndices = (bWind && !s.m_vecWindIndices.empty()) ? &s.m_vecWindIndices[0] : NULL;
	}

	void FillLeaf(const SCynexSptModel& m, int iLod, float fAlpha, CSpeedTreeRT::SGeometry::SLeaf& out)
	{
		out.m_bIsActive = true;
		out.m_fAlphaTestValue = fAlpha;
		out.m_nDiscreteLodLevel = iLod;
		if (iLod < 0 || iLod >= (int) m.m_vecLeafLods.size())
		{
			out.m_usLeafCount = 0;
			return;
		}
		const SCynexSptLeafLod& lod = m.m_vecLeafLods[iLod];
		const unsigned int n = lod.m_uiCount;
		out.m_usLeafCount = (unsigned short) n;
		if (n == 0)
		{
			out.m_pLeafMapIndices = out.m_pLeafClusterIndices = NULL;
			out.m_pCenterCoords = NULL;
			out.m_pLeafMapTexCoords = out.m_pLeafMapCoords = NULL;
			out.m_pColors = NULL;
			out.m_pNormals = out.m_pBinormals = out.m_pTangents = out.m_pWindWeights = NULL;
			out.m_pWindMatrixIndices = NULL;
			return;
		}
		const bool bStatic = m.m_eLeafLighting == CSpeedTreeRT::LIGHT_STATIC;
		const bool bWind = m.m_eLeafWind != CSpeedTreeRT::WIND_NONE;
		out.m_pLeafMapIndices = &lod.m_vecMapIndices[0];
		out.m_pLeafClusterIndices = &lod.m_vecClusterIndices[0];
		out.m_pCenterCoords = (m.m_eLeafWind == CSpeedTreeRT::WIND_CPU && !lod.m_vecCentersWind.empty()) ? &lod.m_vecCentersWind[0] : &lod.m_vecCenters[0];
		out.m_pLeafMapTexCoords = (const float**) &lod.m_vecTexPointers[0];
		out.m_pLeafMapCoords = (const float**) &lod.m_vecCoordPointers[0];
		// Cynex: yaprak rengi iki modda da var (dinamikte farkli degerler)
		const std::vector<unsigned long>& vecColors = bStatic ? lod.m_vecColorsStatic : lod.m_vecColorsDynamic;
		out.m_pColors = vecColors.empty() ? NULL : &vecColors[0];
		out.m_pNormals = (!bStatic && !lod.m_vecNormals.empty()) ? &lod.m_vecNormals[0] : NULL;
		out.m_pBinormals = (!bStatic && !lod.m_vecBinormals.empty()) ? &lod.m_vecBinormals[0] : NULL;
		out.m_pTangents = (!bStatic && !lod.m_vecTangents.empty()) ? &lod.m_vecTangents[0] : NULL;
		out.m_pWindWeights = (bWind && !lod.m_vecWindWeights.empty()) ? &lod.m_vecWindWeights[0] : NULL;
		out.m_pWindMatrixIndices = (bWind && !lod.m_vecWindIndices.empty()) ? &lod.m_vecWindIndices[0] : NULL;
	}

	// Cynex: dikey billboard: kamera azimutuna donuk, agac koku orijinde
	void BillboardCoords(const SCynexSptModel& m, float* pOut)
	{
		const float az = CynexSptGlobals().m_fCameraAzimuth;
		const float s = sinf(az) * m.m_sBillboard.fHalfWidth;
		const float c = cosf(az) * m.m_sBillboard.fHalfWidth;
		const float top = m.m_sBillboard.fTop, bottom = m.m_sBillboard.fBottom;
		const float af[12] = { -s, c, top, s, -c, top, s, -c, bottom, -s, c, bottom };
		memcpy(pOut, af, sizeof(af));
	}

	// Cynex: CPU ruzgar: p' = p + w * (M p - p), M = yerel matris araligindan; pVertices NULL ise hepsi
	void ApplyCpuWind(const SCynexSptModel& m, const std::vector<float>& vecSrc, const std::vector<float>& vecWeights,
		const std::vector<unsigned char>& vecIndices, std::vector<float>& vecDst, const std::vector<unsigned short>* pVertices)
	{
		const SCynexSptGlobals& g = CynexSptGlobals();
		const size_t n = vecSrc.size() / 3;
		if (vecDst.size() != vecSrc.size() || vecWeights.size() < n || vecIndices.size() < n)
			return;
		const unsigned int uiSpan = m.m_uiLocalSpan ? m.m_uiLocalSpan : 1;
		const size_t uiCount = pVertices ? pVertices->size() : n;
		for (size_t k = 0; k < uiCount; ++k)
		{
			const size_t i = pVertices ? (*pVertices)[k] : k;
			unsigned int idx = m.m_uiLocalStart + (vecIndices[i] % uiSpan);
			if (idx >= SCynexSptGlobals::MAX_WIND_MATRICES)
				idx = SCynexSptGlobals::MAX_WIND_MATRICES - 1;
			const float* M = g.m_aafWindMatrices[idx];
			const float* p = &vecSrc[i * 3];
			const float w = vecWeights[i];
			const float x = p[0] * M[0] + p[1] * M[4] + p[2] * M[8] + M[12];
			const float y = p[0] * M[1] + p[1] * M[5] + p[2] * M[9] + M[13];
			const float z = p[0] * M[2] + p[1] * M[6] + p[2] * M[10] + M[14];
			float* q = &vecDst[i * 3];
			q[0] = p[0] + w * (x - p[0]);
			q[1] = p[1] + w * (y - p[1]);
			q[2] = p[2] + w * (z - p[2]);
		}
	}
}

///////////////////////////////////////////////////////////////////////
//	SGeometry

CSpeedTreeRT::SGeometry::SIndexed::SIndexed() :
	m_nDiscreteLodLevel(-1),
	m_usNumStrips(0),
	m_pStripLengths(NULL),
	m_pStrips(NULL),
	m_usVertexCount(0),
	m_pColors(NULL),
	m_pNormals(NULL),
	m_pBinormals(NULL),
	m_pTangents(NULL),
	m_pCoords(NULL),
	m_pTexCoords0(NULL),
	m_pTexCoords1(NULL),
	m_pWindWeights(NULL),
	m_pWindMatrixIndices(NULL)
{
}

CSpeedTreeRT::SGeometry::SIndexed::~SIndexed()
{
}

CSpeedTreeRT::SGeometry::SLeaf::SLeaf() :
	m_bIsActive(false),
	m_fAlphaTestValue(-1.0f),
	m_nDiscreteLodLevel(-1),
	m_usLeafCount(0),
	m_pLeafMapIndices(NULL),
	m_pLeafClusterIndices(NULL),
	m_pCenterCoords(NULL),
	m_pLeafMapTexCoords(NULL),
	m_pLeafMapCoords(NULL),
	m_pColors(NULL),
	m_pNormals(NULL),
	m_pBinormals(NULL),
	m_pTangents(NULL),
	m_pWindWeights(NULL),
	m_pWindMatrixIndices(NULL)
{
}

CSpeedTreeRT::SGeometry::SLeaf::~SLeaf()
{
}

CSpeedTreeRT::SGeometry::SBillboard::SBillboard() :
	m_bIsActive(false),
	m_pTexCoords(NULL),
	m_pCoords(NULL),
	m_fAlphaTestValue(-1.0f)
{
}

CSpeedTreeRT::SGeometry::SBillboard::~SBillboard()
{
}

CSpeedTreeRT::SGeometry::SGeometry() :
	m_fBranchAlphaTestValue(-1.0f),
	m_fFrondAlphaTestValue(-1.0f)
{
}

CSpeedTreeRT::SGeometry::~SGeometry()
{
}

CSpeedTreeRT::STextures::STextures() :
	m_pBranchTextureFilename(NULL),
	m_uiLeafTextureCount(0),
	m_pLeafTextureFilenames(NULL),
	m_uiFrondTextureCount(0),
	m_pFrondTextureFilenames(NULL),
	m_pCompositeFilename(NULL),
	m_pSelfShadowFilename(NULL)
{
}

CSpeedTreeRT::STextures::~STextures()
{
}

///////////////////////////////////////////////////////////////////////
//	olusturma / bellek

CSpeedTreeRT::CSpeedTreeRT() :
	m_pModel(new SCynexSptModel),
	m_pInstanceOf(NULL),
	m_fLodLevel(1.0f)
{
	m_afPosition[0] = m_afPosition[1] = m_afPosition[2] = 0.0f;
}

CSpeedTreeRT::~CSpeedTreeRT()
{
	if (m_pModel && --m_pModel->m_uiRefCount == 0)
		delete m_pModel;
	m_pModel = NULL;
}

void* CSpeedTreeRT::operator new(size_t nSize)
{
	return ::operator new(nSize);
}

void* CSpeedTreeRT::operator new[](size_t nSize)
{
	return ::operator new[](nSize);
}

void CSpeedTreeRT::operator delete(void* pRawMemory)
{
	::operator delete(pRawMemory);
}

void CSpeedTreeRT::operator delete[](void* pRawMemory)
{
	::operator delete[](pRawMemory);
}

///////////////////////////////////////////////////////////////////////
//	model

bool CSpeedTreeRT::Compute(const float* pTransform, unsigned int nSeed, bool bCompositeStrips)
{
	// Cynex: geometri tohum 1 ve birlesik strip ile donusturuldu; diger degerler yok sayilir
	(void) nSeed;
	(void) bCompositeStrips;
	if (!m_pModel->m_bLoaded)
	{
		CynexSptSetError("CSpeedTreeRT::Compute - no tree loaded");
		return false;
	}
	if (m_pModel->m_bComputed || m_pInstanceOf)
	{
		CynexSptSetError("CSpeedTreeRT::Compute - tree already computed");
		return false;
	}
	return m_pModel->Compute(pTransform);
}

CSpeedTreeRT* CSpeedTreeRT::Clone(float x, float y, float z, unsigned int nSeed) const
{
	(void) nSeed;
	if (!m_pModel->m_bLoaded)
	{
		CynexSptSetError("CSpeedTreeRT::Clone - no tree loaded");
		return NULL;
	}
	CSpeedTreeRT* pClone = new CSpeedTreeRT;
	std::string strError;
	SCynexSptModel* pSrc = m_pModel;
	SCynexSptModel* pDst = pClone->m_pModel;
	if (!pDst->Load(pSrc->m_vecFile.empty() ? NULL : &pSrc->m_vecFile[0], (unsigned int) pSrc->m_vecFile.size(), strError))
	{
		CynexSptSetError(strError.c_str());
		delete pClone;
		return NULL;
	}
	pDst->m_eBranchLighting = pSrc->m_eBranchLighting;
	pDst->m_eLeafLighting = pSrc->m_eLeafLighting;
	pDst->m_eFrondLighting = pSrc->m_eFrondLighting;
	pDst->m_eStaticStyle = pSrc->m_eStaticStyle;
	pDst->m_eBranchWind = pSrc->m_eBranchWind;
	pDst->m_eLeafWind = pSrc->m_eLeafWind;
	pDst->m_eFrondWind = pSrc->m_eFrondWind;
	pDst->m_fLeafLightingAdjustment = pSrc->m_fLeafLightingAdjustment;
	memcpy(pDst->m_afBranchMaterial, pSrc->m_afBranchMaterial, sizeof(pDst->m_afBranchMaterial));
	memcpy(pDst->m_afLeafMaterial, pSrc->m_afLeafMaterial, sizeof(pDst->m_afLeafMaterial));
	memcpy(pDst->m_afFrondMaterial, pSrc->m_afFrondMaterial, sizeof(pDst->m_afFrondMaterial));
	pDst->m_uiLocalStart = pSrc->m_uiLocalStart;
	pDst->m_uiLocalSpan = pSrc->m_uiLocalSpan;
	pDst->m_uiRockingGroups = pSrc->m_uiRockingGroups;
	pDst->m_fTreeSize = pSrc->m_fTreeSize;
	pDst->m_fTreeSizeVariance = pSrc->m_fTreeSizeVariance;
	pDst->m_fTargetAlpha = pSrc->m_fTargetAlpha;
	pDst->m_fLodNear = pSrc->m_fLodNear;
	pDst->m_fLodFar = pSrc->m_fLodFar;
	pDst->m_fWindStrength = pSrc->m_fWindStrength;
	pDst->m_fRockStrength = pSrc->m_fRockStrength;
	pDst->m_fRockOffset = pSrc->m_fRockOffset;
	pDst->m_bRocking = pSrc->m_bRocking;
	if (pSrc->m_bComputed)
		pDst->Compute(NULL);
	pClone->SetTreePosition(x, y, z);
	return pClone;
}

const CSpeedTreeRT* CSpeedTreeRT::InstanceOf(void) const
{
	return m_pInstanceOf;
}

CSpeedTreeRT* CSpeedTreeRT::MakeInstance(void)
{
	if (!m_pModel->m_bComputed)
	{
		CynexSptSetError("CSpeedTreeRT::MakeInstance - tree must be computed first");
		return NULL;
	}
	CSpeedTreeRT* pInstance = new CSpeedTreeRT;
	if (--pInstance->m_pModel->m_uiRefCount == 0)
		delete pInstance->m_pModel;
	pInstance->m_pModel = m_pModel;
	++m_pModel->m_uiRefCount;
	pInstance->m_pInstanceOf = this;
	pInstance->m_fLodLevel = m_fLodLevel;
	return pInstance;
}

void CSpeedTreeRT::DeleteTransientData(void)
{
	// Cynex: Compute icin tutulan ek veri yok; dosya kopyasi Clone/SaveTree icin kalir
}

bool CSpeedTreeRT::LoadTree(const char* pFilename)
{
	if (!pFilename)
	{
		CynexSptSetError("CSpeedTreeRT::LoadTree - null filename");
		return false;
	}
	FILE* fp = fopen(pFilename, "rb");
	if (!fp)
	{
		char szError[512];
		snprintf(szError, sizeof(szError), "CSpeedTreeRT::LoadTree - failed to load file '%s'", pFilename);
		CynexSptSetError(szError);
		return false;
	}
	fseek(fp, 0, SEEK_END);
	const long lSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	std::vector<unsigned char> vecData(lSize > 0 ? (size_t) lSize : 0);
	const size_t got = lSize > 0 ? fread(&vecData[0], 1, (size_t) lSize, fp) : 0;
	fclose(fp);
	if (lSize <= 0 || got != (size_t) lSize)
	{
		CynexSptSetError("CSpeedTreeRT::LoadTree - read error");
		return false;
	}
	return LoadTree(&vecData[0], (unsigned int) vecData.size());
}

bool CSpeedTreeRT::LoadTree(const unsigned char* pBlock, unsigned int nNumBytes)
{
	if (m_pModel->m_bComputed || m_pInstanceOf)
	{
		CynexSptSetError("CSpeedTreeRT::LoadTree - tree already computed");
		return false;
	}
	// Cynex: ayni nesneye ikinci yukleme icin modeli tazele, Compute oncesi ayarlari koru
	SCynexSptModel* pFresh = new SCynexSptModel;
	pFresh->m_fWindStrength = m_pModel->m_fWindStrength;
	pFresh->m_uiLocalStart = m_pModel->m_uiLocalStart;
	pFresh->m_uiLocalSpan = m_pModel->m_uiLocalSpan;
	pFresh->m_uiRockingGroups = m_pModel->m_uiRockingGroups;
	pFresh->m_fTargetAlpha = m_pModel->m_fTargetAlpha;
	std::string strError;
	if (!pFresh->Load(pBlock, nNumBytes, strError))
	{
		delete pFresh;
		CynexSptSetError(("CSpeedTreeRT::LoadTree - " + strError).c_str());
		return false;
	}
	if (--m_pModel->m_uiRefCount == 0)
		delete m_pModel;
	m_pModel = pFresh;
	// Cynex: dll LoadTree'de global ruzgar matrislerini birime dondurur
	SCynexSptGlobals& g = CynexSptGlobals();
	for (int i = 0; i < SCynexSptGlobals::MAX_WIND_MATRICES; ++i)
	{
		memset(g.m_aafWindMatrices[i], 0, sizeof(g.m_aafWindMatrices[i]));
		g.m_aafWindMatrices[i][0] = g.m_aafWindMatrices[i][5] = g.m_aafWindMatrices[i][10] = g.m_aafWindMatrices[i][15] = 1.0f;
	}
	return true;
}

unsigned char* CSpeedTreeRT::SaveTree(unsigned int& nNumBytes, bool bSaveLeaves) const
{
	// Cynex: donusturulmus dosyanin kopyasi (LoadTree tekrar okuyabilir)
	(void) bSaveLeaves;
	nNumBytes = (unsigned int) m_pModel->m_vecFile.size();
	if (!nNumBytes)
		return NULL;
	unsigned char* pData = new unsigned char[nNumBytes];
	memcpy(pData, &m_pModel->m_vecFile[0], nNumBytes);
	return pData;
}

void CSpeedTreeRT::GetTreeSize(float& fSize, float& fVariance) const
{
	fSize = m_pModel->m_fTreeSize;
	fVariance = m_pModel->m_fTreeSizeVariance;
}

void CSpeedTreeRT::SetTreeSize(float fNewSize, float fNewVariance)
{
	m_pModel->m_fTreeSize = fNewSize;
	m_pModel->m_fTreeSizeVariance = fNewVariance;
}

unsigned int CSpeedTreeRT::GetSeed() const
{
	return m_pModel->m_sHead.uiSeed;
}

const float* CSpeedTreeRT::GetTreePosition(void) const
{
	return m_afPosition;
}

void CSpeedTreeRT::SetTreePosition(float x, float y, float z)
{
	m_afPosition[0] = x;
	m_afPosition[1] = y;
	m_afPosition[2] = z;
}

void CSpeedTreeRT::SetLeafTargetAlphaMask(unsigned char ucMask)
{
	// Cynex: dll'de Compute sonrasi etkisiz
	if (m_pModel->m_bComputed)
		return;
	m_pModel->m_fTargetAlpha = (float) ucMask;
}

///////////////////////////////////////////////////////////////////////
//	isik

#define CYNEX_SPT_PRECOMPUTE_SETTER(name) \
	if (m_pModel->m_bComputed) \
	{ \
		CynexSptSetError(name "() has no effect after Compute() has been called"); \
		return; \
	}

CSpeedTreeRT::ELightingMethod CSpeedTreeRT::GetBranchLightingMethod(void) const
{
	return m_pModel->m_eBranchLighting;
}

void CSpeedTreeRT::SetBranchLightingMethod(ELightingMethod eMethod)
{
	CYNEX_SPT_PRECOMPUTE_SETTER("SetBranchLightingMethod");
	m_pModel->m_eBranchLighting = eMethod;
}

CSpeedTreeRT::ELightingMethod CSpeedTreeRT::GetLeafLightingMethod(void) const
{
	return m_pModel->m_eLeafLighting;
}

void CSpeedTreeRT::SetLeafLightingMethod(ELightingMethod eMethod)
{
	CYNEX_SPT_PRECOMPUTE_SETTER("SetLeafLightingMethod");
	m_pModel->m_eLeafLighting = eMethod;
}

CSpeedTreeRT::ELightingMethod CSpeedTreeRT::GetFrondLightingMethod(void) const
{
	return m_pModel->m_eFrondLighting;
}

void CSpeedTreeRT::SetFrondLightingMethod(ELightingMethod eMethod)
{
	CYNEX_SPT_PRECOMPUTE_SETTER("SetFrondLightingMethod");
	m_pModel->m_eFrondLighting = eMethod;
}

CSpeedTreeRT::EStaticLightingStyle CSpeedTreeRT::GetStaticLightingStyle(void) const
{
	return m_pModel->m_eStaticStyle;
}

void CSpeedTreeRT::SetStaticLightingStyle(EStaticLightingStyle eStyle)
{
	CYNEX_SPT_PRECOMPUTE_SETTER("SetStaticLightingStyle");
	m_pModel->m_eStaticStyle = eStyle;
}

float CSpeedTreeRT::GetLeafLightingAdjustment() const
{
	return m_pModel->m_fLeafLightingAdjustment;
}

void CSpeedTreeRT::SetLeafLightingAdjustment(float fScalar)
{
	CYNEX_SPT_PRECOMPUTE_SETTER("SetLeafLightingAdjustment");
	m_pModel->m_fLeafLightingAdjustment = fScalar;
}

bool CSpeedTreeRT::GetLightState(unsigned int nLightIndex)
{
	return nLightIndex < SCynexSptGlobals::MAX_LIGHTS ? CynexSptGlobals().m_abLightState[nLightIndex] : false;
}

void CSpeedTreeRT::SetLightState(unsigned int nLightIndex, bool bLightOn)
{
	if (nLightIndex < SCynexSptGlobals::MAX_LIGHTS)
		CynexSptGlobals().m_abLightState[nLightIndex] = bLightOn;
}

const float* CSpeedTreeRT::GetLightAttributes(unsigned int nLightIndex)
{
	return nLightIndex < SCynexSptGlobals::MAX_LIGHTS ? CynexSptGlobals().m_aafLights[nLightIndex] : NULL;
}

void CSpeedTreeRT::SetLightAttributes(unsigned int nLightIndex, const float* pLightAttributes)
{
	// Cynex: statik renkler donusumde oyunun isigiyla hesaplandi; burada sadece saklanir
	if (nLightIndex < SCynexSptGlobals::MAX_LIGHTS && pLightAttributes)
		memcpy(CynexSptGlobals().m_aafLights[nLightIndex], pLightAttributes, 16 * sizeof(float));
}

const float* CSpeedTreeRT::GetBranchMaterial(void) const
{
	return m_pModel->m_afBranchMaterial;
}

void CSpeedTreeRT::SetBranchMaterial(const float* pMaterial)
{
	CYNEX_SPT_PRECOMPUTE_SETTER("SetBranchMaterial");
	if (pMaterial)
		memcpy(m_pModel->m_afBranchMaterial, pMaterial, 13 * sizeof(float));
}

const float* CSpeedTreeRT::GetLeafMaterial(void) const
{
	return m_pModel->m_afLeafMaterial;
}

void CSpeedTreeRT::SetLeafMaterial(const float* pMaterial)
{
	CYNEX_SPT_PRECOMPUTE_SETTER("SetLeafMaterial");
	if (pMaterial)
		memcpy(m_pModel->m_afLeafMaterial, pMaterial, 13 * sizeof(float));
}

const float* CSpeedTreeRT::GetFrondMaterial(void) const
{
	return m_pModel->m_afFrondMaterial;
}

void CSpeedTreeRT::SetFrondMaterial(const float* pMaterial)
{
	CYNEX_SPT_PRECOMPUTE_SETTER("SetFrondMaterial");
	if (pMaterial)
		memcpy(m_pModel->m_afFrondMaterial, pMaterial, 13 * sizeof(float));
}

///////////////////////////////////////////////////////////////////////
//	kamera

void CSpeedTreeRT::GetCamera(float* pPosition, float* pDirection)
{
	const SCynexSptGlobals& g = CynexSptGlobals();
	if (pPosition)
		memcpy(pPosition, g.m_afCameraPos, 3 * sizeof(float));
	if (pDirection)
		memcpy(pDirection, g.m_afCameraDir, 3 * sizeof(float));
}

void CSpeedTreeRT::SetCamera(const float* pPosition, const float* pDirection)
{
	SCynexSptGlobals& g = CynexSptGlobals();
	if (pPosition)
		memcpy(g.m_afCameraPos, pPosition, 3 * sizeof(float));
	if (pDirection)
		memcpy(g.m_afCameraDir, pDirection, 3 * sizeof(float));
	// Cynex: dll ile ayni: azimut atan2(y, x), egim asin(z) (yon normalize edilmez)
	g.m_fCameraAzimuth = atan2f(g.m_afCameraDir[1], g.m_afCameraDir[0]);
	g.m_fCameraPitch = asinf(g.m_afCameraDir[2]);
	for (SCynexSptModel* p = SCynexSptModel::ms_pFirst; p; p = p->m_pNext)
		p->m_bLeavesDirty = true;
}

///////////////////////////////////////////////////////////////////////
//	ruzgar

void CSpeedTreeRT::SetTime(float fTime)
{
	CynexSptGlobals().m_fTime = fTime;
	// Cynex: sallanma kapali agaclar son konumda kalir (dll davranisi)
	for (SCynexSptModel* p = SCynexSptModel::ms_pFirst; p; p = p->m_pNext)
		if (p->m_bRocking)
			p->m_bLeavesDirty = true;
}

void CSpeedTreeRT::ComputeWindEffects(bool bBranches, bool bLeaves, bool bFronds)
{
	SCynexSptModel& m = *m_pModel;
	if (!m.m_bComputed)
		return;
	if (bBranches && m.m_eBranchWind == WIND_CPU && !m.m_sBranches.m_bDeleted)
	{
		const int iLod = DiscreteBranch(m, m_fLodLevel);
		if (iLod >= 0 && iLod < (int) m.m_sBranches.m_vecLodVertices.size())
			ApplyCpuWind(m, m.m_sBranches.m_vecCoords, m.m_sBranches.m_vecWindWeights, m.m_sBranches.m_vecWindIndices, m.m_sBranches.m_vecCoordsWind, &m.m_sBranches.m_vecLodVertices[iLod]);
	}
	if (bFronds && m.m_eFrondWind == WIND_CPU && !m.m_sFronds.m_bDeleted)
	{
		const int iLod = DiscreteFrond(m, m_fLodLevel);
		if (iLod >= 0 && iLod < (int) m.m_sFronds.m_vecLodVertices.size())
			ApplyCpuWind(m, m.m_sFronds.m_vecCoords, m.m_sFronds.m_vecWindWeights, m.m_sFronds.m_vecWindIndices, m.m_sFronds.m_vecCoordsWind, &m.m_sFronds.m_vecLodVertices[iLod]);
	}
	if (bLeaves && m.m_eLeafWind == WIND_CPU)
	{
		// Cynex: gorunen yaprak seviyeleri (gecisteyse iki seviye)
		const int nL = m.m_sHead.usLeafLods;
		STransition t;
		GetTransition(m, m_fLodLevel, nL + (DropActive(m) ? 1 : 0), t);
		const int aiLods[2] = { t.m_iHigh, t.m_bActive ? t.m_iLow : -1 };
		for (int k = 0; k < 2; ++k)
		{
			if (aiLods[k] < 0 || aiLods[k] >= (int) m.m_vecLeafLods.size())
				continue;
			SCynexSptLeafLod& lod = m.m_vecLeafLods[aiLods[k]];
			ApplyCpuWind(m, lod.m_vecCenters, lod.m_vecWindWeights, lod.m_vecWindIndices, lod.m_vecCentersWind, NULL);
		}
	}
}

void CSpeedTreeRT::ResetLeafWindState(void)
{
	m_pModel->m_fRockOffset = 0.0f;
	m_pModel->m_bLeavesDirty = true;
}

bool CSpeedTreeRT::GetLeafRockingState(void) const
{
	return m_pModel->m_bRocking;
}

void CSpeedTreeRT::SetLeafRockingState(bool bFlag)
{
	m_pModel->m_bRocking = bFlag;
}

void CSpeedTreeRT::SetNumLeafRockingGroups(unsigned int nRockingGroups)
{
	// Cynex: donusum tek grupla yapildi (oyun ayari); deger saklanir
	m_pModel->m_uiRockingGroups = nRockingGroups;
}

CSpeedTreeRT::EWindMethod CSpeedTreeRT::GetLeafWindMethod(void) const
{
	return m_pModel->m_eLeafWind;
}

void CSpeedTreeRT::SetLeafWindMethod(EWindMethod eMethod)
{
	CYNEX_SPT_PRECOMPUTE_SETTER("SetLeafWindMethod");
	m_pModel->m_eLeafWind = eMethod;
}

CSpeedTreeRT::EWindMethod CSpeedTreeRT::GetBranchWindMethod(void) const
{
	return m_pModel->m_eBranchWind;
}

void CSpeedTreeRT::SetBranchWindMethod(EWindMethod eMethod)
{
	CYNEX_SPT_PRECOMPUTE_SETTER("SetBranchWindMethod");
	m_pModel->m_eBranchWind = eMethod;
}

CSpeedTreeRT::EWindMethod CSpeedTreeRT::GetFrondWindMethod(void) const
{
	return m_pModel->m_eFrondWind;
}

void CSpeedTreeRT::SetFrondWindMethod(EWindMethod eMethod)
{
	CYNEX_SPT_PRECOMPUTE_SETTER("SetFrondWindMethod");
	m_pModel->m_eFrondWind = eMethod;
}

float CSpeedTreeRT::GetWindStrength(void) const
{
	return m_pModel->m_fWindStrength;
}

float CSpeedTreeRT::SetWindStrength(float fNewStrength, float fOldStrength, float fFrequencyTimeOffset)
{
	// Cynex: frekans gucle orantili; faz surekli kalsin diye kaydirma = w(eski) t + eski kaydirma - w(yeni) t
	SCynexSptModel& m = *m_pModel;
	const float fOld = fOldStrength < 0.0f ? m.m_fRockStrength : fOldStrength;
	const float fOffset = fFrequencyTimeOffset == -1.0f ? m.m_fRockOffset : fFrequencyTimeOffset;
	const float fTime = CynexSptGlobals().m_fTime;
	const float fFreq = m.m_sHead.fRockFrequency;
	const float fNewOffset = fFreq * fOld * fTime + fOffset - fFreq * fNewStrength * fTime;
	m.m_fWindStrength = fNewStrength;
	m.m_fRockStrength = fNewStrength;
	m.m_fRockOffset = fNewOffset;
	m.m_bLeavesDirty = true;
	return fNewOffset;
}

void CSpeedTreeRT::SetNumWindMatrices(unsigned int nNumMatrices)
{
	CynexSptGlobals().m_uiNumWindMatrices = nNumMatrices < SCynexSptGlobals::MAX_WIND_MATRICES ? nNumMatrices : SCynexSptGlobals::MAX_WIND_MATRICES;
}

void CSpeedTreeRT::SetWindMatrix(unsigned int nMatrixIndex, const float* pMatrix)
{
	if (nMatrixIndex < SCynexSptGlobals::MAX_WIND_MATRICES && pMatrix)
		memcpy(CynexSptGlobals().m_aafWindMatrices[nMatrixIndex], pMatrix, 16 * sizeof(float));
}

void CSpeedTreeRT::GetLocalMatrices(unsigned int& nStartingIndex, unsigned int& nMatrixSpan)
{
	nStartingIndex = m_pModel->m_uiLocalStart;
	nMatrixSpan = m_pModel->m_uiLocalSpan;
}

void CSpeedTreeRT::SetLocalMatrices(unsigned int nStartingMatrix, unsigned int nMatrixSpan)
{
	CYNEX_SPT_PRECOMPUTE_SETTER("SetLocalMatrices");
	m_pModel->m_uiLocalStart = nStartingMatrix;
	m_pModel->m_uiLocalSpan = nMatrixSpan;
}

///////////////////////////////////////////////////////////////////////
//	LOD

void CSpeedTreeRT::ComputeLodLevel(void)
{
	const SCynexSptGlobals& g = CynexSptGlobals();
	const float dx = g.m_afCameraPos[0] - m_afPosition[0];
	const float dy = g.m_afCameraPos[1] - m_afPosition[1];
	const float dz = g.m_afCameraPos[2] - m_afPosition[2];
	const float fDistance = ApproxSqrt(dx * dx + dy * dy + dz * dz);
	const float fRange = m_pModel->m_fLodFar - m_pModel->m_fLodNear;
	const float fLod = fRange != 0.0f ? 1.0f - (fDistance - m_pModel->m_fLodNear) / fRange : (fDistance <= m_pModel->m_fLodNear ? 1.0f : 0.0f);
	m_fLodLevel = Clamp01(fLod);
}

float CSpeedTreeRT::GetLodLevel(void) const
{
	return m_fLodLevel;
}

void CSpeedTreeRT::SetLodLevel(float fLodLevel)
{
	m_fLodLevel = fLodLevel;
}

void CSpeedTreeRT::SetDropToBillboard(bool bFlag)
{
	CynexSptGlobals().m_bDropToBillboard = bFlag;
}

void CSpeedTreeRT::GetLodLimits(float& fNear, float& fFar) const
{
	fNear = m_pModel->m_fLodNear;
	fFar = m_pModel->m_fLodFar;
}

void CSpeedTreeRT::SetLodLimits(float fNear, float fFar)
{
	m_pModel->m_fLodNear = fNear;
	m_pModel->m_fLodFar = fFar;
}

short CSpeedTreeRT::GetDiscreteBranchLodLevel(float fLodLevel) const
{
	return (short) DiscreteBranch(*m_pModel, fLodLevel < 0.0f ? m_fLodLevel : fLodLevel);
}

unsigned short CSpeedTreeRT::GetDiscreteLeafLodLevel(float fLodLevel) const
{
	return (unsigned short) DiscreteLeaf(*m_pModel, fLodLevel < 0.0f ? m_fLodLevel : fLodLevel);
}

short CSpeedTreeRT::GetDiscreteFrondLodLevel(float fLodLevel) const
{
	return (short) DiscreteFrond(*m_pModel, fLodLevel < 0.0f ? m_fLodLevel : fLodLevel);
}

unsigned short CSpeedTreeRT::GetNumBranchLodLevels(void) const
{
	return m_pModel->m_sHead.usBranchLods;
}

unsigned short CSpeedTreeRT::GetNumLeafLodLevels(void) const
{
	return m_pModel->m_sHead.usLeafLods;
}

unsigned short CSpeedTreeRT::GetNumFrondLodLevels(void) const
{
	return m_pModel->m_sHead.usFrondLods;
}

///////////////////////////////////////////////////////////////////////
//	geometri

void CSpeedTreeRT::DeleteBranchGeometry(void)
{
	m_pModel->m_sBranches.m_bDeleted = true;
}

void CSpeedTreeRT::DeleteFrondGeometry(void)
{
	m_pModel->m_sFronds.m_bDeleted = true;
}

unsigned char* CSpeedTreeRT::GetFrondGeometryMapIndexes(int nLodLevel) const
{
	SCynexSptIndexed& s = m_pModel->m_sFronds;
	if (nLodLevel < 0 || nLodLevel >= (int) s.m_vecLodMapIndexes.size() || s.m_vecLodMapIndexes[nLodLevel].empty())
		return NULL;
	return &s.m_vecLodMapIndexes[nLodLevel][0];
}

const float* CSpeedTreeRT::GetLeafBillboardTable(unsigned int& nEntryCount) const
{
	nEntryCount = (unsigned int) m_pModel->m_vecBillboardTable.size();
	return m_pModel->m_vecBillboardTable.empty() ? NULL : &m_pModel->m_vecBillboardTable[0];
}

const float* CSpeedTreeRT::GetLeafLodSizeAdjustments(void)
{
	return m_pModel->m_vecLeafLodSizes.empty() ? NULL : &m_pModel->m_vecLeafLodSizes[0];
}

void CSpeedTreeRT::GetGeometry(SGeometry& sGeometry, unsigned long ulBitVector, short sOverrideBranchLodValue, short sOverrideFrondLodValue, short sOverrideLeafLodValue)
{
	SCynexSptModel& m = *m_pModel;
	if (!m.m_bComputed)
		return;

	const float fLod = m_fLodLevel;
	SLodState kFade;
	GetBillboardFade(m, fLod, kFade);

	if (ulBitVector & SpeedTree_BranchGeometry)
	{
		const int iLod = sOverrideBranchLodValue >= 0 ? sOverrideBranchLodValue : DiscreteBranch(m, fLod);
		const int iDiscrete = kFade.m_fFadeAlpha >= 255.0f ? -1 : iLod;
		FillIndexed(m.m_sBranches, iLod, iDiscrete, m.m_eBranchLighting, m.m_eBranchWind, false, sGeometry.m_sBranches);
		sGeometry.m_fBranchAlphaTestValue = kFade.m_fFadeAlpha;
	}

	if (ulBitVector & SpeedTree_FrondGeometry)
	{
		const int iLod = sOverrideFrondLodValue >= 0 ? sOverrideFrondLodValue : DiscreteFrond(m, fLod);
		const int iDiscrete = kFade.m_fFadeAlpha >= 255.0f ? -1 : iLod;
		FillIndexed(m.m_sFronds, iLod, iDiscrete, m.m_eFrondLighting, m.m_eFrondWind, true, sGeometry.m_sFronds);
		sGeometry.m_fFrondAlphaTestValue = kFade.m_fFadeAlpha;
	}

	if (ulBitVector & SpeedTree_LeafGeometry)
	{
		if (m.m_bLeavesDirty)
			m.UpdateLeafCorners();

		const int nL = m.m_sHead.usLeafLods;
		if (sOverrideLeafLodValue >= 0)
		{
			FillLeaf(m, sOverrideLeafLodValue, m.m_fTargetAlpha, sGeometry.m_sLeaves0);
			sGeometry.m_sLeaves1.m_bIsActive = false;
		}
		else if (m.m_sHead.usLeafLodMethod == LOD_SMOOTH)
		{
			STransition t;
			GetTransition(m, fLod, nL + (DropActive(m) ? 1 : 0), t);
			if (t.m_bActive)
			{
				FillLeaf(m, t.m_iHigh, t.m_fHigh, sGeometry.m_sLeaves0);
				if (t.m_iLow < nL)
					FillLeaf(m, t.m_iLow, t.m_fLow, sGeometry.m_sLeaves1);
				else
					sGeometry.m_sLeaves1.m_bIsActive = false;
			}
			else if (t.m_iHigh >= nL)
			{
				sGeometry.m_sLeaves0.m_bIsActive = false;
				sGeometry.m_sLeaves1.m_bIsActive = false;
			}
			else
			{
				FillLeaf(m, t.m_iHigh, m.m_fTargetAlpha, sGeometry.m_sLeaves0);
				sGeometry.m_sLeaves1.m_bIsActive = false;
			}
		}
		else
		{
			const int iLod = m.m_sHead.usLeafLodMethod == LOD_NONE ? 0 : DiscreteLeaf(m, fLod);
			if (iLod >= nL)
				sGeometry.m_sLeaves0.m_bIsActive = false;
			else
				FillLeaf(m, iLod, m.m_fTargetAlpha, sGeometry.m_sLeaves0);
			sGeometry.m_sLeaves1.m_bIsActive = false;
		}
	}

	if (ulBitVector & SpeedTree_BillboardGeometry)
	{
		SGeometry::SBillboard& b0 = sGeometry.m_sBillboard0;
		SGeometry::SBillboard& b1 = sGeometry.m_sBillboard1;
		if (kFade.m_bBillboard)
		{
			BillboardCoords(m, m.m_afBillboardCoords0);
			b0.m_bIsActive = true;
			b0.m_pCoords = m.m_afBillboardCoords0;
			b0.m_fAlphaTestValue = kFade.m_fBillboardAlpha;
			b0.m_pTexCoords = m.m_afBillboardTexCoords;
			b1.m_bIsActive = false;
			const unsigned int N = m.m_sBillboard.uiNum360;
			if ((m.m_sHead.uiFlags & CynexSpt::HEAD_HAS_360) && N > 0 && m.m_vec360TexCoordsOut.size() >= N * 8)
			{
				// Cynex: 360 derece: sektor j'de BB1 = cikan resim j, BB0 = giren resim j+1; egri LOD ile ayni.
				// Cynex: aci ve LOD gorunurlukleri carpilir: (255 - a) / (255 - T) = vLod * vAci
				const double dStep = 6.283185307179586 / (double) N;
				double dPos = ((double) CynexSptGlobals().m_fCameraAzimuth - m.m_sBillboard.f360Offset) / dStep;
				dPos -= floor(dPos / (double) N) * (double) N;
				const unsigned int j0 = (unsigned int) floor(dPos) % N;
				const unsigned int j1 = (j0 + 1) % N;
				const double x = dPos - floor(dPos);
				const double tr = m.m_sHead.fLodTransition;
				const double e = m.m_sHead.fLodExponent;
				double uh = tr < 1.0 ? 1.0 - x / (1.0 - tr) : 0.0;
				double ul = tr < 1.0 ? (x - tr) / (1.0 - tr) : 1.0;
				uh = uh < 0.0 ? 0.0 : (uh > 1.0 ? 1.0 : uh);
				ul = ul < 0.0 ? 0.0 : (ul > 1.0 ? 1.0 : ul);
				const double dTarget = m.m_fTargetAlpha;
				const double dLodVis = (255.0 - kFade.m_fBillboardAlpha) / (255.0 - dTarget);
				memcpy(m.m_afBillboardCoords1, m.m_afBillboardCoords0, sizeof(m.m_afBillboardCoords1));
				b0.m_pTexCoords = &m.m_vec360TexCoordsOut[j1 * 8];
				b0.m_fAlphaTestValue = (float) (255.0 - dLodVis * (1.0 - pow(uh, e)) * (255.0 - dTarget));
				b1.m_bIsActive = true;
				b1.m_pCoords = m.m_afBillboardCoords1;
				b1.m_pTexCoords = &m.m_vec360TexCoordsOut[j0 * 8];
				b1.m_fAlphaTestValue = (float) (255.0 - dLodVis * (1.0 - pow(ul, e)) * (255.0 - dTarget));
			}
		}
		else
		{
			b0.m_bIsActive = false;
			b1.m_bIsActive = false;
		}
		SGeometry::SBillboard& h = sGeometry.m_sHorizontalBillboard;
		if ((m.m_sHead.uiFlags & CynexSpt::HEAD_HAS_HORIZONTAL) && kFade.m_bBillboard)
		{
			h.m_bIsActive = true;
			h.m_pCoords = m.m_sBillboard.afHorizontalCoords;
			h.m_pTexCoords = m.m_afHorizontalTexCoords;
			h.m_fAlphaTestValue = kFade.m_fBillboardAlpha;
		}
		else
			h.m_bIsActive = false;
	}
}

///////////////////////////////////////////////////////////////////////
//	dokular

void CSpeedTreeRT::GetTextures(STextures& sTextures) const
{
	const SCynexSptModel& m = *m_pModel;
	sTextures.m_pBranchTextureFilename = m.m_strBranchTexture.c_str();
	sTextures.m_uiLeafTextureCount = (unsigned int) m.m_vecLeafTexturePointers.size();
	sTextures.m_pLeafTextureFilenames = m.m_vecLeafTexturePointers.empty() ? NULL : (const char**) &m.m_vecLeafTexturePointers[0];
	sTextures.m_uiFrondTextureCount = (unsigned int) m.m_vecFrondTexturePointers.size();
	sTextures.m_pFrondTextureFilenames = m.m_vecFrondTexturePointers.empty() ? NULL : (const char**) &m.m_vecFrondTexturePointers[0];
	sTextures.m_pCompositeFilename = m.m_bHasComposite ? m.m_strComposite.c_str() : NULL;
	sTextures.m_pSelfShadowFilename = m.m_bHasSelfShadow ? m.m_strSelfShadow.c_str() : NULL;
}

void CSpeedTreeRT::SetLeafTextureCoords(unsigned int nLeafMapIndex, const float* pTexCoords)
{
	// Cynex: her yaprak dokusu iki tablo girisi kullanir: 2k aynen, 2k+1 yatay aynali (u koseleri yer degistirir)
	SCynexSptModel& m = *m_pModel;
	const size_t uiEntries = m.m_vecLeafMapTexCoordsOut.size() / 8;
	if (!pTexCoords || nLeafMapIndex * 2 >= uiEntries)
		return;
	const float fSign = CynexSptGlobals().m_bTextureFlip ? -1.0f : 1.0f;
	static const int c_aiMirror[8] = { 2, 1, 0, 3, 6, 5, 4, 7 };
	float* p = &m.m_vecLeafMapTexCoordsOut[nLeafMapIndex * 16];
	for (int i = 0; i < 8; ++i)
		p[i] = (i & 1) ? fSign * pTexCoords[i] : pTexCoords[i];
	if (nLeafMapIndex * 2 + 1 < uiEntries)
		for (int i = 0; i < 8; ++i)
			p[8 + i] = (i & 1) ? fSign * pTexCoords[c_aiMirror[i]] : pTexCoords[c_aiMirror[i]];
}

void CSpeedTreeRT::SetFrondTextureCoords(unsigned int nFrondMapIndex, const float* pTexCoords)
{
	SCynexSptIndexed& s = m_pModel->m_sFronds;
	if (!pTexCoords || s.m_vecLocalUV.empty() || s.m_vecTexCoords0Out.size() < s.m_uiVertexCount * 2)
		return;
	const bool bFlip = CynexSptGlobals().m_bTextureFlip;
	for (unsigned int i = 0; i < s.m_uiVertexCount; ++i)
	{
		if (s.m_vecFrondMap[i] != nFrondMapIndex)
			continue;
		const float u = s.m_vecLocalUV[i * 2];
		const float v = s.m_vecLocalUV[i * 2 + 1];
		const float fS = pTexCoords[0] + u * (pTexCoords[2] - pTexCoords[0]);
		const float fT = pTexCoords[1] + v * (pTexCoords[5] - pTexCoords[1]);
		s.m_vecTexCoords0Out[i * 2] = fS;
		s.m_vecTexCoords0Out[i * 2 + 1] = bFlip ? -fT : fT;
	}
}

bool CSpeedTreeRT::GetTextureFlip(void)
{
	return CynexSptGlobals().m_bTextureFlip;
}

void CSpeedTreeRT::SetTextureFlip(bool bFlag)
{
	CynexSptGlobals().m_bTextureFlip = bFlag;
}

void CSpeedTreeRT::SetBranchTextureFilename(const char* pFilename)
{
	m_pModel->m_strBranchTexture = pFilename ? pFilename : "";
}

void CSpeedTreeRT::SetLeafTextureFilename(unsigned int nLeafMapIndex, const char* pFilename)
{
	SCynexSptModel& m = *m_pModel;
	if (nLeafMapIndex >= m.m_vecLeafTextures.size())
		return;
	m.m_vecLeafTextures[nLeafMapIndex] = pFilename ? pFilename : "";
	m.m_vecLeafTexturePointers[nLeafMapIndex] = m.m_vecLeafTextures[nLeafMapIndex].c_str();
}

void CSpeedTreeRT::SetFrondTextureFilename(unsigned int nFrondMapIndex, const char* pFilename)
{
	SCynexSptModel& m = *m_pModel;
	if (nFrondMapIndex >= m.m_vecFrondTextures.size())
		return;
	m.m_vecFrondTextures[nFrondMapIndex] = pFilename ? pFilename : "";
	m.m_vecFrondTexturePointers[nFrondMapIndex] = m.m_vecFrondTextures[nFrondMapIndex].c_str();
}

///////////////////////////////////////////////////////////////////////
//	bilgi

void CSpeedTreeRT::Authorize(const char* pKey)
{
	(void) pKey;
}

bool CSpeedTreeRT::IsAuthorized(void)
{
	return true;
}

const char* CSpeedTreeRT::GetCurrentError(void)
{
	return CynexSptGlobals().m_strError.c_str();
}

void CSpeedTreeRT::ResetError(void)
{
	CynexSptGlobals().m_strError.clear();
}

void CSpeedTreeRT::GetBoundingBox(float* pBounds) const
{
	if (pBounds)
		memcpy(pBounds, m_pModel->m_sHead.afBoundingBox, 6 * sizeof(float));
}

unsigned int CSpeedTreeRT::GetLeafTriangleCount(float fLodLevel) const
{
	const int iLod = DiscreteLeaf(*m_pModel, fLodLevel < 0.0f ? m_fLodLevel : fLodLevel);
	if (iLod < 0 || iLod >= (int) m_pModel->m_vecLeafLods.size())
		return 0;
	return m_pModel->m_vecLeafLods[iLod].m_uiCount * 2;
}

unsigned int CSpeedTreeRT::GetBranchTriangleCount(float fLodLevel) const
{
	return m_pModel->m_sBranches.GetTriangleCount(DiscreteBranch(*m_pModel, fLodLevel < 0.0f ? m_fLodLevel : fLodLevel));
}

unsigned int CSpeedTreeRT::GetFrondTriangleCount(float fLodLevel) const
{
	return m_pModel->m_sFronds.GetTriangleCount(DiscreteFrond(*m_pModel, fLodLevel < 0.0f ? m_fLodLevel : fLodLevel));
}

///////////////////////////////////////////////////////////////////////
//	carpisma

unsigned int CSpeedTreeRT::GetCollisionObjectCount(void)
{
	return (unsigned int) m_pModel->m_vecCollisions.size();
}

void CSpeedTreeRT::GetCollisionObject(unsigned int nIndex, ECollisionObjectType& eType, float* pPosition, float* pDimensions)
{
	if (nIndex >= m_pModel->m_vecCollisions.size())
		return;
	const CynexSpt::SCollision& c = m_pModel->m_vecCollisions[nIndex];
	eType = (ECollisionObjectType) c.iType;
	if (pPosition)
		memcpy(pPosition, c.afPosition, 3 * sizeof(float));
	if (pDimensions)
		memcpy(pDimensions, c.afDimensions, 3 * sizeof(float));
}

const char* CSpeedTreeRT::GetUserData(void) const
{
	return m_pModel->m_bHasUserData ? m_pModel->m_strUserData.c_str() : NULL;
}
