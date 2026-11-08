///////////////////////////////////////////////////////////////////////
//	CynexSptModel.cpp
//
//	Cynex: sptconv dosyasinin okunmasi ve Compute anindaki tampon kurulumu.

#include "CynexSptModel.h"

#include <math.h>
#include <string.h>

#include <algorithm>

///////////////////////////////////////////////////////////////////////
//	globals

SCynexSptGlobals::SCynexSptGlobals() :
	m_fCameraAzimuth(0.0f),
	m_fCameraPitch(0.0f),
	m_fTime(0.0f),
	m_bTextureFlip(false),
	m_bDropToBillboard(false),
	m_uiNumWindMatrices(4)
{
	m_afCameraPos[0] = m_afCameraPos[1] = m_afCameraPos[2] = 0.0f;
	m_afCameraDir[0] = 1.0f;
	m_afCameraDir[1] = m_afCameraDir[2] = 0.0f;

	// Cynex: dll varsayilani: konum 0, diffuse/specular 1, ambient 0, yonlu, sabit zayiflama
	static const float c_afDefaultLight[16] = { 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0 };
	for (int i = 0; i < MAX_LIGHTS; ++i)
	{
		m_abLightState[i] = false;
		memcpy(m_aafLights[i], c_afDefaultLight, sizeof(c_afDefaultLight));
	}
	for (int i = 0; i < MAX_WIND_MATRICES; ++i)
	{
		memset(m_aafWindMatrices[i], 0, sizeof(m_aafWindMatrices[i]));
		m_aafWindMatrices[i][0] = m_aafWindMatrices[i][5] = m_aafWindMatrices[i][10] = m_aafWindMatrices[i][15] = 1.0f;
	}
}

SCynexSptGlobals& CynexSptGlobals()
{
	static SCynexSptGlobals s_kGlobals;
	return s_kGlobals;
}

void CynexSptSetError(const char* pError)
{
	CynexSptGlobals().m_strError = pError ? pError : "";
}

///////////////////////////////////////////////////////////////////////
//	okuyucu

namespace
{
	class CReader
	{
		public:
			CReader(const unsigned char* p, size_t n) : m_p(p), m_n(n), m_pos(0), m_bFail(false) {}

			bool	Fail() const		{ return m_bFail; }
			size_t	Pos() const			{ return m_pos; }
			size_t	Left() const		{ return m_n - m_pos; }
			void	Seek(size_t pos)	{ if (pos > m_n) m_bFail = true; else m_pos = pos; }

			bool Raw(void* pOut, size_t n)
			{
				if (m_bFail || n > Left())
				{
					m_bFail = true;
					return false;
				}
				if (n)
					memcpy(pOut, m_p + m_pos, n);
				m_pos += n;
				return true;
			}
			uint32_t U32()		{ uint32_t v = 0; Raw(&v, 4); return v; }
			uint16_t U16()		{ uint16_t v = 0; Raw(&v, 2); return v; }
			void Align4()		{ Seek((m_pos + 3) & ~(size_t) 3); }

			// Cynex: eleman sayisi kalan veriyi asamaz (bozuk dosyada dev ayirma olmasin)
			bool Count(uint32_t n, size_t elemSize)
			{
				if (m_bFail || (elemSize && n > Left() / elemSize))
				{
					m_bFail = true;
					return false;
				}
				return true;
			}
			template <typename T> void Array(std::vector<T>& v, uint32_t n)
			{
				if (!Count(n, sizeof(T)))
					return;
				v.resize(n);
				Raw(n ? &v[0] : NULL, n * sizeof(T));
			}
			void Colors(std::vector<unsigned long>& v, uint32_t n)
			{
				if (!Count(n, 4))
					return;
				v.resize(n);
				for (uint32_t i = 0; i < n; ++i)
					v[i] = (unsigned long) U32();
			}
			void Bytes(std::vector<unsigned char>& v, uint32_t n)
			{
				Array(v, n);
				Align4();
			}
			bool String(std::string& s, bool& bPresent)
			{
				const uint16_t n = U16();
				if (n == 0xFFFF)
				{
					bPresent = false;
					s.clear();
					return !m_bFail;
				}
				bPresent = true;
				if (!Count(n, 1))
					return false;
				s.assign((const char*) m_p + m_pos, n);
				m_pos += n;
				return true;
			}

		private:
			const unsigned char*	m_p;
			size_t					m_n;
			size_t					m_pos;
			bool					m_bFail;
	};

	void ReadIndexed(CReader& r, SCynexSptIndexed& s)
	{
		s.m_uiVertexCount = r.U32();
		s.m_uiMask = r.U32();
		const uint32_t n = s.m_uiVertexCount;
		if (n > 65535)
		{
			r.Seek((size_t) -1);
			return;
		}
		if (s.m_uiMask & CynexSpt::ARRAY_COORDS)		r.Array(s.m_vecCoords, n * 3);
		if (s.m_uiMask & CynexSpt::ARRAY_NORMALS)		r.Array(s.m_vecNormals, n * 3);
		if (s.m_uiMask & CynexSpt::ARRAY_BINORMALS)		r.Array(s.m_vecBinormals, n * 3);
		if (s.m_uiMask & CynexSpt::ARRAY_TANGENTS)		r.Array(s.m_vecTangents, n * 3);
		if (s.m_uiMask & CynexSpt::ARRAY_TEXCOORDS0)	r.Array(s.m_vecTexCoords0, n * 2);
		if (s.m_uiMask & CynexSpt::ARRAY_TEXCOORDS1)	r.Array(s.m_vecTexCoords1, n * 2);
		if (s.m_uiMask & CynexSpt::ARRAY_COLORS)		r.Colors(s.m_vecColors, n);
		if (s.m_uiMask & CynexSpt::ARRAY_WIND_WEIGHTS)	r.Array(s.m_vecWindWeights, n);
		if (s.m_uiMask & CynexSpt::ARRAY_WIND_INDICES)	r.Bytes(s.m_vecWindIndices, n);
		if (s.m_uiMask & CynexSpt::ARRAY_FROND_LOCAL_UV)
		{
			r.Array(s.m_vecLocalUV, n * 2);
			r.Bytes(s.m_vecFrondMap, n);
		}

		const uint32_t uiLods = r.U32();
		if (!r.Count(uiLods, 4))
			return;
		s.m_vecLodStripCount.resize(uiLods);
		s.m_vecLodStripStart.resize(uiLods);
		for (uint32_t l = 0; l < uiLods && !r.Fail(); ++l)
		{
			const uint32_t uiStrips = r.U32();
			if (!r.Count(uiStrips, 4) || uiStrips > 65535)
			{
				r.Seek((size_t) -1);
				return;
			}
			s.m_vecLodStripCount[l] = (unsigned short) uiStrips;
			s.m_vecLodStripStart[l] = (unsigned int) s.m_vecStripLengths.size();
			for (uint32_t k = 0; k < uiStrips && !r.Fail(); ++k)
			{
				const uint32_t uiLen = r.U32();
				if (!r.Count(uiLen, 2) || uiLen > 65535)
				{
					r.Seek((size_t) -1);
					return;
				}
				s.m_vecStripLengths.push_back((unsigned short) uiLen);
				s.m_vecStripOffsets.push_back((unsigned int) s.m_vecIndices.size());
				const size_t base = s.m_vecIndices.size();
				s.m_vecIndices.resize(base + uiLen);
				r.Raw(uiLen ? &s.m_vecIndices[base] : NULL, uiLen * 2);
				r.Align4();
			}
		}
		const uint32_t uiMapLods = r.U32();
		if (!r.Count(uiMapLods, 4))
			return;
		s.m_vecLodMapIndexes.resize(uiMapLods);
		for (uint32_t l = 0; l < uiMapLods && !r.Fail(); ++l)
			r.Bytes(s.m_vecLodMapIndexes[l], r.U32());

		// Cynex: indeksler vertex sayisini asmamali
		for (size_t i = 0; i < s.m_vecIndices.size(); ++i)
			if (s.m_vecIndices[i] >= n)
			{
				r.Seek((size_t) -1);
				return;
			}
	}

	void FinishStrips(SCynexSptIndexed& s)
	{
		s.m_vecStripPointers.resize(s.m_vecStripLengths.size());
		for (size_t k = 0; k < s.m_vecStripLengths.size(); ++k)
			s.m_vecStripPointers[k] = s.m_vecIndices.empty() ? NULL : &s.m_vecIndices[0] + s.m_vecStripOffsets[k];

		// Cynex: dll CPU ruzgarda sadece o anki LOD'un strip'lerindeki vertexleri gunceller
		s.m_vecLodVertices.resize(s.m_vecLodStripCount.size());
		std::vector<unsigned char> vecUsed(s.m_uiVertexCount, 0);
		for (size_t l = 0; l < s.m_vecLodStripCount.size(); ++l)
		{
			std::fill(vecUsed.begin(), vecUsed.end(), (unsigned char) 0);
			for (unsigned int k = 0; k < s.m_vecLodStripCount[l]; ++k)
			{
				const unsigned int strip = s.m_vecLodStripStart[l] + k;
				const unsigned short* p = s.m_vecStripPointers[strip];
				for (unsigned int i = 0; i < s.m_vecStripLengths[strip]; ++i)
					vecUsed[p[i]] = 1;
			}
			for (unsigned int v = 0; v < s.m_uiVertexCount; ++v)
				if (vecUsed[v])
					s.m_vecLodVertices[l].push_back((unsigned short) v);
		}
	}

	void FlipTexCoords(const std::vector<float>& in, std::vector<float>& out, bool bFlip)
	{
		out = in;
		if (bFlip)
			for (size_t i = 1; i < out.size(); i += 2)
				out[i] = -out[i];
	}
}

unsigned int SCynexSptIndexed::GetTriangleCount(int iLod) const
{
	if (iLod < 0 || iLod >= (int) m_vecLodStripCount.size())
		return 0;
	unsigned int uiCount = 0;
	const unsigned int start = m_vecLodStripStart[iLod];
	for (unsigned int k = 0; k < m_vecLodStripCount[iLod]; ++k)
		if (m_vecStripLengths[start + k] > 2)
			uiCount += m_vecStripLengths[start + k] - 2;
	return uiCount;
}

///////////////////////////////////////////////////////////////////////
//	SCynexSptModel

SCynexSptModel* SCynexSptModel::ms_pFirst = NULL;

SCynexSptModel::SCynexSptModel() :
	m_uiRefCount(1),
	m_bLoaded(false),
	m_bComputed(false),
	m_pPrev(NULL),
	m_pNext(NULL),
	m_bHasComposite(false),
	m_bHasSelfShadow(false),
	m_bHasUserData(false),
	m_eBranchLighting(CSpeedTreeRT::LIGHT_DYNAMIC),
	m_eLeafLighting(CSpeedTreeRT::LIGHT_DYNAMIC),
	m_eFrondLighting(CSpeedTreeRT::LIGHT_DYNAMIC),
	m_eStaticStyle(CSpeedTreeRT::SLS_BASIC),
	m_eBranchWind(CSpeedTreeRT::WIND_NONE),
	m_eLeafWind(CSpeedTreeRT::WIND_NONE),
	m_eFrondWind(CSpeedTreeRT::WIND_NONE),
	m_fLeafLightingAdjustment(0.5f),
	m_uiLocalStart(0),
	m_uiLocalSpan(4),
	m_uiRockingGroups(3),
	m_fTreeSize(0.0f),
	m_fTreeSizeVariance(0.0f),
	m_fGeometryScale(1.0f),
	m_fTargetAlpha(84.0f),
	m_fLodNear(100.0f),
	m_fLodFar(1100.0f),
	m_fWindStrength(0.25f),
	m_fRockStrength(0.25f),
	m_fRockOffset(0.0f),
	m_bRocking(true),
	m_bLeavesDirty(true),
	m_bTextureFlip(false)
{
	memset(&m_sHead, 0, sizeof(m_sHead));
	memset(&m_sBillboard, 0, sizeof(m_sBillboard));
	for (int i = 0; i < 13; ++i)
		m_afBranchMaterial[i] = m_afLeafMaterial[i] = m_afFrondMaterial[i] = (i < 6) ? 1.0f : 0.0f;
	memset(m_afBillboardTexCoords, 0, sizeof(m_afBillboardTexCoords));
	memset(m_afHorizontalTexCoords, 0, sizeof(m_afHorizontalTexCoords));
	memset(m_afBillboardCoords0, 0, sizeof(m_afBillboardCoords0));
	memset(m_afBillboardCoords1, 0, sizeof(m_afBillboardCoords1));
	Link();
}

SCynexSptModel::~SCynexSptModel()
{
	Unlink();
}

void SCynexSptModel::Link()
{
	m_pPrev = NULL;
	m_pNext = ms_pFirst;
	if (ms_pFirst)
		ms_pFirst->m_pPrev = this;
	ms_pFirst = this;
}

void SCynexSptModel::Unlink()
{
	if (m_pPrev)
		m_pPrev->m_pNext = m_pNext;
	else if (ms_pFirst == this)
		ms_pFirst = m_pNext;
	if (m_pNext)
		m_pNext->m_pPrev = m_pPrev;
	m_pPrev = m_pNext = NULL;
}

bool SCynexSptModel::Load(const unsigned char* pBlock, unsigned int uiSize, std::string& strError)
{
	if (!pBlock || uiSize < 16 || memcmp(pBlock, CynexSpt::c_szMagic, 8) != 0)
	{
		strError = "not a converted SpeedTree file (run sptconv)";
		return false;
	}
	CReader r(pBlock, uiSize);
	r.Seek(8);
	const uint32_t uiVersion = r.U32();
	const uint32_t uiChunks = r.U32();
	if (uiVersion != CynexSpt::c_uiVersion)
	{
		strError = "unsupported converted SpeedTree file version";
		return false;
	}

	bool bHead = false;
	for (uint32_t c = 0; c < uiChunks && !r.Fail(); ++c)
	{
		r.Align4();
		const uint32_t uiTag = r.U32();
		const uint32_t uiBytes = r.U32();
		if (r.Fail() || uiBytes > r.Left())
		{
			strError = "truncated chunk";
			return false;
		}
		const size_t start = r.Pos();
		CReader cr(pBlock + start, uiBytes);
		switch (uiTag)
		{
			case CynexSpt::TAG_HEAD:
			{
				cr.Raw(&m_sHead, sizeof(m_sHead));
				cr.Array(m_vecLeafLodSizes, cr.U32());
				bHead = !cr.Fail();
				break;
			}
			case CynexSpt::TAG_TEXS:
			{
				bool bBranch = false;
				cr.String(m_strBranchTexture, bBranch);
				cr.String(m_strComposite, m_bHasComposite);
				cr.String(m_strSelfShadow, m_bHasSelfShadow);
				cr.String(m_strUserData, m_bHasUserData);
				bool bDummy = false;
				m_vecLeafTextures.resize(cr.U16());
				for (size_t i = 0; i < m_vecLeafTextures.size() && !cr.Fail(); ++i)
					cr.String(m_vecLeafTextures[i], bDummy);
				m_vecFrondTextures.resize(cr.U16());
				for (size_t i = 0; i < m_vecFrondTextures.size() && !cr.Fail(); ++i)
					cr.String(m_vecFrondTextures[i], bDummy);
				break;
			}
			case CynexSpt::TAG_COLL:
				cr.Array(m_vecCollisions, cr.U32());
				break;
			case CynexSpt::TAG_BRCH:
				ReadIndexed(cr, m_sBranches);
				break;
			case CynexSpt::TAG_FRND:
				ReadIndexed(cr, m_sFronds);
				break;
			case CynexSpt::TAG_LEAF:
			{
				cr.Array(m_vecClusters, cr.U32());
				const uint32_t uiMaps = cr.U32();
				cr.Array(m_vecLeafMapTexCoords, uiMaps * 8);
				cr.Bytes(m_vecLeafMapNoFlip, uiMaps);
				const uint32_t uiLods = cr.U32();
				if (!cr.Count(uiLods, 8))
					break;
				m_vecLeafLods.resize(uiLods);
				for (uint32_t l = 0; l < uiLods && !cr.Fail(); ++l)
				{
					SCynexSptLeafLod& lod = m_vecLeafLods[l];
					lod.m_uiCount = cr.U32();
					lod.m_uiMask = cr.U32();
					const uint32_t n = lod.m_uiCount;
					if (n > 65535)
					{
						cr.Seek((size_t) -1);
						break;
					}
					if (!n)
						continue;
					cr.Bytes(lod.m_vecMapIndices, n);
					cr.Bytes(lod.m_vecClusterIndices, n);
					cr.Array(lod.m_vecCenters, n * 3);
					if (lod.m_uiMask & CynexSpt::LEAF_COLORS_STATIC)	cr.Colors(lod.m_vecColorsStatic, n);
					if (lod.m_uiMask & CynexSpt::LEAF_COLORS_DYNAMIC)	cr.Colors(lod.m_vecColorsDynamic, n);
					if (lod.m_uiMask & CynexSpt::LEAF_NORMALS)		cr.Array(lod.m_vecNormals, n * 3);
					if (lod.m_uiMask & CynexSpt::LEAF_BINORMALS)		cr.Array(lod.m_vecBinormals, n * 3);
					if (lod.m_uiMask & CynexSpt::LEAF_TANGENTS)		cr.Array(lod.m_vecTangents, n * 3);
					if (lod.m_uiMask & CynexSpt::LEAF_WIND_WEIGHTS)	cr.Array(lod.m_vecWindWeights, n);
					if (lod.m_uiMask & CynexSpt::LEAF_WIND_INDICES)	cr.Bytes(lod.m_vecWindIndices, n);
				}
				break;
			}
			case CynexSpt::TAG_BILL:
			{
				cr.Raw(&m_sBillboard, sizeof(m_sBillboard));
				if (m_sBillboard.uiNum360 > 64)
				{
					cr.Seek((size_t) -1);
					break;
				}
				cr.Array(m_vec360TexCoords, m_sBillboard.uiNum360 * 8);
				break;
			}
			default:
				break;
		}
		if (cr.Fail())
		{
			strError = "corrupt converted SpeedTree file";
			return false;
		}
		r.Seek(start + uiBytes);
	}
	if (r.Fail() || !bHead)
	{
		strError = "corrupt converted SpeedTree file";
		return false;
	}

	// Cynex: tutarlilik: kume ve harita indeksleri tablolarin icinde olmali
	const size_t uiMaps = m_vecLeafMapTexCoords.size() / 8;
	for (size_t l = 0; l < m_vecLeafLods.size(); ++l)
	{
		const SCynexSptLeafLod& lod = m_vecLeafLods[l];
		for (unsigned int i = 0; i < lod.m_uiCount; ++i)
			if (lod.m_vecClusterIndices[i] >= m_vecClusters.size() || lod.m_vecMapIndices[i] >= uiMaps)
			{
				strError = "corrupt leaf tables";
				return false;
			}
	}
	if (m_vecLeafLodSizes.size() < m_vecLeafLods.size())
		m_vecLeafLodSizes.resize(m_vecLeafLods.size(), 1.0f);

	FinishStrips(m_sBranches);
	FinishStrips(m_sFronds);
	m_vecFile.assign(pBlock, pBlock + uiSize);
	m_vecLeafTexturePointers.resize(m_vecLeafTextures.size());
	for (size_t i = 0; i < m_vecLeafTextures.size(); ++i)
		m_vecLeafTexturePointers[i] = m_vecLeafTextures[i].c_str();
	m_vecFrondTexturePointers.resize(m_vecFrondTextures.size());
	for (size_t i = 0; i < m_vecFrondTextures.size(); ++i)
		m_vecFrondTexturePointers[i] = m_vecFrondTextures[i].c_str();
	m_bLoaded = true;
	ResetSettingsFromFile();
	return true;
}

void SCynexSptModel::ResetSettingsFromFile()
{
	const CynexSpt::SHead& h = m_sHead;
	m_eBranchLighting = (CSpeedTreeRT::ELightingMethod) h.iBranchLighting;
	m_eLeafLighting = (CSpeedTreeRT::ELightingMethod) h.iLeafLighting;
	m_eFrondLighting = (CSpeedTreeRT::ELightingMethod) h.iFrondLighting;
	m_eStaticStyle = (CSpeedTreeRT::EStaticLightingStyle) h.iStaticLightingStyle;
	m_eBranchWind = (CSpeedTreeRT::EWindMethod) h.iBranchWind;
	m_eLeafWind = (CSpeedTreeRT::EWindMethod) h.iLeafWind;
	m_eFrondWind = (CSpeedTreeRT::EWindMethod) h.iFrondWind;
	m_bRocking = h.iLeafRocking != 0;
	m_fLeafLightingAdjustment = h.fLeafLightingAdjustment;
	memcpy(m_afBranchMaterial, h.afBranchMaterial, sizeof(m_afBranchMaterial));
	memcpy(m_afLeafMaterial, h.afLeafMaterial, sizeof(m_afLeafMaterial));
	memcpy(m_afFrondMaterial, h.afFrondMaterial, sizeof(m_afFrondMaterial));
	m_fLodNear = h.fLodNear;
	m_fLodFar = h.fLodFar;
	m_fTreeSize = h.fTreeSize;
	m_fTreeSizeVariance = h.fTreeSizeVariance;
	// Cynex: yuklemeden once verilen GetWindStrength degeri korunur, sallanma dosyadaki guce doner
	m_fRockStrength = h.fWindStrengthInitial;
	m_fRockOffset = 0.0f;
}

bool SCynexSptModel::Compute(const float* pTransform)
{
	SCynexSptGlobals& g = CynexSptGlobals();
	m_bTextureFlip = g.m_bTextureFlip;

	// Cynex: SetTreeSize Compute oncesi degistiyse geometri esit olcekle (dosya tek tohumla uretildi)
	m_fGeometryScale = (m_sHead.fTreeSize > 0.0f && m_fTreeSize > 0.0f) ? m_fTreeSize / m_sHead.fTreeSize : 1.0f;
	const bool bScale = fabsf(m_fGeometryScale - 1.0f) > 1e-6f;

	SCynexSptIndexed* apParts[2] = { &m_sBranches, &m_sFronds };
	for (int p = 0; p < 2; ++p)
	{
		SCynexSptIndexed& s = *apParts[p];
		if (bScale)
			for (size_t i = 0; i < s.m_vecCoords.size(); ++i)
				s.m_vecCoords[i] *= m_fGeometryScale;
		if (pTransform)
		{
			// Cynex: 4x4 satir duzeni (D3D), konum + normal/binormal/tangent
			const float* m = pTransform;
			for (size_t i = 0; i + 2 < s.m_vecCoords.size(); i += 3)
			{
				const float x = s.m_vecCoords[i], y = s.m_vecCoords[i + 1], z = s.m_vecCoords[i + 2];
				s.m_vecCoords[i] = x * m[0] + y * m[4] + z * m[8] + m[12];
				s.m_vecCoords[i + 1] = x * m[1] + y * m[5] + z * m[9] + m[13];
				s.m_vecCoords[i + 2] = x * m[2] + y * m[6] + z * m[10] + m[14];
			}
			std::vector<float>* apVec[3] = { &s.m_vecNormals, &s.m_vecBinormals, &s.m_vecTangents };
			for (int v = 0; v < 3; ++v)
				for (size_t i = 0; i + 2 < apVec[v]->size(); i += 3)
				{
					float* q = &(*apVec[v])[i];
					const float x = q[0], y = q[1], z = q[2];
					q[0] = x * m[0] + y * m[4] + z * m[8];
					q[1] = x * m[1] + y * m[5] + z * m[9];
					q[2] = x * m[2] + y * m[6] + z * m[10];
				}
		}
		FlipTexCoords(s.m_vecTexCoords0, s.m_vecTexCoords0Out, m_bTextureFlip);
		FlipTexCoords(s.m_vecTexCoords1, s.m_vecTexCoords1Out, m_bTextureFlip);
		const CSpeedTreeRT::EWindMethod eWind = p == 0 ? m_eBranchWind : m_eFrondWind;
		if (eWind == CSpeedTreeRT::WIND_CPU)
			s.m_vecCoordsWind = s.m_vecCoords;
		else
			std::vector<float>().swap(s.m_vecCoordsWind);
	}

	FlipTexCoords(m_vecLeafMapTexCoords, m_vecLeafMapTexCoordsOut, m_bTextureFlip);
	// Cynex: gomulu texcoord'u olmayan haritalar (varsayilan birim kare) dll'de cevrilmez
	if (m_bTextureFlip)
		for (size_t i = 0; i < m_vecLeafMapNoFlip.size() && (i + 1) * 8 <= m_vecLeafMapTexCoordsOut.size(); ++i)
			if (m_vecLeafMapNoFlip[i])
				memcpy(&m_vecLeafMapTexCoordsOut[i * 8], &m_vecLeafMapTexCoords[i * 8], 8 * sizeof(float));
	const size_t uiClusters = m_vecClusters.size();
	m_vecBillboardTable.assign(uiClusters * 16, 0.0f);
	for (size_t l = 0; l < m_vecLeafLods.size(); ++l)
	{
		SCynexSptLeafLod& lod = m_vecLeafLods[l];
		if (bScale || pTransform)
			for (size_t i = 0; i + 2 < lod.m_vecCenters.size(); i += 3)
			{
				float* q = &lod.m_vecCenters[i];
				q[0] *= m_fGeometryScale; q[1] *= m_fGeometryScale; q[2] *= m_fGeometryScale;
				if (pTransform)
				{
					const float* m = pTransform;
					const float x = q[0], y = q[1], z = q[2];
					q[0] = x * m[0] + y * m[4] + z * m[8] + m[12];
					q[1] = x * m[1] + y * m[5] + z * m[9] + m[13];
					q[2] = x * m[2] + y * m[6] + z * m[10] + m[14];
				}
			}
		lod.m_vecCorners.assign(uiClusters * 16, 0.0f);
		lod.m_vecTexPointers.resize(lod.m_uiCount);
		lod.m_vecCoordPointers.resize(lod.m_uiCount);
		for (unsigned int i = 0; i < lod.m_uiCount; ++i)
		{
			lod.m_vecTexPointers[i] = &m_vecLeafMapTexCoordsOut[lod.m_vecMapIndices[i] * 8];
			lod.m_vecCoordPointers[i] = &lod.m_vecCorners[lod.m_vecClusterIndices[i] * 16];
		}
		if (m_eLeafWind == CSpeedTreeRT::WIND_CPU)
			lod.m_vecCentersWind = lod.m_vecCenters;
		else
			std::vector<float>().swap(lod.m_vecCentersWind);
	}
	if (bScale)
	{
		for (size_t c = 0; c < uiClusters; ++c)
			for (int k = 0; k < 12; ++k)
				m_vecClusters[c].afCorners[k] *= m_fGeometryScale;
		for (int i = 0; i < 6; ++i)
			m_sHead.afBoundingBox[i] *= m_fGeometryScale;
		m_sBillboard.fHalfWidth *= m_fGeometryScale;
		m_sBillboard.fTop *= m_fGeometryScale;
		m_sBillboard.fBottom *= m_fGeometryScale;
		for (int i = 0; i < 12; ++i)
			m_sBillboard.afHorizontalCoords[i] *= m_fGeometryScale;
	}

	memcpy(m_afBillboardTexCoords, m_sBillboard.afTexCoords, sizeof(m_afBillboardTexCoords));
	memcpy(m_afHorizontalTexCoords, m_sBillboard.afHorizontalTexCoords, sizeof(m_afHorizontalTexCoords));
	m_vec360TexCoordsOut = m_vec360TexCoords;
	if (m_bTextureFlip)
	{
		for (int i = 1; i < 8; i += 2)
		{
			m_afBillboardTexCoords[i] = -m_afBillboardTexCoords[i];
			m_afHorizontalTexCoords[i] = -m_afHorizontalTexCoords[i];
		}
		for (size_t i = 1; i < m_vec360TexCoordsOut.size(); i += 2)
			m_vec360TexCoordsOut[i] = -m_vec360TexCoordsOut[i];
	}

	// Cynex: dll Compute'ta ruzgar gucunu dosyadakine dondurur
	m_fWindStrength = m_fRockStrength;
	m_bLeavesDirty = true;
	m_bComputed = true;
	return true;
}

///////////////////////////////////////////////////////////////////////
//	yaprak kose hesabi: kose = boy * Rz(az - pi + yaw) * Ry(egim - pitch) * Rx(sallanma) * Q

void SCynexSptModel::UpdateLeafCorners()
{
	const SCynexSptGlobals& g = CynexSptGlobals();
	const size_t uiClusters = m_vecClusters.size();
	const double dPi = 3.14159265358979323846;
	// Cynex: dll acilari x87 double ile hesapliyor; ayni hassasiyet icin double
	const double dRockAmp = (double) m_sHead.fRockAngle * m_fRockStrength;
	const double dRockArg = (double) m_sHead.fRockFrequency * m_fRockStrength * g.m_fTime + m_fRockOffset;
	// Cynex: sallanma kapaliyken dll kume basina kucuk sabit aci birakiyor: s=1 genliginin 1/1200'u (olculdu)
	const double dRestAmp = (double) m_sHead.fRockAngle / 1200.0;

	for (size_t c = 0; c < uiClusters; ++c)
	{
		const CynexSpt::SLeafCluster& kCluster = m_vecClusters[c];
		const double a = (double) g.m_fCameraAzimuth - dPi + kCluster.fYaw;
		const double b = (double) g.m_fCameraPitch - kCluster.fPitch;
		const double r = m_bRocking ? dRockAmp * sin(dRockArg + kCluster.fPhase) : dRestAmp * sin((double) kCluster.fPhase);
		const double ca = cos(a), sa = sin(a);
		const double cb = cos(b), sb = sin(b);
		const double cr = cos(r), sr = sin(r);
		// Cynex: Rz * Ry * Rx
		const double m00 = ca * cb, m01 = ca * sb * sr - sa * cr, m02 = ca * sb * cr + sa * sr;
		const double m10 = sa * cb, m11 = sa * sb * sr + ca * cr, m12 = sa * sb * cr - ca * sr;
		const double m20 = -sb, m21 = cb * sr, m22 = cb * cr;
		float* pOut = &m_vecBillboardTable[c * 16];
		for (int k = 0; k < 4; ++k)
		{
			const float* q = &kCluster.afCorners[k * 3];
			pOut[k * 4 + 0] = (float) (m00 * q[0] + m01 * q[1] + m02 * q[2]);
			pOut[k * 4 + 1] = (float) (m10 * q[0] + m11 * q[1] + m12 * q[2]);
			pOut[k * 4 + 2] = (float) (m20 * q[0] + m21 * q[1] + m22 * q[2]);
			pOut[k * 4 + 3] = 0.0f;
		}
	}
	for (size_t l = 0; l < m_vecLeafLods.size(); ++l)
	{
		SCynexSptLeafLod& lod = m_vecLeafLods[l];
		const float fSize = m_vecLeafLodSizes[l];
		float* pDst = lod.m_vecCorners.empty() ? NULL : &lod.m_vecCorners[0];
		const float* pSrc = m_vecBillboardTable.empty() ? NULL : &m_vecBillboardTable[0];
		for (size_t i = 0; i < uiClusters * 16; ++i)
			pDst[i] = pSrc[i] * fSize;
	}
	m_bLeavesDirty = false;
}
