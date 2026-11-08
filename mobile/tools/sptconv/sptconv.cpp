// Cynex: sptconv - SpeedTreeRT 1.6 .spt dosyalarini mobil runtime (cynex_speedtree) formatina cevirir
// Cynex: gercek SpeedTreeRT.dll kara kutu olarak kullanilir; her dosya ayri alt surecte islenir (dll cokmeleri izole)
#include <windows.h>
#include <SpeedTreeRT.h>

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "../../speedtree/src/CynexSptFormat.h"

namespace fs = std::filesystem;

///////////////////////////////////////////////////////////////////////
//	genel yardimcilar

static std::string ToUtf8(const std::wstring& w)
{
	if (w.empty())
		return std::string();
	const int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int) w.size(), NULL, 0, NULL, NULL);
	std::string s(n, '\0');
	WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int) w.size(), &s[0], n, NULL, NULL);
	return s;
}

static bool ReadWholeFile(const fs::path& path, std::vector<unsigned char>& out)
{
	FILE* fp = _wfopen(path.c_str(), L"rb");
	if (!fp)
		return false;
	fseek(fp, 0, SEEK_END);
	const long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (size <= 0)
	{
		fclose(fp);
		return false;
	}
	out.resize((size_t) size);
	const size_t got = fread(&out[0], 1, (size_t) size, fp);
	fclose(fp);
	return got == (size_t) size;
}

static bool WriteWholeFile(const fs::path& path, const std::vector<unsigned char>& data)
{
	std::error_code ec;
	fs::create_directories(path.parent_path(), ec);
	FILE* fp = _wfopen(path.c_str(), L"wb");
	if (!fp)
		return false;
	const size_t put = data.empty() ? 0 : fwrite(&data[0], 1, data.size(), fp);
	fclose(fp);
	return put == data.size();
}

class CWriter
{
	public:
		void Raw(const void* p, size_t n)
		{
			const unsigned char* b = (const unsigned char*) p;
			m_vecData.insert(m_vecData.end(), b, b + n);
		}
		void U8(uint8_t v)			{ Raw(&v, 1); }
		void U16(uint16_t v)		{ Raw(&v, 2); }
		void U32(uint32_t v)		{ Raw(&v, 4); }
		void F32(float v)			{ Raw(&v, 4); }
		void Floats(const float* p, size_t n)	{ if (n) Raw(p, n * 4); }
		void Align4()				{ while (m_vecData.size() & 3) m_vecData.push_back(0); }
		void String(const char* s)
		{
			if (!s)
			{
				U16(0xFFFF);
				return;
			}
			const size_t n = strlen(s);
			U16((uint16_t) n);
			Raw(s, n);
		}
		size_t BeginChunk(uint32_t uiTag)
		{
			Align4();
			U32(uiTag);
			U32(0);
			return m_vecData.size();
		}
		void EndChunk(size_t start)
		{
			Align4();
			const uint32_t size = (uint32_t) (m_vecData.size() - start);
			memcpy(&m_vecData[start - 4], &size, 4);
		}
		std::vector<unsigned char>& Data()	{ return m_vecData; }

	private:
		std::vector<unsigned char> m_vecData;
};

///////////////////////////////////////////////////////////////////////
//	oracle ayarlari (oyunun SpeedTreeLib ile birebir)

// Cynex: CSpeedTreeForestDirectX8::SetRenderingDevice isik 0
static const float c_afGameLight[16] =
{
	-0.707f, -0.300f, 0.707f,	// pos
	1.0f, 1.0f, 1.0f,			// diffuse
	0.5f, 0.5f, 0.5f,			// ambient
	1.0f, 1.0f, 1.0f,			// specular
	0.0f,						// directional
	1.0f, 0.0f, 0.0f			// attenuation
};

static const float c_fTargetAlpha = 84.0f;

static void SetCameraDir(float x, float y, float z)
{
	const float afPos[3] = { 0.0f, 0.0f, 0.0f };
	const float afDir[3] = { x, y, z };
	CSpeedTreeRT::SetCamera(afPos, afDir);
}

static void InitOracleGlobals()
{
	CSpeedTreeRT::SetNumWindMatrices(4);
	CSpeedTreeRT::SetLightAttributes(0, c_afGameLight);
	CSpeedTreeRT::SetLightState(0, true);
	CSpeedTreeRT::SetTextureFlip(false);
	CSpeedTreeRT::SetDropToBillboard(false);
	CSpeedTreeRT::SetTime(0.0f);
	SetCameraDir(-1.0f, 0.0f, 0.0f);
}

// Cynex: dll GetGeometry icinde istisna yakalayip isi yarim birakabiliyor; parca parca ve SEH ile cagir
static bool SafeGetGeometry(CSpeedTreeRT* pTree, CSpeedTreeRT::SGeometry& g, unsigned long ulBits, short b = -1, short f = -1, short l = -1)
{
	__try
	{
		CSpeedTreeRT::ResetError();
		pTree->GetGeometry(g, ulBits, b, f, l);
		return CSpeedTreeRT::GetCurrentError()[0] == 0;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
}

static CSpeedTreeRT* MakeOracleTree(const std::vector<unsigned char>& block, CSpeedTreeRT::ELightingMethod eLight, CSpeedTreeRT::EWindMethod eWind, std::string& strError)
{
	CSpeedTreeRT* pTree = new CSpeedTreeRT;
	pTree->SetWindStrength(1.0f);
	pTree->SetLocalMatrices(0, 4);
	if (!pTree->LoadTree(&block[0], (unsigned int) block.size()))
	{
		strError = std::string("LoadTree: ") + CSpeedTreeRT::GetCurrentError();
		return NULL;
	}
	pTree->SetBranchLightingMethod(eLight);
	pTree->SetLeafLightingMethod(eLight);
	pTree->SetFrondLightingMethod(eLight);
	pTree->SetBranchWindMethod(eWind);
	pTree->SetLeafWindMethod(eWind);
	pTree->SetFrondWindMethod(eWind);
	pTree->SetNumLeafRockingGroups(1);
	if (!pTree->Compute(NULL, 1))
	{
		strError = std::string("Compute: ") + CSpeedTreeRT::GetCurrentError();
		return NULL;
	}
	pTree->SetLeafRockingState(true);
	return pTree;
}

///////////////////////////////////////////////////////////////////////
//	donusum verisi

struct SIndexedData
{
	uint32_t						uiMask = 0;
	unsigned int					uiVertexCount = 0;
	std::vector<float>				vecCoords, vecNormals, vecBinormals, vecTangents, vecTex0, vecTex1, vecWindWeights, vecLocalUV;
	std::vector<uint32_t>			vecColors;
	std::vector<uint8_t>			vecWindIndices, vecFrondMap;
	std::vector<std::vector<std::vector<uint16_t> > > vecLodStrips;	// lod -> strip -> index
	std::vector<std::vector<uint8_t> > vecLodMapIndexes;			// sadece frond
};

struct SLeafLod
{
	uint32_t						uiMask = 0;
	unsigned int					uiCount = 0;
	std::vector<uint8_t>			vecMapIndices, vecClusterIndices;
	std::vector<float>				vecCenters, vecNormals, vecBinormals, vecTangents, vecWindWeights;
	std::vector<uint32_t>			vecColorsStatic, vecColorsDynamic;
	std::vector<uint8_t>			vecWindIndices;
};

struct STreeData
{
	CynexSpt::SHead					sHead;
	std::string						strBranchTexture, strComposite, strSelfShadow, strUserData;
	bool							bHasComposite = false, bHasSelfShadow = false, bHasUserData = false;
	std::vector<std::string>		vecLeafTextures, vecFrondTextures;
	std::vector<CynexSpt::SCollision> vecCollisions;
	std::vector<float>				vecLeafLodSizes;
	SIndexedData					sBranches, sFronds;
	std::vector<CynexSpt::SLeafCluster> vecClusters;
	std::vector<float>				vecLeafMapTexCoords;	// harita basina 8
	std::vector<uint8_t>			vecLeafMapNoFlip;		// dll bu haritayi cevirmiyor (gomulu texcoord yok)
	std::vector<SLeafLod>			vecLeafLods;
	CynexSpt::SBillboard			sBillboard;
	std::vector<float>				vec360TexCoords;
	std::vector<std::string>		vecWarnings;
	double							dRockFitError = 0.0;
	double							dLodFitError = 0.0;
	double							d360FitError = 0.0;
};

static void Warn(STreeData& d, const char* fmt, ...)
{
	char buf[512];
	va_list va;
	va_start(va, fmt);
	vsnprintf(buf, sizeof(buf), fmt, va);
	va_end(va);
	d.vecWarnings.push_back(buf);
}

static bool SameFloats(const float* a, const float* b, size_t n, float fTol = 0.0f)
{
	for (size_t i = 0; i < n; ++i)
		if (fabsf(a[i] - b[i]) > fTol)
			return false;
	return true;
}

///////////////////////////////////////////////////////////////////////
//	indexed geometri (dal / frond)

static void ExtractIndexed(STreeData& d, CSpeedTreeRT* pA, CSpeedTreeRT* pB, bool bFronds, SIndexedData& out)
{
	const unsigned long ulBit = bFronds ? SpeedTree_FrondGeometry : SpeedTree_BranchGeometry;
	CSpeedTreeRT::SGeometry gA, gB;
	pA->SetLodLevel(1.0f);
	pB->SetLodLevel(1.0f);
	SafeGetGeometry(pA, gA, ulBit);
	SafeGetGeometry(pB, gB, ulBit);
	const CSpeedTreeRT::SGeometry::SIndexed& a = bFronds ? gA.m_sFronds : gA.m_sBranches;
	const CSpeedTreeRT::SGeometry::SIndexed& b = bFronds ? gB.m_sFronds : gB.m_sBranches;

	const unsigned int n = a.m_usVertexCount;
	out.uiVertexCount = n;
	if (b.m_usVertexCount != n)
		Warn(d, "%s: vertex count farkli (statik %u, dinamik %u)", bFronds ? "frond" : "branch", n, b.m_usVertexCount);

	if (n > 0)
	{
		if (a.m_pCoords)
		{
			out.uiMask |= CynexSpt::ARRAY_COORDS;
			out.vecCoords.assign(a.m_pCoords, a.m_pCoords + n * 3);
			if (b.m_pCoords && !SameFloats(a.m_pCoords, b.m_pCoords, n * 3))
				Warn(d, "%s: koordinatlar isik/ruzgar moduna gore degisiyor", bFronds ? "frond" : "branch");
		}
		if (b.m_pNormals)
		{
			out.uiMask |= CynexSpt::ARRAY_NORMALS;
			out.vecNormals.assign(b.m_pNormals, b.m_pNormals + n * 3);
		}
		// Cynex: binormal/tangent: dallarda statikte de var, frondlarda sadece dinamikte
		const float* pBin = a.m_pBinormals ? a.m_pBinormals : b.m_pBinormals;
		const float* pTan = a.m_pTangents ? a.m_pTangents : b.m_pTangents;
		if (a.m_pBinormals && a.m_pTangents)
			out.uiMask |= CynexSpt::ARRAY_BINTAN_STATIC;
		if (pBin)
		{
			out.uiMask |= CynexSpt::ARRAY_BINORMALS;
			out.vecBinormals.assign(pBin, pBin + n * 3);
		}
		if (pTan)
		{
			out.uiMask |= CynexSpt::ARRAY_TANGENTS;
			out.vecTangents.assign(pTan, pTan + n * 3);
		}
		if (a.m_pBinormals && b.m_pBinormals && !SameFloats(a.m_pBinormals, b.m_pBinormals, n * 3))
			Warn(d, "%s: binormal statik/dinamik farkli", bFronds ? "frond" : "branch");
		if (a.m_pTexCoords0)
		{
			out.uiMask |= CynexSpt::ARRAY_TEXCOORDS0;
			out.vecTex0.assign(a.m_pTexCoords0, a.m_pTexCoords0 + n * 2);
		}
		if (a.m_pTexCoords1)
		{
			out.uiMask |= CynexSpt::ARRAY_TEXCOORDS1;
			out.vecTex1.assign(a.m_pTexCoords1, a.m_pTexCoords1 + n * 2);
		}
		if (a.m_pColors)
		{
			out.uiMask |= CynexSpt::ARRAY_COLORS;
			out.vecColors.assign(a.m_pColors, a.m_pColors + n);
		}
		if (b.m_pWindWeights)
		{
			out.uiMask |= CynexSpt::ARRAY_WIND_WEIGHTS;
			out.vecWindWeights.assign(b.m_pWindWeights, b.m_pWindWeights + n);
		}
		if (b.m_pWindMatrixIndices)
		{
			out.uiMask |= CynexSpt::ARRAY_WIND_INDICES;
			out.vecWindIndices.assign(b.m_pWindMatrixIndices, b.m_pWindMatrixIndices + n);
		}
	}

	// Cynex: LOD strip'leri (frond yoksa override GetGeometry dll'de cokuyor, atla)
	const unsigned int uiLods = bFronds ? pA->GetNumFrondLodLevels() : pA->GetNumBranchLodLevels();
	out.vecLodStrips.resize(uiLods);
	if (bFronds)
		out.vecLodMapIndexes.resize(uiLods);
	for (unsigned int l = 0; l < uiLods; ++l)
	{
		if (n > 0)
		{
			CSpeedTreeRT::SGeometry g;
			if (SafeGetGeometry(pA, g, ulBit, bFronds ? -1 : (short) l, bFronds ? (short) l : -1, -1))
			{
				const CSpeedTreeRT::SGeometry::SIndexed& s = bFronds ? g.m_sFronds : g.m_sBranches;
				for (unsigned int k = 0; k < s.m_usNumStrips; ++k)
					out.vecLodStrips[l].push_back(std::vector<uint16_t>(s.m_pStrips[k], s.m_pStrips[k] + s.m_pStripLengths[k]));
			}
			else
				Warn(d, "%s lod %u GetGeometry hata", bFronds ? "frond" : "branch", l);
		}
		if (bFronds)
		{
			const size_t nStrips = std::max<size_t>(1, out.vecLodStrips[l].size());
			const unsigned char* p = n > 0 ? pA->GetFrondGeometryMapIndexes((int) l) : NULL;
			out.vecLodMapIndexes[l].assign(nStrips, 0);
			if (p)
				for (size_t k = 0; k < nStrips; ++k)
					out.vecLodMapIndexes[l][k] = p[k];
		}
	}
}

// Cynex: SetFrondTextureCoords icin vertex basina harita ici (u,v) ve harita indeksi:
// Cynex: harita m'ye u ekseninde [2m, 2m+1] dortgeni verilip geri okunur
// Cynex: dll bazi agaclarda SetFrondTextureCoords sonrasi heap'i bozuyor; ayri alt surecte yapilir
static int FrondUVChild(const fs::path& inPath, const fs::path& outPath, unsigned int uiFrondMaps)
{
	std::vector<unsigned char> block;
	if (!ReadWholeFile(inPath, block))
		return 3;
	InitOracleGlobals();
	std::string strError;
	CSpeedTreeRT* pT = MakeOracleTree(block, CSpeedTreeRT::LIGHT_STATIC, CSpeedTreeRT::WIND_NONE, strError);
	if (!pT)
		return 2;
	for (unsigned int m = 0; m < uiFrondMaps; ++m)
	{
		const float u0 = 2.0f * m, u1 = 2.0f * m + 1.0f;
		const float afRect[8] = { u0, 0.0f, u1, 0.0f, u1, 1.0f, u0, 1.0f };
		pT->SetFrondTextureCoords(m, afRect);
	}
	CSpeedTreeRT::SGeometry g;
	if (!SafeGetGeometry(pT, g, SpeedTree_FrondGeometry) || !g.m_sFronds.m_pTexCoords0)
		return 2;
	CWriter w;
	w.U32(g.m_sFronds.m_usVertexCount);
	w.Floats(g.m_sFronds.m_pTexCoords0, g.m_sFronds.m_usVertexCount * 2);
	return WriteWholeFile(outPath, w.Data()) ? 0 : 3;
}

static bool RunChild(const std::wstring& strArgs, DWORD& dwExit)
{
	wchar_t szExe[MAX_PATH * 4];
	GetModuleFileNameW(NULL, szExe, (DWORD) (sizeof(szExe) / sizeof(szExe[0])));
	std::wstring cmd = L"\"" + std::wstring(szExe) + L"\" " + strArgs;
	std::vector<wchar_t> buf(cmd.begin(), cmd.end());
	buf.push_back(0);
	STARTUPINFOW si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));
	if (!CreateProcessW(NULL, &buf[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
		return false;
	WaitForSingleObject(pi.hProcess, INFINITE);
	GetExitCodeProcess(pi.hProcess, &dwExit);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return true;
}

static void ExtractFrondLocalUV(STreeData& d, const fs::path& inPath, const fs::path& outPath, SIndexedData& fr, unsigned int uiFrondMaps)
{
	if (fr.uiVertexCount == 0 || uiFrondMaps == 0)
		return;
	fs::path tmpPath = outPath;
	tmpPath += L".fronduv.tmp";
	wchar_t szMaps[16];
	swprintf(szMaps, 16, L"%u", uiFrondMaps);
	DWORD dwExit = 1;
	std::vector<unsigned char> data;
	const bool bRan = RunChild(L"--fronduv \"" + inPath.wstring() + L"\" \"" + tmpPath.wstring() + L"\" " + szMaps, dwExit);
	const bool bRead = bRan && dwExit == 0 && ReadWholeFile(tmpPath, data);
	std::error_code ec;
	fs::remove(tmpPath, ec);
	uint32_t n = 0;
	if (bRead && data.size() >= 4)
		memcpy(&n, &data[0], 4);
	if (!bRead || n != fr.uiVertexCount || data.size() < 4 + n * 8)
	{
		Warn(d, "frond local uv okunamadi (dll SetFrondTextureCoords hatasi); SetFrondTextureCoords bu agacta etkisiz");
		return;
	}
	std::vector<float> vecTc(n * 2);
	memcpy(&vecTc[0], &data[4], n * 8);
	const float* tc = &vecTc[0];
	fr.vecLocalUV.resize(fr.uiVertexCount * 2);
	fr.vecFrondMap.resize(fr.uiVertexCount);
	for (unsigned int i = 0; i < fr.uiVertexCount; ++i)
	{
		int m = (int) floorf(tc[i * 2] * 0.5f);
		if (m < 0)
			m = 0;
		if (m >= (int) uiFrondMaps)
			m = (int) uiFrondMaps - 1;
		fr.vecFrondMap[i] = (uint8_t) m;
		fr.vecLocalUV[i * 2] = tc[i * 2] - 2.0f * m;
		fr.vecLocalUV[i * 2 + 1] = tc[i * 2 + 1];
	}
	fr.uiMask |= CynexSpt::ARRAY_FROND_LOCAL_UV;
}

///////////////////////////////////////////////////////////////////////
//	yapraklar

static void ExtractLeaves(STreeData& d, CSpeedTreeRT* pA, CSpeedTreeRT* pB)
{
	const unsigned int uiLods = pA->GetNumLeafLodLevels();
	const unsigned int uiMaps = (unsigned int) d.vecLeafTextures.size();
	d.vecLeafMapTexCoords.assign(std::max(1u, uiMaps) * 8, 0.0f);
	std::vector<bool> vecMapSeen(std::max(1u, uiMaps), false);
	d.vecLeafLods.resize(uiLods);

	for (unsigned int l = 0; l < uiLods; ++l)
	{
		CSpeedTreeRT::SGeometry gA, gB;
		if (!SafeGetGeometry(pA, gA, SpeedTree_LeafGeometry, -1, -1, (short) l) || !SafeGetGeometry(pB, gB, SpeedTree_LeafGeometry, -1, -1, (short) l))
		{
			Warn(d, "leaf lod %u GetGeometry hata", l);
			continue;
		}
		const CSpeedTreeRT::SGeometry::SLeaf& a = gA.m_sLeaves0;
		const CSpeedTreeRT::SGeometry::SLeaf& b = gB.m_sLeaves0;
		SLeafLod& out = d.vecLeafLods[l];
		const unsigned int n = a.m_bIsActive ? a.m_usLeafCount : 0;
		out.uiCount = n;
		if (n == 0)
			continue;
		if (b.m_usLeafCount != n)
			Warn(d, "leaf lod %u: sayi farkli (%u / %u)", l, n, b.m_usLeafCount);

		out.vecMapIndices.assign(a.m_pLeafMapIndices, a.m_pLeafMapIndices + n);
		out.vecClusterIndices.assign(a.m_pLeafClusterIndices, a.m_pLeafClusterIndices + n);
		out.vecCenters.assign(a.m_pCenterCoords, a.m_pCenterCoords + n * 3);
		if (b.m_pCenterCoords && !SameFloats(a.m_pCenterCoords, b.m_pCenterCoords, n * 3))
			Warn(d, "leaf lod %u: merkezler moda gore farkli", l);
		if (a.m_pColors)
		{
			out.uiMask |= CynexSpt::LEAF_COLORS_STATIC;
			out.vecColorsStatic.assign(a.m_pColors, a.m_pColors + n);
		}
		if (b.m_pColors)
		{
			out.uiMask |= CynexSpt::LEAF_COLORS_DYNAMIC;
			out.vecColorsDynamic.assign(b.m_pColors, b.m_pColors + n);
		}
		if (b.m_pNormals)
		{
			out.uiMask |= CynexSpt::LEAF_NORMALS;
			out.vecNormals.assign(b.m_pNormals, b.m_pNormals + n * 3);
		}
		if (b.m_pBinormals)
		{
			out.uiMask |= CynexSpt::LEAF_BINORMALS;
			out.vecBinormals.assign(b.m_pBinormals, b.m_pBinormals + n * 3);
		}
		if (b.m_pTangents)
		{
			out.uiMask |= CynexSpt::LEAF_TANGENTS;
			out.vecTangents.assign(b.m_pTangents, b.m_pTangents + n * 3);
		}
		if (b.m_pWindWeights)
		{
			out.uiMask |= CynexSpt::LEAF_WIND_WEIGHTS;
			out.vecWindWeights.assign(b.m_pWindWeights, b.m_pWindWeights + n);
		}
		if (b.m_pWindMatrixIndices)
		{
			out.uiMask |= CynexSpt::LEAF_WIND_INDICES;
			out.vecWindIndices.assign(b.m_pWindMatrixIndices, b.m_pWindMatrixIndices + n);
		}

		// Cynex: texcoord tablosu harita indeksine bagli olmali; dogrula
		for (unsigned int i = 0; i < n; ++i)
		{
			const unsigned int m = a.m_pLeafMapIndices[i];
			if (m >= vecMapSeen.size())
			{
				vecMapSeen.resize(m + 1, false);
				d.vecLeafMapTexCoords.resize((m + 1) * 8, 0.0f);
			}
			const float* tc = a.m_pLeafMapTexCoords[i];
			if (!vecMapSeen[m])
			{
				vecMapSeen[m] = true;
				memcpy(&d.vecLeafMapTexCoords[m * 8], tc, 8 * sizeof(float));
				// Cynex: B agaci texture flip acik hesaplandi; t ayni kaldiysa dll bu haritayi cevirmiyor
				if (d.vecLeafMapNoFlip.size() <= m)
					d.vecLeafMapNoFlip.resize(m + 1, 0);
				if (i < b.m_usLeafCount && b.m_pLeafMapTexCoords)
					for (int k = 1; k < 8; k += 2)
						if (tc[k] != 0.0f && b.m_pLeafMapTexCoords[i][k] == tc[k])
							d.vecLeafMapNoFlip[m] = 1;
			}
			else if (!SameFloats(&d.vecLeafMapTexCoords[m * 8], tc, 8))
				Warn(d, "leaf lod %u: texcoord harita disi degisiyor (leaf %u)", l, i);
		}
	}
}

typedef double TVec3[3];

static void RotZ(double a, const double* in, double* out)
{
	const double c = cos(a), s = sin(a);
	out[0] = c * in[0] - s * in[1];
	out[1] = s * in[0] + c * in[1];
	out[2] = in[2];
}

static void RotY(double a, const double* in, double* out)
{
	const double c = cos(a), s = sin(a);
	out[0] = c * in[0] + s * in[2];
	out[1] = in[1];
	out[2] = -s * in[0] + c * in[2];
}

// Cynex: kose = Rz(az - pi + yaw) * Ry(egim - pitch) * Rx(sallanma) * Q ; burada az = pi, egim = 0, sallanma = 0
static void SolveCluster(const float* pCorners16, CynexSpt::SLeafCluster& c)
{
	double q[4][3];
	for (int k = 0; k < 4; ++k)
		for (int j = 0; j < 3; ++j)
			q[k][j] = pCorners16[k * 4 + j];

	// Cynex: Newell normali; Q duzleminde +x yonune bakacak sekilde (kose sirasi sabit)
	double n[3] = { 0.0, 0.0, 0.0 };
	for (int k = 0; k < 4; ++k)
	{
		const double* p0 = q[k];
		const double* p1 = q[(k + 1) & 3];
		n[0] += (p0[1] - p1[1]) * (p0[2] + p1[2]);
		n[1] += (p0[2] - p1[2]) * (p0[0] + p1[0]);
		n[2] += (p0[0] - p1[0]) * (p0[1] + p1[1]);
	}
	const double len = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
	if (len < 1e-12)
	{
		c.fYaw = c.fPitch = 0.0f;
		for (int k = 0; k < 4; ++k)
			for (int j = 0; j < 3; ++j)
				c.afCorners[k * 3 + j] = (float) q[k][j];
		return;
	}
	n[0] /= len; n[1] /= len; n[2] /= len;
	// Cynex: Newell sirasi (0,1,2,3) Q'da -x verir; isareti cevir
	n[0] = -n[0]; n[1] = -n[1]; n[2] = -n[2];
	const double pitch = asin(std::max(-1.0, std::min(1.0, n[2])));
	const double yaw = atan2(n[1], n[0]);
	c.fYaw = (float) yaw;
	c.fPitch = (float) pitch;
	for (int k = 0; k < 4; ++k)
	{
		double t[3], u[3];
		RotZ(-yaw, q[k], t);
		RotY(pitch, t, u);
		for (int j = 0; j < 3; ++j)
			c.afCorners[k * 3 + j] = (float) u[j];
	}
}

// Cynex: kosesinden sallanma acisini bul (Q'daki ust kenar acisina gore)
static double ClusterRoll(const float* pCorners16, const CynexSpt::SLeafCluster& c)
{
	double e[3], t[3], u[3];
	for (int j = 0; j < 3; ++j)
		e[j] = (double) pCorners16[4 + j] - (double) pCorners16[j];
	RotZ(-c.fYaw, e, t);
	RotY(c.fPitch, t, u);
	const double qy = (double) c.afCorners[3 + 1] - c.afCorners[1];
	const double qz = (double) c.afCorners[3 + 2] - c.afCorners[2];
	double r = atan2(u[2], u[1]) - atan2(qz, qy);
	while (r > M_PI) r -= 2.0 * M_PI;
	while (r < -M_PI) r += 2.0 * M_PI;
	return r;
}

static float SnapToFileFloat(const std::vector<unsigned char>& block, float fValue, float fTol);

static void ExtractClustersAndRocking(STreeData& d, const std::vector<unsigned char>& block, CSpeedTreeRT* pA)
{
	CynexSpt::SHead& h = d.sHead;
	// Cynex: baslangic gucu Compute sonrasi GetWindStrength; frekans: t=1'de SetWindStrength(1, 0, 0) = -omega1
	const float fInitialStrength = pA->GetWindStrength();
	CSpeedTreeRT::SetTime(1.0f);
	const double dOmega = -(double) pA->SetWindStrength(1.0f, 0.0f, 0.0f);
	pA->SetWindStrength(0.0f, 0.0f, 0.0f);
	CSpeedTreeRT::SetTime(0.0f);
	SetCameraDir(-1.0f, 0.0f, 0.0f);
	h.fWindStrengthInitial = fInitialStrength;
	h.fRockFrequency = (float) dOmega;

	CSpeedTreeRT::SGeometry g;
	SafeGetGeometry(pA, g, SpeedTree_LeafGeometry, -1, -1, 0);
	unsigned int uiEntries = 0;
	const float* pTable = pA->GetLeafBillboardTable(uiEntries);
	const unsigned int uiClusters = pTable ? uiEntries / 16 : 0;
	d.vecClusters.resize(uiClusters);
	for (unsigned int c = 0; c < uiClusters; ++c)
	{
		memset(&d.vecClusters[c], 0, sizeof(CynexSpt::SLeafCluster));
		SolveCluster(pTable + c * 16, d.vecClusters[c]);
	}

	h.fRockAngle = 0.0f;
	// Cynex: yapraksiz agacta da tablo hesaplanir ve sallanir (GetLeafBillboardTable)
	if (uiClusters == 0)
		return;

	// Cynex: s=1 ile ornekle; x_k = A sin(w t_k + faz)
	pA->SetWindStrength(1.0f, 0.0f, 0.0f);
	const int K = 160;
	const double dt = 0.0125;
	std::vector<std::vector<double> > vecRoll(uiClusters, std::vector<double>(K));
	for (int k = 0; k < K; ++k)
	{
		CSpeedTreeRT::SetTime((float) (k * dt));
		SafeGetGeometry(pA, g, SpeedTree_LeafGeometry, -1, -1, 0);
		pTable = pA->GetLeafBillboardTable(uiEntries);
		for (unsigned int c = 0; c < uiClusters; ++c)
			vecRoll[c][k] = ClusterRoll(pTable + c * 16, d.vecClusters[c]);
	}

	// Cynex: x[k+1] + x[k-1] = 2 cos(w dt) x[k]
	double num = 0.0, den = 0.0;
	for (unsigned int c = 0; c < uiClusters; ++c)
		for (int k = 1; k < K - 1; ++k)
		{
			num += (vecRoll[c][k + 1] + vecRoll[c][k - 1]) * vecRoll[c][k];
			den += 2.0 * vecRoll[c][k] * vecRoll[c][k];
		}
	if (den < 1e-20)
	{
		Warn(d, "sallanma olculemedi (genlik 0)");
		CSpeedTreeRT::SetTime(0.0f);
		return;
	}
	const double cw = std::max(-1.0, std::min(1.0, num / den));
	const double wEstimate = acos(cw) / dt;
	const double w = dOmega > 0.0 ? dOmega : wEstimate;
	if (dOmega > 0.0 && wEstimate > 0.0 && fabs(wEstimate - dOmega) > 1e-3 * dOmega)
		Warn(d, "sallanma frekansi tutarsiz (%.6f / %.6f)", wEstimate, dOmega);

	double sumA = 0.0;
	std::vector<double> vecA(uiClusters), vecPhase(uiClusters);
	for (unsigned int c = 0; c < uiClusters; ++c)
	{
		// Cynex: x = a sin(wt) + b cos(wt) en kucuk kareler
		double ss = 0, sc = 0, cc = 0, xs = 0, xc = 0;
		for (int k = 0; k < K; ++k)
		{
			const double s = sin(w * k * dt), co = cos(w * k * dt);
			ss += s * s; sc += s * co; cc += co * co;
			xs += vecRoll[c][k] * s; xc += vecRoll[c][k] * co;
		}
		const double det = ss * cc - sc * sc;
		const double a = (xs * cc - xc * sc) / det;
		const double b = (xc * ss - xs * sc) / det;
		vecA[c] = sqrt(a * a + b * b);
		vecPhase[c] = atan2(b, a);
		sumA += vecA[c];
	}
	double A = sumA / uiClusters;
	double w2 = w;

	// Cynex: uzun pencere ile Gauss-Newton inceltme (omega hatasi zamanla buyuyor)
	{
		const int K2 = 200;
		const double dt2 = 0.2;
		std::vector<std::vector<double> > vecRoll2(uiClusters, std::vector<double>(K2));
		for (int k = 0; k < K2; ++k)
		{
			CSpeedTreeRT::SetTime((float) (k * dt2));
			SafeGetGeometry(pA, g, SpeedTree_LeafGeometry, -1, -1, 0);
			pTable = pA->GetLeafBillboardTable(uiEntries);
			for (unsigned int c = 0; c < uiClusters; ++c)
				vecRoll2[c][k] = ClusterRoll(pTable + c * 16, d.vecClusters[c]);
		}
		const int P = (int) uiClusters + 2;
		for (int it = 0; it < 8; ++it)
		{
			std::vector<double> JtJ(P * P, 0.0), Jtr(P, 0.0);
			for (unsigned int c = 0; c < uiClusters; ++c)
				for (int k = 0; k < K2; ++k)
				{
					const double t = k * dt2;
					const double arg = w2 * t + vecPhase[c];
					const double res = A * sin(arg) - vecRoll2[c][k];
					double J[3];
					int idx[3] = { 0, 1, 2 + (int) c };
					J[0] = sin(arg);
					J[1] = dOmega > 0.0 ? 0.0 : A * t * cos(arg);
					J[2] = A * cos(arg);
					for (int a = 0; a < 3; ++a)
					{
						Jtr[idx[a]] += J[a] * res;
						for (int b = 0; b < 3; ++b)
							JtJ[idx[a] * P + idx[b]] += J[a] * J[b];
					}
				}
			if (dOmega > 0.0)
				JtJ[1 * P + 1] = 1.0;
			// Cynex: Gauss eleme
			std::vector<double> M = JtJ, v = Jtr;
			bool bSingular = false;
			for (int i = 0; i < P && !bSingular; ++i)
			{
				int piv = i;
				for (int r2 = i + 1; r2 < P; ++r2)
					if (fabs(M[r2 * P + i]) > fabs(M[piv * P + i]))
						piv = r2;
				if (fabs(M[piv * P + i]) < 1e-30)
				{
					bSingular = true;
					break;
				}
				if (piv != i)
				{
					for (int c2 = 0; c2 < P; ++c2)
						std::swap(M[i * P + c2], M[piv * P + c2]);
					std::swap(v[i], v[piv]);
				}
				for (int r2 = i + 1; r2 < P; ++r2)
				{
					const double f = M[r2 * P + i] / M[i * P + i];
					for (int c2 = i; c2 < P; ++c2)
						M[r2 * P + c2] -= f * M[i * P + c2];
					v[r2] -= f * v[i];
				}
			}
			if (bSingular)
				break;
			std::vector<double> delta(P, 0.0);
			for (int i = P - 1; i >= 0; --i)
			{
				double acc = v[i];
				for (int c2 = i + 1; c2 < P; ++c2)
					acc -= M[i * P + c2] * delta[c2];
				delta[i] = acc / M[i * P + i];
			}
			A -= delta[0];
			w2 -= delta[1];
			for (unsigned int c = 0; c < uiClusters; ++c)
				vecPhase[c] -= delta[2 + c];
		}
		double maxErr2 = 0.0;
		for (unsigned int c = 0; c < uiClusters; ++c)
			for (int k = 0; k < K2; ++k)
				maxErr2 = std::max(maxErr2, fabs(A * sin(w2 * k * dt2 + vecPhase[c]) - vecRoll2[c][k]));
		d.dRockFitError = maxErr2;
	}

	double maxErr = d.dRockFitError;
	for (unsigned int c = 0; c < uiClusters; ++c)
	{
		d.vecClusters[c].fPhase = (float) vecPhase[c];
		for (int k = 0; k < K; ++k)
			maxErr = std::max(maxErr, fabs(A * sin(w2 * k * dt + vecPhase[c]) - vecRoll[c][k]));
	}
	d.dRockFitError = maxErr;
	if (maxErr > 2e-5 * std::max(1.0, A))
		Warn(d, "sallanma uyumu zayif (max %.3g rad)", maxErr);

	h.fRockAngle = (float) A;
	(void) block;
	CSpeedTreeRT::SetTime(0.0f);
}

///////////////////////////////////////////////////////////////////////
//	LOD gecis egrisi

struct SLodSample
{
	bool	bL0, bL1;
	float	fA0, fA1;
	int		iLod0, iLod1;
	bool	bB;
	float	fAB, fABB;
	bool	bBB;
};

static SLodSample SampleLod(CSpeedTreeRT* pTree, float f)
{
	SLodSample s;
	memset(&s, 0, sizeof(s));
	pTree->SetLodLevel(f);
	CSpeedTreeRT::SGeometry g;
	SafeGetGeometry(pTree, g, SpeedTree_LeafGeometry);
	s.bL0 = g.m_sLeaves0.m_bIsActive; s.fA0 = g.m_sLeaves0.m_fAlphaTestValue; s.iLod0 = g.m_sLeaves0.m_nDiscreteLodLevel;
	s.bL1 = g.m_sLeaves1.m_bIsActive; s.fA1 = g.m_sLeaves1.m_fAlphaTestValue; s.iLod1 = g.m_sLeaves1.m_nDiscreteLodLevel;
	CSpeedTreeRT::SGeometry g2;
	SafeGetGeometry(pTree, g2, SpeedTree_BranchGeometry);
	s.fAB = g2.m_fBranchAlphaTestValue;
	CSpeedTreeRT::SGeometry g3;
	SafeGetGeometry(pTree, g3, SpeedTree_BillboardGeometry);
	s.bBB = g3.m_sBillboard0.m_bIsActive; s.fABB = g3.m_sBillboard0.m_fAlphaTestValue;
	return s;
}

static double CurveHigh(double x, double t, double e)
{
	double u = t < 1.0 ? 1.0 - x / (1.0 - t) : 0.0;
	u = std::max(0.0, std::min(1.0, u));
	return pow(u, e);
}

static double CurveLow(double x, double t, double e)
{
	double u = t < 1.0 ? (x - t) / (1.0 - t) : 1.0;
	u = std::max(0.0, std::min(1.0, u));
	return pow(u, e);
}

// Cynex: (x, yuksek, dusuk) orneklerinden t ve e bul
static double FitCurve(const std::vector<double>& vx, const std::vector<double>& vh, const std::vector<double>& vl, double& tOut, double& eOut)
{
	double best = 1e30, bt = 0.0, be = 1.0;
	for (int it = 0; it <= 950; ++it)
	{
		const double t = it * 0.001;
		// Cynex: log(v) = e log(u) dogrusal en kucuk kareler
		double sxy = 0.0, sxx = 0.0;
		for (size_t i = 0; i < vx.size(); ++i)
		{
			const double uh = 1.0 - vx[i] / (1.0 - t);
			const double ul = (vx[i] - t) / (1.0 - t);
			if (uh > 0.02 && uh < 0.98 && vh[i] > 1e-4 && vh[i] < 1.0 - 1e-6)
			{
				sxy += log(uh) * log(vh[i]);
				sxx += log(uh) * log(uh);
			}
			if (ul > 0.02 && ul < 0.98 && vl[i] > 1e-4 && vl[i] < 1.0 - 1e-6)
			{
				sxy += log(ul) * log(vl[i]);
				sxx += log(ul) * log(ul);
			}
		}
		if (sxx <= 0.0)
			continue;
		const double e = sxy / sxx;
		double err = 0.0;
		for (size_t i = 0; i < vx.size(); ++i)
			err = std::max(err, std::max(fabs(CurveHigh(vx[i], t, e) - vh[i]), fabs(CurveLow(vx[i], t, e) - vl[i])));
		if (err < best)
		{
			best = err; bt = t; be = e;
		}
	}
	// Cynex: ince ayar
	double step = 0.0005;
	for (int pass = 0; pass < 40; ++pass)
	{
		bool bImproved = false;
		const double cand[4][2] = { { bt + step, be }, { bt - step, be }, { bt, be + step * 4 }, { bt, be - step * 4 } };
		for (int k = 0; k < 4; ++k)
		{
			const double t = cand[k][0], e = cand[k][1];
			if (t < 0.0 || t >= 0.99 || e <= 0.0)
				continue;
			double err = 0.0;
			for (size_t i = 0; i < vx.size(); ++i)
				err = std::max(err, std::max(fabs(CurveHigh(vx[i], t, e) - vh[i]), fabs(CurveLow(vx[i], t, e) - vl[i])));
			if (err < best)
			{
				best = err; bt = t; be = e; bImproved = true;
			}
		}
		if (!bImproved)
			step *= 0.5;
	}
	tOut = bt;
	eOut = be;
	return best;
}

// Cynex: dosyadaki float degerlerinden en yakini (oracle hassasiyetini birebir tutturmak icin)
static float SnapToFileFloat(const std::vector<unsigned char>& block, float fValue, float fTol)
{
	float fBest = fValue, fBestDiff = fTol;
	for (size_t i = 0; i + 4 <= block.size(); ++i)
	{
		float v;
		memcpy(&v, &block[i], 4);
		if (!(v == v))
			continue;
		const float diff = fabsf(v - fValue);
		if (diff < fBestDiff)
		{
			fBestDiff = diff;
			fBest = v;
		}
	}
	return fBest;
}

static void ExtractLodParams(STreeData& d, const std::vector<unsigned char>& block, CSpeedTreeRT* pA)
{
	CynexSpt::SHead& h = d.sHead;
	h.fLodRadius = 0.0f;
	h.fLodTransition = 0.0f;
	h.fLodExponent = 1.0f;
	h.usLeafLodMethod = CSpeedTreeRT::LOD_SMOOTH;

	const unsigned int nL = h.usLeafLods;
	const bool bBillboard = (h.uiFlags & CynexSpt::HEAD_HAS_BILLBOARD) != 0;

	// Cynex: yaprak LOD yontemi: sinirda L1 aktifse SMOOTH, sadece seviye degisiyorsa POP
	if (nL >= 2)
	{
		CSpeedTreeRT::SetDropToBillboard(false);
		const float fb = 1.0f - 1.0f / nL;
		const SLodSample sAt = SampleLod(pA, fb);
		const SLodSample sAbove = SampleLod(pA, fb + 0.01f);
		const SLodSample sBelow = SampleLod(pA, fb - 0.01f);
		if (sAt.bL1 || sAbove.bL1 || sBelow.bL1)
			h.usLeafLodMethod = CSpeedTreeRT::LOD_SMOOTH;
		else if (sAbove.iLod0 != sBelow.iLod0)
			h.usLeafLodMethod = CSpeedTreeRT::LOD_POP;
		else
			h.usLeafLodMethod = CSpeedTreeRT::LOD_NONE;
	}

	unsigned int uiCount = 0;
	bool bUseBillboard = false;
	if (nL >= 2 && h.usLeafLodMethod == CSpeedTreeRT::LOD_SMOOTH)
	{
		CSpeedTreeRT::SetDropToBillboard(false);
		uiCount = nL;
	}
	else if (bBillboard)
	{
		CSpeedTreeRT::SetDropToBillboard(true);
		uiCount = nL + 1;
		bUseBillboard = true;
	}
	else
		return;

	// Cynex: yaprak gecisi ilk sinirda (1 - 1/n), billboard gecisi son sinirda (1/n)
	const double fb = bUseBillboard ? 1.0 / uiCount : 1.0 - 1.0 / uiCount;
	// Cynex: ust uc: gecis bolgesi bitince dusuk seviye (L1 / billboard -> branch alfasi 84) kapanir
	auto InZone = [&](float f) -> bool
	{
		const SLodSample s = SampleLod(pA, f);
		if (bUseBillboard)
			return s.bBB;
		return s.bL1;
	};
	double lo = fb, hi = std::min(1.0, fb + 0.5 / uiCount);
	if (!InZone((float) lo))
	{
		Warn(d, "lod gecis bolgesi bulunamadi");
		CSpeedTreeRT::SetDropToBillboard(false);
		return;
	}
	if (InZone((float) hi))
	{
		Warn(d, "lod gecis yaricapi bant genisliginden buyuk");
		CSpeedTreeRT::SetDropToBillboard(false);
		return;
	}
	for (int i = 0; i < 40; ++i)
	{
		const double mid = 0.5 * (lo + hi);
		if (InZone((float) mid))
			lo = mid;
		else
			hi = mid;
	}
	double r = 0.5 * (lo + hi) - fb;
	r = SnapToFileFloat(block, (float) r, 2e-6f);

	std::vector<double> vx, vh, vl;
	for (int i = 0; i < 64; ++i)
	{
		const double x = (i + 0.5) / 64.0;
		const float f = (float) (fb - r + x * 2.0 * r);
		const SLodSample s = SampleLod(pA, f);
		double hv, lv;
		if (bUseBillboard)
		{
			hv = s.fAB;
			lv = s.bBB ? s.fABB : 255.0f;
		}
		else
		{
			hv = s.fA0;
			lv = s.bL1 ? s.fA1 : 255.0f;
		}
		vx.push_back((f - (fb - r)) / (2.0 * r));
		vh.push_back((hv - c_fTargetAlpha) / (255.0 - c_fTargetAlpha));
		vl.push_back((lv - c_fTargetAlpha) / (255.0 - c_fTargetAlpha));
	}
	double t = 0.0, e = 1.0;
	double err = FitCurve(vx, vh, vl, t, e);
	// Cynex: dosyadaki degere oturt, hata artmiyorsa kullan
	const float tSnap = SnapToFileFloat(block, (float) t, 2e-3f);
	const float eSnap = SnapToFileFloat(block, (float) e, 2e-3f * std::max(1.0f, (float) e));
	double errSnap = 0.0;
	for (size_t i = 0; i < vx.size(); ++i)
		errSnap = std::max(errSnap, std::max(fabs(CurveHigh(vx[i], tSnap, eSnap) - vh[i]), fabs(CurveLow(vx[i], tSnap, eSnap) - vl[i])));
	if (errSnap <= err + 1e-6)
	{
		t = tSnap; e = eSnap; err = errSnap;
	}
	d.dLodFitError = err * (255.0 - c_fTargetAlpha);
	if (d.dLodFitError > 0.01)
		Warn(d, "lod egri uyumu zayif (max %.4f alfa)", d.dLodFitError);
	h.fLodRadius = (float) r;
	h.fLodTransition = (float) t;
	h.fLodExponent = (float) e;
	CSpeedTreeRT::SetDropToBillboard(false);
	pA->SetLodLevel(1.0f);
}

///////////////////////////////////////////////////////////////////////
//	billboard

static void ExtractBillboard(STreeData& d, CSpeedTreeRT* pA)
{
	CynexSpt::SBillboard& b = d.sBillboard;
	memset(&b, 0, sizeof(b));
	CSpeedTreeRT::SetDropToBillboard(true);
	pA->SetLodLevel(0.0f);
	SetCameraDir(-1.0f, 0.0f, 0.0f);
	CSpeedTreeRT::SGeometry g;
	SafeGetGeometry(pA, g, SpeedTree_BillboardGeometry);
	if (g.m_sBillboard0.m_bIsActive && g.m_sBillboard0.m_pCoords && g.m_sBillboard0.m_pTexCoords)
	{
		d.sHead.uiFlags |= CynexSpt::HEAD_HAS_BILLBOARD;
		const float* c = g.m_sBillboard0.m_pCoords;
		// Cynex: az = pi: kose0 = (-sin(az) w, cos(az) w, ust)
		b.fHalfWidth = -c[1];
		b.fTop = c[2];
		b.fBottom = c[8];
		memcpy(b.afTexCoords, g.m_sBillboard0.m_pTexCoords, 8 * sizeof(float));
	}

	// Cynex: 360 derece: sektor j = [pi + j*2pi/N, pi + (j+1)*2pi/N); BB1 = cikan resim j, BB0 = giren resim j+1
	bool b360 = false;
	std::vector<std::vector<float> > vecImages;
	if (d.sHead.uiFlags & CynexSpt::HEAD_HAS_BILLBOARD)
	{
		const int iSteps = 720;
		for (int i = 0; i < iSteps; ++i)
		{
			const double a = i * 2.0 * M_PI / iSteps;
			SetCameraDir((float) cos(a), (float) sin(a), 0.0f);
			CSpeedTreeRT::SGeometry gg;
			SafeGetGeometry(pA, gg, SpeedTree_BillboardGeometry);
			if (gg.m_sBillboard1.m_bIsActive)
				b360 = true;
			const CSpeedTreeRT::SGeometry::SBillboard* apB[2] = { &gg.m_sBillboard0, &gg.m_sBillboard1 };
			for (int k = 0; k < 2; ++k)
			{
				if (!apB[k]->m_bIsActive || !apB[k]->m_pTexCoords)
					continue;
				bool bFound = false;
				for (size_t m = 0; m < vecImages.size() && !bFound; ++m)
					bFound = SameFloats(&vecImages[m][0], apB[k]->m_pTexCoords, 8);
				if (!bFound)
					vecImages.push_back(std::vector<float>(apB[k]->m_pTexCoords, apB[k]->m_pTexCoords + 8));
			}
		}
	}
	if (b360 && !vecImages.empty())
	{
		const size_t N = vecImages.size();
		d.sHead.uiFlags |= CynexSpt::HEAD_HAS_360;
		for (size_t j = 0; j < N; ++j)
		{
			const double a = M_PI + (j + 0.05) * 2.0 * M_PI / N;
			SetCameraDir((float) cos(a), (float) sin(a), 0.0f);
			CSpeedTreeRT::SGeometry gg;
			SafeGetGeometry(pA, gg, SpeedTree_BillboardGeometry);
			const float* tc = gg.m_sBillboard1.m_bIsActive && gg.m_sBillboard1.m_pTexCoords ? gg.m_sBillboard1.m_pTexCoords : &vecImages[j][0];
			if (!gg.m_sBillboard1.m_bIsActive)
				Warn(d, "360 billboard resmi %u okunamadi", (unsigned int) j);
			d.vec360TexCoords.insert(d.vec360TexCoords.end(), tc, tc + 8);
		}
		b.uiNum360 = (uint32_t) N;
		b.f360Offset = (float) M_PI;
	}

	// Cynex: yatay billboard: yukaridan bak
	SetCameraDir(0.0f, 0.0f, -1.0f);
	CSpeedTreeRT::SGeometry gh;
	SafeGetGeometry(pA, gh, SpeedTree_BillboardGeometry);
	if (gh.m_sHorizontalBillboard.m_bIsActive && gh.m_sHorizontalBillboard.m_pCoords && gh.m_sHorizontalBillboard.m_pTexCoords)
	{
		d.sHead.uiFlags |= CynexSpt::HEAD_HAS_HORIZONTAL;
		memcpy(b.afHorizontalCoords, gh.m_sHorizontalBillboard.m_pCoords, 12 * sizeof(float));
		memcpy(b.afHorizontalTexCoords, gh.m_sHorizontalBillboard.m_pTexCoords, 8 * sizeof(float));
	}
	SetCameraDir(-1.0f, 0.0f, 0.0f);
	CSpeedTreeRT::SetDropToBillboard(false);
	pA->SetLodLevel(1.0f);
}

///////////////////////////////////////////////////////////////////////
//	tek dosya donusumu

static void WriteIndexed(CWriter& w, const SIndexedData& s)
{
	w.U32(s.uiVertexCount);
	w.U32(s.uiMask);
	const unsigned int n = s.uiVertexCount;
	if (s.uiMask & CynexSpt::ARRAY_COORDS)		w.Floats(&s.vecCoords[0], n * 3);
	if (s.uiMask & CynexSpt::ARRAY_NORMALS)		w.Floats(&s.vecNormals[0], n * 3);
	if (s.uiMask & CynexSpt::ARRAY_BINORMALS)	w.Floats(&s.vecBinormals[0], n * 3);
	if (s.uiMask & CynexSpt::ARRAY_TANGENTS)	w.Floats(&s.vecTangents[0], n * 3);
	if (s.uiMask & CynexSpt::ARRAY_TEXCOORDS0)	w.Floats(&s.vecTex0[0], n * 2);
	if (s.uiMask & CynexSpt::ARRAY_TEXCOORDS1)	w.Floats(&s.vecTex1[0], n * 2);
	if (s.uiMask & CynexSpt::ARRAY_COLORS)		w.Raw(&s.vecColors[0], n * 4);
	if (s.uiMask & CynexSpt::ARRAY_WIND_WEIGHTS)	w.Floats(&s.vecWindWeights[0], n);
	if (s.uiMask & CynexSpt::ARRAY_WIND_INDICES)
	{
		w.Raw(&s.vecWindIndices[0], n);
		w.Align4();
	}
	if (s.uiMask & CynexSpt::ARRAY_FROND_LOCAL_UV)
	{
		w.Floats(&s.vecLocalUV[0], n * 2);
		w.Raw(&s.vecFrondMap[0], n);
		w.Align4();
	}
	w.U32((uint32_t) s.vecLodStrips.size());
	for (size_t l = 0; l < s.vecLodStrips.size(); ++l)
	{
		w.U32((uint32_t) s.vecLodStrips[l].size());
		for (size_t k = 0; k < s.vecLodStrips[l].size(); ++k)
		{
			w.U32((uint32_t) s.vecLodStrips[l][k].size());
			if (!s.vecLodStrips[l][k].empty())
				w.Raw(&s.vecLodStrips[l][k][0], s.vecLodStrips[l][k].size() * 2);
			w.Align4();
		}
	}
	w.U32((uint32_t) s.vecLodMapIndexes.size());
	for (size_t l = 0; l < s.vecLodMapIndexes.size(); ++l)
	{
		w.U32((uint32_t) s.vecLodMapIndexes[l].size());
		if (!s.vecLodMapIndexes[l].empty())
			w.Raw(&s.vecLodMapIndexes[l][0], s.vecLodMapIndexes[l].size());
		w.Align4();
	}
}

static void Serialize(const STreeData& d, std::vector<unsigned char>& out)
{
	CWriter w;
	w.Raw(CynexSpt::c_szMagic, 8);
	w.U32(CynexSpt::c_uiVersion);
	w.U32(0);

	size_t c = w.BeginChunk(CynexSpt::TAG_HEAD);
	w.Raw(&d.sHead, sizeof(d.sHead));
	w.U32((uint32_t) d.vecLeafLodSizes.size());
	w.Floats(d.vecLeafLodSizes.empty() ? NULL : &d.vecLeafLodSizes[0], d.vecLeafLodSizes.size());
	w.EndChunk(c);

	c = w.BeginChunk(CynexSpt::TAG_TEXS);
	w.String(d.strBranchTexture.c_str());
	w.String(d.bHasComposite ? d.strComposite.c_str() : NULL);
	w.String(d.bHasSelfShadow ? d.strSelfShadow.c_str() : NULL);
	w.String(d.bHasUserData ? d.strUserData.c_str() : NULL);
	w.U16((uint16_t) d.vecLeafTextures.size());
	for (size_t i = 0; i < d.vecLeafTextures.size(); ++i)
		w.String(d.vecLeafTextures[i].c_str());
	w.U16((uint16_t) d.vecFrondTextures.size());
	for (size_t i = 0; i < d.vecFrondTextures.size(); ++i)
		w.String(d.vecFrondTextures[i].c_str());
	w.EndChunk(c);

	c = w.BeginChunk(CynexSpt::TAG_COLL);
	w.U32((uint32_t) d.vecCollisions.size());
	if (!d.vecCollisions.empty())
		w.Raw(&d.vecCollisions[0], d.vecCollisions.size() * sizeof(CynexSpt::SCollision));
	w.EndChunk(c);

	c = w.BeginChunk(CynexSpt::TAG_BRCH);
	WriteIndexed(w, d.sBranches);
	w.EndChunk(c);

	c = w.BeginChunk(CynexSpt::TAG_FRND);
	WriteIndexed(w, d.sFronds);
	w.EndChunk(c);

	c = w.BeginChunk(CynexSpt::TAG_LEAF);
	w.U32((uint32_t) d.vecClusters.size());
	if (!d.vecClusters.empty())
		w.Raw(&d.vecClusters[0], d.vecClusters.size() * sizeof(CynexSpt::SLeafCluster));
	w.U32((uint32_t) (d.vecLeafMapTexCoords.size() / 8));
	w.Floats(d.vecLeafMapTexCoords.empty() ? NULL : &d.vecLeafMapTexCoords[0], d.vecLeafMapTexCoords.size());
	{
		std::vector<uint8_t> vecNoFlip(d.vecLeafMapTexCoords.size() / 8, 0);
		for (size_t m = 0; m < vecNoFlip.size() && m < d.vecLeafMapNoFlip.size(); ++m)
			vecNoFlip[m] = d.vecLeafMapNoFlip[m];
		if (!vecNoFlip.empty())
			w.Raw(&vecNoFlip[0], vecNoFlip.size());
		w.Align4();
	}
	w.U32((uint32_t) d.vecLeafLods.size());
	for (size_t l = 0; l < d.vecLeafLods.size(); ++l)
	{
		const SLeafLod& s = d.vecLeafLods[l];
		const unsigned int n = s.uiCount;
		w.U32(n);
		w.U32(s.uiMask);
		if (!n)
			continue;
		w.Raw(&s.vecMapIndices[0], n);
		w.Align4();
		w.Raw(&s.vecClusterIndices[0], n);
		w.Align4();
		w.Floats(&s.vecCenters[0], n * 3);
		if (s.uiMask & CynexSpt::LEAF_COLORS_STATIC)	w.Raw(&s.vecColorsStatic[0], n * 4);
		if (s.uiMask & CynexSpt::LEAF_COLORS_DYNAMIC)	w.Raw(&s.vecColorsDynamic[0], n * 4);
		if (s.uiMask & CynexSpt::LEAF_NORMALS)		w.Floats(&s.vecNormals[0], n * 3);
		if (s.uiMask & CynexSpt::LEAF_BINORMALS)	w.Floats(&s.vecBinormals[0], n * 3);
		if (s.uiMask & CynexSpt::LEAF_TANGENTS)		w.Floats(&s.vecTangents[0], n * 3);
		if (s.uiMask & CynexSpt::LEAF_WIND_WEIGHTS)	w.Floats(&s.vecWindWeights[0], n);
		if (s.uiMask & CynexSpt::LEAF_WIND_INDICES)
		{
			w.Raw(&s.vecWindIndices[0], n);
			w.Align4();
		}
	}
	w.EndChunk(c);

	c = w.BeginChunk(CynexSpt::TAG_BILL);
	w.Raw(&d.sBillboard, sizeof(d.sBillboard));
	w.Floats(d.vec360TexCoords.empty() ? NULL : &d.vec360TexCoords[0], d.vec360TexCoords.size());
	w.EndChunk(c);

	const uint32_t uiChunks = 7;
	memcpy(&w.Data()[12], &uiChunks, 4);
	out.swap(w.Data());
}

static void Trace(const char* pStep)
{
	static const bool s_bTrace = getenv("SPTCONV_TRACE") != NULL;
	if (s_bTrace)
	{
		fprintf(stderr, "[sptconv] %s\n", pStep);
		fflush(stderr);
	}
}

static int ConvertOne(const fs::path& inPath, const fs::path& outPath)
{
	std::vector<unsigned char> block;
	if (!ReadWholeFile(inPath, block))
	{
		printf("FAIL read\n");
		return 3;
	}
	if (block.size() >= 8 && memcmp(&block[0], CynexSpt::c_szMagic, 8) == 0)
	{
		printf("FAIL already converted\n");
		return 4;
	}
	// Cynex: eski donusum kalmasin (oracle basarisizsa cikti olmamali); yerinde donusumde dokunma
	{
		std::error_code ec;
		if (!fs::equivalent(inPath, outPath, ec))
			fs::remove(outPath, ec);
	}

	InitOracleGlobals();
	STreeData d;
	memset(&d.sHead, 0, sizeof(d.sHead));
	memcpy(d.sHead.afConvertLight, c_afGameLight, sizeof(c_afGameLight));

	// Cynex: dosya varsayilanlari (setter oncesi)
	{
		CSpeedTreeRT* pProbe = new CSpeedTreeRT;
		if (!pProbe->LoadTree(&block[0], (unsigned int) block.size()))
		{
			printf("FAIL oracle LoadTree: %s\n", CSpeedTreeRT::GetCurrentError());
			return 2;
		}
		CynexSpt::SHead& h = d.sHead;
		h.iBranchLighting = pProbe->GetBranchLightingMethod();
		h.iLeafLighting = pProbe->GetLeafLightingMethod();
		h.iFrondLighting = pProbe->GetFrondLightingMethod();
		h.iStaticLightingStyle = pProbe->GetStaticLightingStyle();
		h.iBranchWind = pProbe->GetBranchWindMethod();
		h.iLeafWind = pProbe->GetLeafWindMethod();
		h.iFrondWind = pProbe->GetFrondWindMethod();
		h.iLeafRocking = pProbe->GetLeafRockingState() ? 1 : 0;
		h.fLeafLightingAdjustment = pProbe->GetLeafLightingAdjustment();
		pProbe->GetLodLimits(h.fLodNear, h.fLodFar);
		pProbe->GetTreeSize(h.fTreeSize, h.fTreeSizeVariance);
		h.uiSeed = pProbe->GetSeed();
		memcpy(h.afBranchMaterial, pProbe->GetBranchMaterial(), 13 * sizeof(float));
		memcpy(h.afLeafMaterial, pProbe->GetLeafMaterial(), 13 * sizeof(float));
		memcpy(h.afFrondMaterial, pProbe->GetFrondMaterial(), 13 * sizeof(float));
	}

	std::string strError;
	CSpeedTreeRT* pA = MakeOracleTree(block, CSpeedTreeRT::LIGHT_STATIC, CSpeedTreeRT::WIND_NONE, strError);
	if (!pA)
	{
		printf("FAIL oracle %s\n", strError.c_str());
		return 2;
	}
	Trace("tree B");
	// Cynex: B flip acik: hangi texcoord'larin dll'de cevrildigini anlamak icin
	CSpeedTreeRT::SetTextureFlip(true);
	CSpeedTreeRT* pB = MakeOracleTree(block, CSpeedTreeRT::LIGHT_DYNAMIC, CSpeedTreeRT::WIND_GPU, strError);
	CSpeedTreeRT::SetTextureFlip(false);
	if (!pB)
	{
		printf("FAIL oracle (dinamik) %s\n", strError.c_str());
		return 2;
	}

	CynexSpt::SHead& h = d.sHead;
	h.usBranchLods = pA->GetNumBranchLodLevels();
	h.usFrondLods = pA->GetNumFrondLodLevels();
	h.usLeafLods = pA->GetNumLeafLodLevels();
	pA->GetBoundingBox(h.afBoundingBox);

	CSpeedTreeRT::STextures tex;
	pA->GetTextures(tex);
	d.strBranchTexture = tex.m_pBranchTextureFilename ? tex.m_pBranchTextureFilename : "";
	d.bHasComposite = tex.m_pCompositeFilename != NULL;
	d.strComposite = d.bHasComposite ? tex.m_pCompositeFilename : "";
	d.bHasSelfShadow = tex.m_pSelfShadowFilename != NULL;
	d.strSelfShadow = d.bHasSelfShadow ? tex.m_pSelfShadowFilename : "";
	for (unsigned int i = 0; i < tex.m_uiLeafTextureCount; ++i)
		d.vecLeafTextures.push_back(tex.m_pLeafTextureFilenames[i] ? tex.m_pLeafTextureFilenames[i] : "");
	for (unsigned int i = 0; i < tex.m_uiFrondTextureCount; ++i)
		d.vecFrondTextures.push_back(tex.m_pFrondTextureFilenames[i] ? tex.m_pFrondTextureFilenames[i] : "");
	const char* pUser = pA->GetUserData();
	d.bHasUserData = pUser != NULL;
	d.strUserData = pUser ? pUser : "";

	for (unsigned int i = 0; i < pA->GetCollisionObjectCount(); ++i)
	{
		CynexSpt::SCollision co;
		memset(&co, 0, sizeof(co));
		CSpeedTreeRT::ECollisionObjectType eType = CSpeedTreeRT::CO_SPHERE;
		pA->GetCollisionObject(i, eType, co.afPosition, co.afDimensions);
		co.iType = eType;
		d.vecCollisions.push_back(co);
	}

	const float* pSizes = pA->GetLeafLodSizeAdjustments();
	for (unsigned int i = 0; i < h.usLeafLods; ++i)
		d.vecLeafLodSizes.push_back(pSizes ? pSizes[i] : 1.0f);

	Trace("ExtractIndexed");
	ExtractIndexed(d, pA, pB, false, d.sBranches);
	Trace("ExtractIndexed");
	ExtractIndexed(d, pA, pB, true, d.sFronds);
	Trace("ExtractFrondLocalUV");
	ExtractFrondLocalUV(d, inPath, outPath, d.sFronds, (unsigned int) d.vecFrondTextures.size());
	Trace("ExtractLeaves");
	ExtractLeaves(d, pA, pB);
	Trace("ExtractBillboard");
	ExtractBillboard(d, pA);
	Trace("ExtractLodParams");
	ExtractLodParams(d, block, pA);
	Trace("ExtractClustersAndRocking");
	ExtractClustersAndRocking(d, block, pA);

	std::vector<unsigned char> out;
	Serialize(d, out);
	if (!WriteWholeFile(outPath, out))
	{
		printf("FAIL write\n");
		return 3;
	}

	unsigned int uiLeaves = 0;
	for (size_t l = 0; l < d.vecLeafLods.size(); ++l)
		uiLeaves += d.vecLeafLods[l].uiCount;
	printf("OK in=%u out=%u lods=%u/%u/%u bv=%u fv=%u leaves=%u clusters=%u flags=%u rock=%.4f/%.4f s0=%.4f lod=%.5f/%.5f/%.5f lodfit=%.5f rockfit=%.3g warn=%u\n",
		(unsigned int) block.size(), (unsigned int) out.size(), h.usBranchLods, h.usFrondLods, h.usLeafLods,
		d.sBranches.uiVertexCount, d.sFronds.uiVertexCount, uiLeaves, (unsigned int) d.vecClusters.size(), h.uiFlags,
		h.fRockAngle, h.fRockFrequency, h.fWindStrengthInitial, h.fLodRadius, h.fLodTransition, h.fLodExponent,
		d.dLodFitError, d.dRockFitError, (unsigned int) d.vecWarnings.size());
	for (size_t i = 0; i < d.vecWarnings.size(); ++i)
		printf("WARN %s\n", d.vecWarnings[i].c_str());
	fflush(stdout);
	return 0;
}

///////////////////////////////////////////////////////////////////////
//	toplu donusum (her dosya alt surecte)

struct SJob
{
	fs::path			inPath;
	fs::path			outPath;
	PROCESS_INFORMATION	pi;
	HANDLE				hRead;
	std::string			strOutput;
	bool				bRunning;
	int					iAttempts;
};

static bool StartJob(const std::wstring& strExe, SJob& job)
{
	SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
	HANDLE hRead = NULL, hWrite = NULL;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
		return false;
	SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);
	STARTUPINFOW si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = hWrite;
	si.hStdError = hWrite;
	si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	std::wstring cmd = L"\"" + strExe + L"\" --one \"" + job.inPath.wstring() + L"\" \"" + job.outPath.wstring() + L"\"";
	std::vector<wchar_t> buf(cmd.begin(), cmd.end());
	buf.push_back(0);
	memset(&job.pi, 0, sizeof(job.pi));
	// Cynex: dll cokerse WER penceresi acilmasin
	const BOOL ok = CreateProcessW(NULL, &buf[0], NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &job.pi);
	CloseHandle(hWrite);
	if (!ok)
	{
		CloseHandle(hRead);
		return false;
	}
	job.hRead = hRead;
	job.bRunning = true;
	return true;
}

static void DrainJob(SJob& job)
{
	char buf[4096];
	DWORD avail = 0;
	while (PeekNamedPipe(job.hRead, NULL, 0, NULL, &avail, NULL) && avail > 0)
	{
		DWORD got = 0;
		if (!ReadFile(job.hRead, buf, std::min<DWORD>(avail, sizeof(buf)), &got, NULL) || got == 0)
			break;
		job.strOutput.append(buf, got);
	}
}

static void PrintUsage()
{
	printf("kullanim: sptconv <girdi klasoru|.spt> <cikti klasoru> [-j N]\n");
	printf("          sptconv --one <girdi.spt> <cikti.spt>\n");
}

int wmain(int argc, wchar_t** argv)
{
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
	if (argc >= 5 && wcscmp(argv[1], L"--fronduv") == 0)
	{
		const int ret = FrondUVChild(argv[2], argv[3], (unsigned int) _wtoi(argv[4]));
		TerminateProcess(GetCurrentProcess(), (UINT) ret);
		return ret;
	}
	if (argc >= 4 && wcscmp(argv[1], L"--one") == 0)
	{
		const int ret = ConvertOne(argv[2], argv[3]);
		fflush(stdout);
		// Cynex: dll agac silerken/cikista cokebiliyor; yikicilari calistirmadan cik
		TerminateProcess(GetCurrentProcess(), (UINT) ret);
		return ret;
	}
	if (argc < 3)
	{
		PrintUsage();
		return 1;
	}

	const fs::path inRoot = argv[1];
	const fs::path outRoot = argv[2];
	int iJobs = 0;
	for (int i = 3; i < argc; ++i)
		if (wcscmp(argv[i], L"-j") == 0 && i + 1 < argc)
			iJobs = _wtoi(argv[++i]);
	if (iJobs <= 0)
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		iJobs = std::max<int>(1, (int) si.dwNumberOfProcessors);
	}

	std::vector<SJob> vecJobs;
	std::error_code ec;
	if (fs::is_regular_file(inRoot, ec))
	{
		SJob job = {};
		job.inPath = inRoot;
		job.outPath = outRoot / inRoot.filename();
		vecJobs.push_back(job);
	}
	else
	{
		for (fs::recursive_directory_iterator it(inRoot, fs::directory_options::skip_permission_denied, ec), end; it != end; it.increment(ec))
		{
			if (ec)
				break;
			if (!it->is_regular_file(ec))
				continue;
			std::wstring ext = it->path().extension().wstring();
			std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
			if (ext != L".spt")
				continue;
			SJob job = {};
			job.inPath = it->path();
			job.outPath = outRoot / fs::relative(it->path(), inRoot, ec);
			vecJobs.push_back(job);
		}
	}
	if (vecJobs.empty())
	{
		printf("hic .spt bulunamadi\n");
		return 1;
	}

	wchar_t szExe[MAX_PATH * 4];
	GetModuleFileNameW(NULL, szExe, (DWORD) (sizeof(szExe) / sizeof(szExe[0])));
	const DWORD dwStart = GetTickCount();

	size_t next = 0, done = 0;
	std::vector<size_t> vecActive;
	long long llIn = 0, llOut = 0;
	size_t uiOk = 0, uiOracleFail = 0, uiCrash = 0, uiOther = 0, uiWarned = 0;
	std::map<std::string, size_t> kFeatures;
	std::vector<std::string> vecReport;

	while (done < vecJobs.size())
	{
		while (vecActive.size() < (size_t) iJobs && next < vecJobs.size())
		{
			if (StartJob(szExe, vecJobs[next]))
				vecActive.push_back(next);
			else
			{
				vecReport.push_back("BASLATILAMADI " + ToUtf8(vecJobs[next].inPath.wstring()));
				++uiOther;
				++done;
			}
			++next;
		}
		std::vector<HANDLE> vecHandles;
		for (size_t i = 0; i < vecActive.size(); ++i)
			vecHandles.push_back(vecJobs[vecActive[i]].pi.hProcess);
		const DWORD w = WaitForMultipleObjects((DWORD) vecHandles.size(), &vecHandles[0], FALSE, 50);
		for (size_t i = 0; i < vecActive.size(); ++i)
			DrainJob(vecJobs[vecActive[i]]);
		if (w == WAIT_TIMEOUT)
			continue;
		for (size_t i = 0; i < vecActive.size();)
		{
			SJob& job = vecJobs[vecActive[i]];
			if (WaitForSingleObject(job.pi.hProcess, 0) != WAIT_OBJECT_0)
			{
				++i;
				continue;
			}
			DrainJob(job);
			DWORD code = 0;
			GetExitCodeProcess(job.pi.hProcess, &code);
			CloseHandle(job.pi.hProcess);
			CloseHandle(job.pi.hThread);
			CloseHandle(job.hRead);
			const std::string strName = ToUtf8(fs::relative(job.inPath, fs::is_regular_file(inRoot, ec) ? inRoot.parent_path() : inRoot, ec).wstring());
			const std::string& o = job.strOutput;
			if (code == 0 && o.compare(0, 2, "OK") == 0)
			{
				++uiOk;
				unsigned int in = 0, out = 0, flags = 0, bv = 0, fv = 0, leaves = 0, warn = 0;
				const char* p = o.c_str();
				sscanf(p, "OK in=%u out=%u", &in, &out);
				const char* q;
				if ((q = strstr(p, "bv=")) != NULL) sscanf(q, "bv=%u fv=%u leaves=%u", &bv, &fv, &leaves);
				if ((q = strstr(p, "flags=")) != NULL) sscanf(q, "flags=%u", &flags);
				if ((q = strstr(p, "warn=")) != NULL) sscanf(q, "warn=%u", &warn);
				llIn += in; llOut += out;
				if (fv) ++kFeatures["frond"];
				if (leaves) ++kFeatures["yaprak"];
				if (flags & CynexSpt::HEAD_HAS_BILLBOARD) ++kFeatures["billboard"];
				if (flags & CynexSpt::HEAD_HAS_360) ++kFeatures["billboard360"];
				if (flags & CynexSpt::HEAD_HAS_HORIZONTAL) ++kFeatures["yatay billboard"];
				if (warn)
				{
					++uiWarned;
					vecReport.push_back("UYARI " + strName + "\n" + o);
				}
			}
			else if (code == 2)
			{
				++uiOracleFail;
				vecReport.push_back("ORACLE " + strName + ": " + o.substr(0, o.find('\n')));
			}
			else if (o.compare(0, 4, "FAIL") == 0)
			{
				++uiOther;
				vecReport.push_back("HATA " + strName + ": " + o.substr(0, o.find('\n')));
			}
			else if (job.iAttempts < 2)
			{
				// Cynex: dll heap bozulmasi ortama bagli; ayni dosyayi yeniden dene
				++job.iAttempts;
				job.strOutput.clear();
				vecActive.erase(vecActive.begin() + i);
				if (StartJob(szExe, job))
					vecActive.push_back(&job - &vecJobs[0]);
				else
				{
					++uiCrash;
					++done;
					vecReport.push_back("BASLATILAMADI " + strName);
				}
				continue;
			}
			else
			{
				++uiCrash;
				char sz[32];
				sprintf(sz, "0x%08lX", code);
				vecReport.push_back(std::string("COKTU ") + strName + " (" + sz + ")");
			}
			++done;
			if ((done % 25) == 0 || done == vecJobs.size())
			{
				printf("\r%zu / %zu", done, vecJobs.size());
				fflush(stdout);
			}
			vecActive.erase(vecActive.begin() + i);
		}
	}
	printf("\n");

	std::string strSummary;
	char line[512];
	sprintf(line, "sptconv ozeti: %zu dosya, %zu donustu, %zu oracle hatasi (orijinal de yuklenemiyor), %zu coktu, %zu diger hata, %zu uyarili\n",
		vecJobs.size(), uiOk, uiOracleFail, uiCrash, uiOther, uiWarned);
	strSummary += line;
	sprintf(line, "boyut: girdi %.1f KB -> cikti %.1f KB, sure %.1f sn\n", llIn / 1024.0, llOut / 1024.0, (GetTickCount() - dwStart) / 1000.0);
	strSummary += line;
	for (std::map<std::string, size_t>::const_iterator it = kFeatures.begin(); it != kFeatures.end(); ++it)
	{
		sprintf(line, "  %s: %zu\n", it->first.c_str(), it->second);
		strSummary += line;
	}
	printf("%s", strSummary.c_str());
	for (size_t i = 0; i < vecReport.size(); ++i)
		if (vecReport[i].compare(0, 5, "UYARI") != 0)
			printf("%s\n", vecReport[i].c_str());

	std::string strFull = strSummary;
	for (size_t i = 0; i < vecReport.size(); ++i)
		strFull += vecReport[i] + "\n";
	std::vector<unsigned char> vecSummary(strFull.begin(), strFull.end());
	WriteWholeFile(outRoot / "sptconv_summary.txt", vecSummary);
	return (uiCrash || uiOther) ? 2 : 0;
}
