// Cynex: gr2conv - Granny 2.11 .gr2 donusturucu (mobil paket)
// Cynex: cikti = sikistirmasiz, little-endian, 64bit pointer duzeni; egriler DaK32fC32f + sabit/birim
#include <windows.h>
#include <granny.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct SOptions
{
	fs::path	inDir;
	fs::path	outDir;
	int			iPointerBits = 64;
	bool		bNormalizeCurves = true;
	bool		bStrip = false;
	bool		bStatsOnly = false;
	bool		bVerify = true;
	bool		bVerbose = false;
	bool		bOverwrite = true;
};

typedef std::map<std::string, long long> TCounter;

struct SSummary
{
	long long	llFiles = 0;
	long long	llConverted = 0;
	long long	llFailed = 0;
	long long	llSkipped = 0;
	long long	llBytesIn = 0;
	long long	llBytesOut = 0;
	TCounter	kStats;
	std::vector<std::string> vecFailures;
};

static std::string g_strLastGrannyError;

static void GrannyLogToString(granny_log_message_type Type, granny_log_message_origin Origin,
	char const* File, granny_int32x Line, char const* Message, void* UserData)
{
	(void) Origin; (void) File; (void) Line; (void) UserData;
	if (Type == GrannyErrorLogMessage || Type == GrannyWarningLogMessage)
	{
		if (!g_strLastGrannyError.empty())
			g_strLastGrannyError += " | ";
		g_strLastGrannyError += Message ? Message : "?";
	}
}

static std::string ToUtf8(const fs::path& path)
{
	const std::wstring w = path.wstring();
	if (w.empty())
		return std::string();
	const int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int) w.size(), NULL, 0, NULL, NULL);
	std::string s(n, '\0');
	WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int) w.size(), &s[0], n, NULL, NULL);
	return s;
}

static bool ReadWholeFile(const fs::path& path, std::vector<granny_uint8>& out)
{
	FILE* fp = _wfopen(path.c_str(), L"rb");
	if (!fp)
		return false;
	fseek(fp, 0, SEEK_END);
	const long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (size < 0)
	{
		fclose(fp);
		return false;
	}
	out.resize((size_t) size);
	const size_t got = size ? fread(out.data(), 1, (size_t) size, fp) : 0;
	fclose(fp);
	return got == (size_t) size;
}

static bool WriteWholeFile(const fs::path& path, const granny_uint8* data, size_t size)
{
	std::error_code ec;
	fs::create_directories(path.parent_path(), ec);
	FILE* fp = _wfopen(path.c_str(), L"wb");
	if (!fp)
		return false;
	const size_t put = size ? fwrite(data, 1, size, fp) : 0;
	fclose(fp);
	return put == size;
}

/////////////////////////////////////////////////////////////////////////////
// Cynex: egri tipleri

struct SCurveTypeName
{
	granny_data_type_definition** ppType;
	const char* szName;
};

static const SCurveTypeName gs_akCurveTypes[] =
{
	{ &GrannyCurveDataDaKeyframes32fType,	"DaKeyframes32f" },
	{ &GrannyCurveDataDaK32fC32fType,		"DaK32fC32f" },
	{ &GrannyCurveDataDaK16uC16uType,		"DaK16uC16u" },
	{ &GrannyCurveDataDaK8uC8uType,			"DaK8uC8u" },
	{ &GrannyCurveDataD3K16uC16uType,		"D3K16uC16u" },
	{ &GrannyCurveDataD3K8uC8uType,			"D3K8uC8u" },
	{ &GrannyCurveDataD4nK16uC15uType,		"D4nK16uC15u" },
	{ &GrannyCurveDataD4nK8uC7uType,		"D4nK8uC7u" },
	{ &GrannyCurveDataDaIdentityType,		"DaIdentity" },
	{ &GrannyCurveDataDaConstant32fType,	"DaConstant32f" },
	{ &GrannyCurveDataD3Constant32fType,	"D3Constant32f" },
	{ &GrannyCurveDataD4Constant32fType,	"D4Constant32f" },
	{ &GrannyCurveDataD9I1K16uC16uType,		"D9I1K16uC16u" },
	{ &GrannyCurveDataD9I3K16uC16uType,		"D9I3K16uC16u" },
	{ &GrannyCurveDataD9I1K8uC8uType,		"D9I1K8uC8u" },
	{ &GrannyCurveDataD9I3K8uC8uType,		"D9I3K8uC8u" },
	{ &GrannyCurveDataD3I1K32fC32fType,		"D3I1K32fC32f" },
	{ &GrannyCurveDataD3I1K16uC16uType,		"D3I1K16uC16u" },
	{ &GrannyCurveDataD3I1K8uC8uType,		"D3I1K8uC8u" },
};

static const char* GetCurveTypeName(const granny_curve2& curve)
{
	if (!curve.CurveData.Type || !curve.CurveData.Object)
		return "Empty";
	for (const SCurveTypeName& r : gs_akCurveTypes)
		if (GrannyDataTypesAreEqual(*r.ppType, curve.CurveData.Type))
			return r.szName;
	return "Unknown";
}

// Cynex: runtime'in bildigi egri formatlari
static bool IsRuntimeCurve(const granny_curve2& curve)
{
	if (!curve.CurveData.Type || !curve.CurveData.Object)
		return true;
	const char* szName = GetCurveTypeName(curve);
	return !strcmp(szName, "DaK32fC32f") || !strcmp(szName, "DaKeyframes32f") || !strcmp(szName, "DaIdentity")
		|| !strcmp(szName, "DaConstant32f") || !strcmp(szName, "D3Constant32f")
		|| !strcmp(szName, "D4Constant32f");
}

static std::string MemberTypeName(granny_member_type eType)
{
	static const char* s_aszNames[] =
	{
		"End", "Inline", "Ref", "RefArray", "ArrayOfRefs", "Variant", "Unsupported", "RefVariantArray",
		"String", "Transform", "Real32", "Int8", "UInt8", "BinormalInt8", "NormalUInt8", "Int16", "UInt16",
		"BinormalInt16", "NormalUInt16", "Int32", "UInt32", "Real16", "EmptyRef",
	};
	if ((int) eType >= 0 && (int) eType < (int) (sizeof(s_aszNames) / sizeof(s_aszNames[0])))
		return s_aszNames[eType];
	return "Type" + std::to_string((int) eType);
}

static std::string TypeSignature(const granny_data_type_definition* pType)
{
	std::string s;
	if (!pType)
		return "null";
	for (const granny_data_type_definition* p = pType; p->Type != GrannyEndMember; ++p)
	{
		if (!s.empty())
			s += ",";
		s += p->Name ? p->Name : "?";
		s += ":" + MemberTypeName(p->Type);
		if (p->ArrayWidth > 1)
			s += "[" + std::to_string(p->ArrayWidth) + "]";
	}
	return s;
}

/////////////////////////////////////////////////////////////////////////////
// Cynex: istatistik

static void CountCurve(TCounter& kStats, const char* szKind, const granny_curve2& curve)
{
	char sz[160];
	const int degree = (curve.CurveData.Type && curve.CurveData.Object) ? GrannyCurveGetDegree(&curve) : -1;
	snprintf(sz, sizeof(sz), "curve.%s.%s.deg%d", szKind, GetCurveTypeName(curve), degree);
	++kStats[sz];
}

static void CollectStats(TCounter& kStats, granny_file* pFile, granny_file_info* pInfo)
{
	char sz[512];

	snprintf(sz, sizeof(sz), "file.version=%u", pFile->Header->Version);
	++kStats[sz];
	snprintf(sz, sizeof(sz), "file.typetag=0x%08X", pFile->Header->TypeTag);
	++kStats[sz];

	const granny_grn_section* pSections = GrannyGetGRNSectionArray(pFile->Header);
	for (granny_uint32 i = 0; i < pFile->Header->SectionArrayCount; ++i)
	{
		if (pSections[i].DataSize == 0)
			continue;
		snprintf(sz, sizeof(sz), "section.format=%u", pSections[i].Format);
		++kStats[sz];
	}

	if (pInfo->ArtToolInfo && pInfo->ArtToolInfo->FromArtToolName)
	{
		snprintf(sz, sizeof(sz), "arttool=%s", pInfo->ArtToolInfo->FromArtToolName);
		++kStats[sz];
	}

	for (int m = 0; m < pInfo->ModelCount; ++m)
	{
		const granny_model* pModel = pInfo->Models[m];
		++kStats["model"];
		for (int b = 0; b < pModel->MeshBindingCount; ++b)
		{
			const granny_mesh* pMesh = pModel->MeshBindings[b].Mesh;
			const bool isRigid = GrannyMeshIsRigid(pMesh);
			snprintf(sz, sizeof(sz), "vertex.%s.{%s}", isRigid ? "rigid" : "deform",
				TypeSignature(GrannyGetMeshVertexType(pMesh)).c_str());
			++kStats[sz];
			if (pMesh->MorphTargetCount > 0)
				++kStats["mesh.morphtargets"];
			if (pMesh->PrimaryTopology)
			{
				if (pMesh->PrimaryTopology->Index16Count > 0)
					++kStats["mesh.indices16"];
				if (pMesh->PrimaryTopology->IndexCount > 0)
					++kStats["mesh.indices32"];
			}
			if (pMesh->ExtendedData.Type)
			{
				snprintf(sz, sizeof(sz), "mesh.ext={%s}", TypeSignature(pMesh->ExtendedData.Type).c_str());
				++kStats[sz];
			}
			if (pMesh->BoneBindingCount == 0)
				++kStats["mesh.nobones"];
		}
	}

	for (int m = 0; m < pInfo->MaterialCount; ++m)
	{
		const granny_material* pMaterial = pInfo->Materials[m];
		if (pMaterial && pMaterial->ExtendedData.Type)
		{
			snprintf(sz, sizeof(sz), "material.ext={%s}", TypeSignature(pMaterial->ExtendedData.Type).c_str());
			++kStats[sz];
		}
	}

	for (int t = 0; t < pInfo->TextureCount; ++t)
		if (pInfo->Textures[t] && pInfo->Textures[t]->ImageCount > 0)
			++kStats["texture.embedded"];

	for (int a = 0; a < pInfo->AnimationCount; ++a)
	{
		const granny_animation* pAnim = pInfo->Animations[a];
		++kStats["animation"];
		snprintf(sz, sizeof(sz), "animation.trackgroups=%d", pAnim->TrackGroupCount);
		++kStats[sz];
		snprintf(sz, sizeof(sz), "animation.flags=%d.loop=%d", pAnim->Flags, pAnim->DefaultLoopCount);
		++kStats[sz];
		for (int g = 0; g < pAnim->TrackGroupCount; ++g)
		{
			const granny_track_group* pGroup = pAnim->TrackGroups[g];
			snprintf(sz, sizeof(sz), "trackgroup.flags=0x%X", pGroup->Flags);
			++kStats[sz];
			if (pGroup->PeriodicLoop)
				++kStats["trackgroup.periodicloop"];
			if (pGroup->VectorTrackCount > 0)
				++kStats["trackgroup.vectortracks"];
			if (pGroup->TextTrackCount > 0)
				++kStats["trackgroup.texttracks"];
			if (pGroup->InitialPlacement.Flags)
				++kStats["trackgroup.initialplacement"];
			for (int t = 0; t < pGroup->TransformTrackCount; ++t)
			{
				const granny_transform_track& rTrack = pGroup->TransformTracks[t];
				CountCurve(kStats, "pos", rTrack.PositionCurve);
				CountCurve(kStats, "ori", rTrack.OrientationCurve);
				CountCurve(kStats, "ss", rTrack.ScaleShearCurve);
				snprintf(sz, sizeof(sz), "track.flags=0x%X", rTrack.Flags);
				++kStats[sz];
			}
			for (int v = 0; v < pGroup->VectorTrackCount; ++v)
				CountCurve(kStats, "vec", pGroup->VectorTracks[v].ValueCurve);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Cynex: egri normalizasyonu
// Cynex: DaKeyframes32f oldugu gibi kalir - granny onu kare indeksiyle ve quaternion komsulugu olmadan orneklendiriyor

static bool IsKeyframeCurve(const granny_curve2& rCurve)
{
	return rCurve.CurveData.Type && rCurve.CurveData.Object && GrannyDataTypesAreEqual(GrannyCurveDataDaKeyframes32fType, rCurve.CurveData.Type);
}

class CCurvePool
{
	public:
		~CCurvePool()
		{
			for (granny_curve2* p : m_vecCurves)
				GrannyFreeCurve(p);
		}

		bool Normalize(granny_curve2& rCurve, const granny_real32* c_pfIdentity)
		{
			if (IsRuntimeCurve(rCurve))
				return true;

			const char* szSource = GetCurveTypeName(rCurve);
			granny_curve2* pNew = GrannyCurveConvertToDaK32fC32f(&rCurve, c_pfIdentity);
			if (!pNew)
			{
				m_strError = szSource;
				return false;
			}

			m_vecCurves.push_back(pNew);
			rCurve = *pNew;
			return true;
		}

		std::string	m_strError;

	private:
		std::vector<granny_curve2*> m_vecCurves;
};

static bool NormalizeTrack(granny_transform_track& rTrack, CCurvePool& rkPool)
{
	return rkPool.Normalize(rTrack.PositionCurve, GrannyCurveIdentityPosition)
		&& rkPool.Normalize(rTrack.OrientationCurve, GrannyCurveIdentityOrientation)
		&& rkPool.Normalize(rTrack.ScaleShearCurve, GrannyCurveIdentityScaleShear);
}

static bool NormalizeCurves(granny_file_info* pInfo, CCurvePool& rkPool)
{
	for (int a = 0; a < pInfo->AnimationCount; ++a)
	{
		granny_animation* pAnim = pInfo->Animations[a];
		for (int g = 0; g < pAnim->TrackGroupCount; ++g)
		{
			granny_track_group* pGroup = pAnim->TrackGroups[g];
			for (int t = 0; t < pGroup->TransformTrackCount; ++t)
				if (!NormalizeTrack(pGroup->TransformTracks[t], rkPool))
					return false;
			for (int v = 0; v < pGroup->VectorTrackCount; ++v)
			{
				granny_vector_track& rTrack = pGroup->VectorTracks[v];
				std::vector<granny_real32> vecZero(rTrack.Dimension > 0 ? rTrack.Dimension : 1, 0.0f);
				if (!rkPool.Normalize(rTrack.ValueCurve, vecZero.data()))
					return false;
			}
		}
	}

	// Cynex: animasyona bagli olmayan track group'lar
	for (int g = 0; g < pInfo->TrackGroupCount; ++g)
	{
		granny_track_group* pGroup = pInfo->TrackGroups[g];
		bool bOwned = false;
		for (int a = 0; a < pInfo->AnimationCount && !bOwned; ++a)
			for (int i = 0; i < pInfo->Animations[a]->TrackGroupCount; ++i)
				if (pInfo->Animations[a]->TrackGroups[i] == pGroup)
					bOwned = true;
		if (bOwned)
			continue;
		for (int t = 0; t < pGroup->TransformTrackCount; ++t)
			if (!NormalizeTrack(pGroup->TransformTracks[t], rkPool))
				return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Cynex: --strip - oyunun ve runtime'in hic okumadigi veriler atilir
// Cynex: korunanlar: materyal ExtendedData (Two-sided), text track'ler, bone binding OBB'leri

static void ClearVariant(granny_variant& rVariant)
{
	rVariant.Type = NULL;
	rVariant.Object = NULL;
}

static void StripTopology(granny_tri_topology* pTopo)
{
	if (!pTopo)
		return;
	pTopo->VertexToVertexCount = 0;
	pTopo->VertexToVertexMap = NULL;
	pTopo->VertexToTriangleCount = 0;
	pTopo->VertexToTriangleMap = NULL;
	pTopo->SideToNeighborCount = 0;
	pTopo->SideToNeighborMap = NULL;
	pTopo->PolygonIndexStartCount = 0;
	pTopo->PolygonIndexStarts = NULL;
	pTopo->PolygonIndexCount = 0;
	pTopo->PolygonIndices = NULL;
	pTopo->BonesForTriangleCount = 0;
	pTopo->BonesForTriangle = NULL;
	pTopo->TriangleToBoneCount = 0;
	pTopo->TriangleToBoneIndices = NULL;
	pTopo->TriAnnotationSetCount = 0;
	pTopo->TriAnnotationSets = NULL;
}

static void StripUnused(granny_file_info* pInfo)
{
	ClearVariant(pInfo->ExtendedData);
	if (pInfo->ArtToolInfo)
		ClearVariant(pInfo->ArtToolInfo->ExtendedData);
	if (pInfo->ExporterInfo)
		ClearVariant(pInfo->ExporterInfo->ExtendedData);

	for (int i = 0; i < pInfo->TextureCount; ++i)
	{
		granny_texture* pTexture = pInfo->Textures[i];
		if (!pTexture)
			continue;
		pTexture->ImageCount = 0;
		pTexture->Images = NULL;
		ClearVariant(pTexture->ExtendedData);
	}

	for (int i = 0; i < pInfo->SkeletonCount; ++i)
	{
		granny_skeleton* pSkeleton = pInfo->Skeletons[i];
		if (!pSkeleton)
			continue;
		ClearVariant(pSkeleton->ExtendedData);
		for (int b = 0; b < pSkeleton->BoneCount; ++b)
			ClearVariant(pSkeleton->Bones[b].ExtendedData);
	}

	for (int i = 0; i < pInfo->ModelCount; ++i)
		if (pInfo->Models[i])
			ClearVariant(pInfo->Models[i]->ExtendedData);

	for (int i = 0; i < pInfo->VertexDataCount; ++i)
	{
		granny_vertex_data* pData = pInfo->VertexDatas[i];
		if (!pData)
			continue;
		pData->VertexComponentNameCount = 0;
		pData->VertexComponentNames = NULL;
		pData->VertexAnnotationSetCount = 0;
		pData->VertexAnnotationSets = NULL;
	}

	for (int i = 0; i < pInfo->TriTopologyCount; ++i)
		StripTopology(pInfo->TriTopologies[i]);

	for (int i = 0; i < pInfo->MeshCount; ++i)
	{
		granny_mesh* pMesh = pInfo->Meshes[i];
		if (!pMesh)
			continue;
		ClearVariant(pMesh->ExtendedData);
		StripTopology(pMesh->PrimaryTopology);
		if (pMesh->PrimaryVertexData)
		{
			pMesh->PrimaryVertexData->VertexComponentNameCount = 0;
			pMesh->PrimaryVertexData->VertexComponentNames = NULL;
			pMesh->PrimaryVertexData->VertexAnnotationSetCount = 0;
			pMesh->PrimaryVertexData->VertexAnnotationSets = NULL;
		}
		for (int b = 0; b < pMesh->BoneBindingCount; ++b)
		{
			pMesh->BoneBindings[b].TriangleCount = 0;
			pMesh->BoneBindings[b].TriangleIndices = NULL;
		}
	}

	for (int a = 0; a < pInfo->AnimationCount; ++a)
	{
		granny_animation* pAnim = pInfo->Animations[a];
		if (!pAnim)
			continue;
		ClearVariant(pAnim->ExtendedData);
		for (int g = 0; g < pAnim->TrackGroupCount; ++g)
		{
			granny_track_group* pGroup = pAnim->TrackGroups[g];
			if (!pGroup)
				continue;
			ClearVariant(pGroup->ExtendedData);
			pGroup->VectorTrackCount = 0;
			pGroup->VectorTracks = NULL;
			pGroup->TransformLODErrorCount = 0;
			pGroup->TransformLODErrors = NULL;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Cynex: section atamasi (oyun 1,2,4,5'i model yuklendikten sonra serbest birakiyor)

static void AssignSections(granny_file_data_tree_writer* pWriter, granny_file_info* pInfo)
{
	for (int m = 0; m < pInfo->MeshCount; ++m)
	{
		granny_mesh* pMesh = pInfo->Meshes[m];
		if (!pMesh)
			continue;
		const bool isRigid = GrannyMeshIsRigid(pMesh);
		if (pMesh->PrimaryVertexData && pMesh->PrimaryVertexData->Vertices)
			GrannySetFileSectionForObject(pWriter, pMesh->PrimaryVertexData->Vertices,
				isRigid ? GrannyStandardRigidVertexSection : GrannyStandardDeformableVertexSection);
		granny_tri_topology* pTopo = pMesh->PrimaryTopology;
		if (pTopo)
		{
			const granny_int32x iSection = isRigid ? GrannyStandardRigidIndexSection : GrannyStandardDeformableIndexSection;
			if (pTopo->Indices)
				GrannySetFileSectionForObject(pWriter, pTopo->Indices, iSection);
			if (pTopo->Indices16)
				GrannySetFileSectionForObject(pWriter, pTopo->Indices16, iSection);
		}
	}

	for (int t = 0; t < pInfo->TextureCount; ++t)
	{
		granny_texture* pTexture = pInfo->Textures[t];
		if (!pTexture)
			continue;
		for (int i = 0; i < pTexture->ImageCount; ++i)
		{
			granny_texture_image& rImage = pTexture->Images[i];
			for (int l = 0; l < rImage.MIPLevelCount; ++l)
				if (rImage.MIPLevels[l].PixelBytes)
					GrannySetFileSectionForObject(pWriter, rImage.MIPLevels[l].PixelBytes, GrannyStandardTextureSection);
		}
	}
}

static bool StealWriter(granny_file_writer* pWriter, std::vector<granny_uint8>& out)
{
	granny_uint8* pBuffer = NULL;
	granny_int32x iSize = 0;
	const bool bOk = GrannyStealMemoryWriterBuffer(pWriter, &pBuffer, &iSize);
	GrannyCloseFileWriter(pWriter);
	if (!bOk)
		return false;
	out.assign(pBuffer, pBuffer + iSize);
	GrannyFreeMemoryWriterBuffer(pBuffer);
	return true;
}

static bool WriteNativeFile(granny_file_info* pInfo, std::vector<granny_uint8>& out)
{
	granny_file_data_tree_writer* pWriter = GrannyBeginFileDataTreeWriting(GrannyFileInfoType, pInfo,
		GrannyStandardMainSection, GrannyStandardMainSection);
	if (!pWriter)
		return false;

	AssignSections(pWriter, pInfo);

	granny_file_builder* pBuilder = GrannyBeginFileInMemory(GrannyStandardSectionCount, GrannyCurrentGRNStandardTag,
		GrannyGRNFileMV_ThisPlatform, 1 << 16);
	if (!pBuilder)
	{
		GrannyEndFileDataTreeWriting(pWriter);
		return false;
	}

	for (int i = 0; i < GrannyStandardSectionCount; ++i)
		GrannySetFileSectionFormat(pBuilder, i, GrannyNoCompression, 16);

	if (!GrannyWriteDataTreeToFileBuilder(pWriter, pBuilder))
	{
		GrannyAbortFile(pBuilder);
		GrannyEndFileDataTreeWriting(pWriter);
		return false;
	}
	GrannyEndFileDataTreeWriting(pWriter);

	granny_file_writer* pMemWriter = GrannyCreateMemoryFileWriter(1 << 16);
	if (!GrannyEndFileToWriter(pBuilder, pMemWriter))
	{
		GrannyCloseFileWriter(pMemWriter);
		return false;
	}
	return StealWriter(pMemWriter, out);
}

static bool ConvertPlatform(const std::vector<granny_uint8>& in, int iPointerBits, std::vector<granny_uint8>& out)
{
	granny_grn_file_magic_value kMagic;
	if (!GrannyGetMagicValueForPlatform(iPointerBits, true, &kMagic))
		return false;

	granny_file_reader* pReader = GrannyCreateMemoryFileReader((granny_int32x) in.size(), in.data());
	granny_file_writer* pWriter = GrannyCreateMemoryFileWriter(1 << 16);
	const bool bOk = GrannyPlatformConvertReaderToWriterAligned(pReader, pWriter, &kMagic, false, 16);
	GrannyCloseFileReader(pReader);
	if (!bOk)
	{
		GrannyCloseFileWriter(pWriter);
		return false;
	}
	return StealWriter(pWriter, out);
}

/////////////////////////////////////////////////////////////////////////////
// Cynex: dogrulama - cikti dosyasi granny ile tekrar acilip ozet karsilastirilir

static bool AlmostEqual(float a, float b)
{
	if (a != a || b != b)
		return (a != a) && (b != b);
	const float d = a > b ? a - b : b - a;
	const float m = (a < 0 ? -a : a) > (b < 0 ? -b : b) ? (a < 0 ? -a : a) : (b < 0 ? -b : b);
	return d <= 1e-5f * (m > 1.0f ? m : 1.0f);
}

static bool VerifyOutput(granny_file_info* pSrc, const std::vector<granny_uint8>& out, std::string& err)
{
	granny_file* pFile = GrannyReadEntireFileFromMemory((granny_int32x) out.size(), out.data());
	if (!pFile)
	{
		err = "cikti tekrar okunamadi";
		return false;
	}

	bool bOk = true;
	granny_file_info* pDst = GrannyGetFileInfo(pFile);
	if (!pDst || pDst->ModelCount != pSrc->ModelCount || pDst->AnimationCount != pSrc->AnimationCount
		|| pDst->MeshCount != pSrc->MeshCount)
	{
		err = "cikti yapisi farkli";
		bOk = false;
	}

	for (int m = 0; bOk && m < pSrc->MeshCount; ++m)
	{
		const granny_mesh* a = pSrc->Meshes[m];
		const granny_mesh* b = pDst->Meshes[m];
		const int vtxA = GrannyGetMeshVertexCount(a);
		if (vtxA != GrannyGetMeshVertexCount(b) || GrannyGetMeshIndexCount(a) != GrannyGetMeshIndexCount(b))
		{
			err = "mesh sayilari farkli";
			bOk = false;
			break;
		}
		const int size = GrannyGetTotalObjectSize(GrannyGetMeshVertexType(a)) * vtxA;
		if (size != GrannyGetTotalObjectSize(GrannyGetMeshVertexType(b)) * vtxA
			|| (size > 0 && memcmp(GrannyGetMeshVertices(a), GrannyGetMeshVertices(b), size) != 0))
		{
			err = "vertex verisi farkli";
			bOk = false;
		}
	}

	// Cynex: egri orneklemesi kaynakla ayni olmali
	for (int a = 0; bOk && a < pSrc->AnimationCount; ++a)
	{
		const granny_animation* pA = pSrc->Animations[a];
		const granny_animation* pB = pDst->Animations[a];
		if (pA->TrackGroupCount != pB->TrackGroupCount || !AlmostEqual(pA->Duration, pB->Duration))
		{
			err = "animasyon farkli";
			bOk = false;
			break;
		}
		for (int g = 0; bOk && g < pA->TrackGroupCount; ++g)
		{
			const granny_track_group* pGA = pA->TrackGroups[g];
			const granny_track_group* pGB = pB->TrackGroups[g];
			if (pGA->TransformTrackCount != pGB->TransformTrackCount)
			{
				err = "track sayisi farkli";
				bOk = false;
				break;
			}
			for (int t = 0; bOk && t < pGA->TransformTrackCount; ++t)
			{
				const granny_transform_track& rA = pGA->TransformTracks[t];
				const granny_transform_track& rB = pGB->TransformTracks[t];
				// Cynex: keyframe egriler birebir korunur
				const granny_curve2* kc[3] = { &rA.PositionCurve, &rA.OrientationCurve, &rA.ScaleShearCurve };
				const granny_curve2* kd[3] = { &rB.PositionCurve, &rB.OrientationCurve, &rB.ScaleShearCurve };
				for (int c = 0; c < 3 && bOk; ++c)
				{
					if (!IsKeyframeCurve(*kc[c]))
						continue;
					const granny_curve_data_da_keyframes32f* pKa = static_cast<const granny_curve_data_da_keyframes32f*>(kc[c]->CurveData.Object);
					const granny_curve_data_da_keyframes32f* pKb = IsKeyframeCurve(*kd[c]) ? static_cast<const granny_curve_data_da_keyframes32f*>(kd[c]->CurveData.Object) : NULL;
					if (!pKb || pKa->ControlCount != pKb->ControlCount
						|| (pKa->ControlCount > 0 && memcmp(pKa->Controls, pKb->Controls, pKa->ControlCount * sizeof(granny_real32)) != 0))
					{
						err = std::string("keyframe egrisi farkli: ") + (rA.Name ? rA.Name : "?");
						bOk = false;
					}
				}
				for (int s = 0; s <= 8 && bOk; ++s)
				{
					const float fT = pA->Duration * (float) s / 8.0f;
					float fA[9], fB[9];
					const granny_curve2* ac[3] = { &rA.PositionCurve, &rA.OrientationCurve, &rA.ScaleShearCurve };
					const granny_curve2* bc[3] = { &rB.PositionCurve, &rB.OrientationCurve, &rB.ScaleShearCurve };
					const int dims[3] = { 3, 4, 9 };
					const granny_real32* ids[3] = { GrannyCurveIdentityPosition, GrannyCurveIdentityOrientation, GrannyCurveIdentityScaleShear };
					for (int c = 0; c < 3 && bOk; ++c)
					{
						if (IsKeyframeCurve(*ac[c]))
							continue;
						GrannyEvaluateCurveAtT(dims[c], c == 1, false, ac[c], false, pA->Duration, fT, fA, ids[c]);
						GrannyEvaluateCurveAtT(dims[c], c == 1, false, bc[c], false, pA->Duration, fT, fB, ids[c]);
						for (int k = 0; k < dims[c]; ++k)
						{
							if (!AlmostEqual(fA[k], fB[k]))
							{
								char sz[256];
								snprintf(sz, sizeof(sz), "egri farkli: %s c%d t=%g %g!=%g", rA.Name ? rA.Name : "?", c, fT, fA[k], fB[k]);
								err = sz;
								bOk = false;
								break;
							}
						}
					}
				}
			}
		}
	}

	GrannyFreeFile(pFile);
	return bOk;
}

/////////////////////////////////////////////////////////////////////////////

static bool ConvertOne(const std::vector<granny_uint8>& in, const SOptions& opt, TCounter& kStats,
	std::vector<granny_uint8>& out, std::string& err)
{
	g_strLastGrannyError.clear();

	granny_file* pFile = GrannyReadEntireFileFromMemory((granny_int32x) in.size(), in.data());
	if (!pFile)
	{
		err = "granny okuyamadi: " + g_strLastGrannyError;
		return false;
	}

	granny_file_info* pInfo = GrannyGetFileInfo(pFile);
	if (!pInfo)
	{
		GrannyFreeFile(pFile);
		err = "file_info yok: " + g_strLastGrannyError;
		return false;
	}

	CollectStats(kStats, pFile, pInfo);

	if (opt.bStatsOnly)
	{
		GrannyFreeFile(pFile);
		return true;
	}

	bool bOk = true;
	{
		CCurvePool kPool;
		if (opt.bStrip)
			StripUnused(pInfo);
		if (opt.bNormalizeCurves && !NormalizeCurves(pInfo, kPool))
		{
			err = "egri donusturulemedi (" + kPool.m_strError + "): " + g_strLastGrannyError;
			bOk = false;
		}

		std::vector<granny_uint8> vecNative;
		if (bOk && !WriteNativeFile(pInfo, vecNative))
		{
			err = "yazilamadi: " + g_strLastGrannyError;
			bOk = false;
		}

		if (bOk)
		{
			if (opt.iPointerBits == 32)
			{
				out.swap(vecNative);
			}
			else if (!ConvertPlatform(vecNative, opt.iPointerBits, out))
			{
				err = "platform donusumu basarisiz: " + g_strLastGrannyError;
				bOk = false;
			}
		}

		// Cynex: kiyas, bellekte degistirilmis agaca degil orijinal dosyaya karsi
		if (bOk && opt.bVerify)
		{
			granny_file* pRefFile = GrannyReadEntireFileFromMemory((granny_int32x) in.size(), in.data());
			granny_file_info* pRefInfo = pRefFile ? GrannyGetFileInfo(pRefFile) : NULL;
			if (!pRefInfo)
			{
				err = "dogrulama icin orijinal tekrar okunamadi";
				bOk = false;
			}
			else if (!VerifyOutput(pRefInfo, out, err))
				bOk = false;
			if (pRefFile)
				GrannyFreeFile(pRefFile);
		}
	}

	GrannyFreeFile(pFile);
	return bOk;
}

static void PrintUsage()
{
	printf(
		"gr2conv - Granny .gr2 -> mobil duzen (sikistirmasiz, little-endian, 64bit pointer)\n"
		"kullanim: gr2conv [secenekler] <giris-dizini> <cikis-dizini>\n"
		"  --ptr32          32bit pointer duzeni yaz (x86 dogrulama icin)\n"
		"  --keep-curves    egri formatlarini olduklari gibi birak\n"
		"  --no-verify      ciktiyi granny ile tekrar acip karsilastirma\n"
		"  --stats          sadece istatistik (cikti yazmaz)\n"
		"  --skip-existing  cikti dosyasi varsa atla\n"
		"  --verbose        her dosyayi yaz\n");
}

static bool ParseArgs(int argc, wchar_t** argv, SOptions& opt)
{
	std::vector<std::wstring> vecPositional;
	for (int i = 1; i < argc; ++i)
	{
		const std::wstring a = argv[i];
		if (a == L"--ptr32")
			opt.iPointerBits = 32;
		else if (a == L"--keep-curves")
			opt.bNormalizeCurves = false;
		else if (a == L"--strip")
			opt.bStrip = true;
		else if (a == L"--no-verify")
			opt.bVerify = false;
		else if (a == L"--stats")
			opt.bStatsOnly = true;
		else if (a == L"--skip-existing")
			opt.bOverwrite = false;
		else if (a == L"--verbose")
			opt.bVerbose = true;
		else if (a.size() > 2 && a[0] == L'-' && a[1] == L'-')
			return false;
		else
			vecPositional.push_back(a);
	}

	if (vecPositional.size() < 1 || (!opt.bStatsOnly && vecPositional.size() < 2))
		return false;
	opt.inDir = vecPositional[0];
	if (vecPositional.size() > 1)
		opt.outDir = vecPositional[1];
	return true;
}

static std::string FormatBytes(long long ll)
{
	char sz[64];
	if (ll >= (1LL << 30))
		snprintf(sz, sizeof(sz), "%.2f GB", ll / double(1LL << 30));
	else if (ll >= (1LL << 20))
		snprintf(sz, sizeof(sz), "%.2f MB", ll / double(1LL << 20));
	else
		snprintf(sz, sizeof(sz), "%lld B", ll);
	return sz;
}

int wmain(int argc, wchar_t** argv)
{
	SOptions opt;
	if (!ParseArgs(argc, argv, opt))
	{
		PrintUsage();
		return 2;
	}

	if (!GrannyVersionsMatch)
	{
		fprintf(stderr, "granny2.dll surumu header ile uyusmuyor\n");
		return 1;
	}

	granny_log_callback kLog = { GrannyLogToString, NULL };
	GrannySetLogCallback(&kLog);

	std::error_code ec;
	std::vector<fs::path> vecFiles;
	for (fs::recursive_directory_iterator it(opt.inDir, fs::directory_options::skip_permission_denied, ec), end; it != end; it.increment(ec))
	{
		if (ec)
			break;
		if (!it->is_regular_file(ec))
			continue;
		std::wstring ext = it->path().extension().wstring();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
		if (ext == L".gr2")
			vecFiles.push_back(it->path());
	}
	std::sort(vecFiles.begin(), vecFiles.end());

	printf("%zu .gr2 dosyasi bulundu (%s)\n", vecFiles.size(), ToUtf8(opt.inDir).c_str());

	SSummary kSum;
	const auto tStart = std::chrono::steady_clock::now();
	std::vector<granny_uint8> vecIn, vecOut;

	for (size_t i = 0; i < vecFiles.size(); ++i)
	{
		const fs::path& src = vecFiles[i];
		const fs::path rel = fs::relative(src, opt.inDir, ec);
		const fs::path dst = opt.outDir / rel;
		++kSum.llFiles;

		if (!opt.bStatsOnly && !opt.bOverwrite && fs::exists(dst, ec))
		{
			++kSum.llSkipped;
			continue;
		}

		std::string err;
		bool bOk = ReadWholeFile(src, vecIn);
		if (!bOk)
			err = "okunamadi";
		else
		{
			kSum.llBytesIn += (long long) vecIn.size();
			vecOut.clear();
			bOk = ConvertOne(vecIn, opt, kSum.kStats, vecOut, err);
			if (bOk && !opt.bStatsOnly)
			{
				if (!WriteWholeFile(dst, vecOut.data(), vecOut.size()))
				{
					err = "yazilamadi: " + ToUtf8(dst);
					bOk = false;
				}
				else
					kSum.llBytesOut += (long long) vecOut.size();
			}
		}

		if (bOk)
			++kSum.llConverted;
		else
		{
			++kSum.llFailed;
			kSum.vecFailures.push_back(ToUtf8(rel) + ": " + err);
		}

		if (opt.bVerbose || !bOk)
			printf("[%zu/%zu] %s %s%s\n", i + 1, vecFiles.size(), bOk ? "OK  " : "HATA", ToUtf8(rel).c_str(), bOk ? "" : (" - " + err).c_str());
		else if ((i + 1) % 500 == 0)
			printf("[%zu/%zu]\n", i + 1, vecFiles.size());
	}

	const double dSec = std::chrono::duration<double>(std::chrono::steady_clock::now() - tStart).count();

	std::string strReport;
	char sz[512];
	snprintf(sz, sizeof(sz), "dosya: %lld, basarili: %lld, hatali: %lld, atlanan: %lld, sure: %.1f sn\n", kSum.llFiles, kSum.llConverted, kSum.llFailed, kSum.llSkipped, dSec);
	strReport += sz;
	snprintf(sz, sizeof(sz), "boyut: giris %s, cikis %s\n", FormatBytes(kSum.llBytesIn).c_str(), FormatBytes(kSum.llBytesOut).c_str());
	strReport += sz;
	strReport += "\nistatistik:\n";
	for (const auto& kv : kSum.kStats)
	{
		snprintf(sz, sizeof(sz), "  %-90s %lld\n", kv.first.c_str(), kv.second);
		strReport += sz;
	}
	if (!kSum.vecFailures.empty())
	{
		strReport += "\nhatalar:\n";
		for (const std::string& s : kSum.vecFailures)
			strReport += "  " + s + "\n";
	}

	fputs(strReport.c_str(), stdout);

	if (!opt.outDir.empty())
	{
		fs::create_directories(opt.outDir, ec);
		const fs::path report = opt.outDir / (opt.bStatsOnly ? L"gr2conv_stats.txt" : L"gr2conv_report.txt");
		WriteWholeFile(report, (const granny_uint8*) strReport.data(), strReport.size());
	}

	return kSum.llFailed ? 1 : 0;
}
