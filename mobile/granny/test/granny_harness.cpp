// Cynex: cynex_granny dogrulama araci
// Cynex: ayni senaryo gercek granny2.dll (orijinal dosya) ve cynex_granny.dll (cevrilmis dosya) ile kosulur, sonuclar karsilastirilir
// Cynex: kullanim:
//   granny_harness compare <oracle.dll> <origdir> <cynex.dll> <convdir> <cases.txt> [rapor.txt]   (x86, ayni process)
//   granny_harness dump <granny.dll> <datadir> <cases.txt> <cikti.bin>
//   granny_harness diff <a.bin> <b.bin> [rapor.txt]
#include <windows.h>
#include <granny.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#define GRANNY_API_LIST(X) \
	X(GrannyReadEntireFileFromMemory) X(GrannyGetFileInfo) X(GrannyFreeFile) X(GrannyFreeFileSection) \
	X(GrannyMeshIsRigid) X(GrannyGetMeshVertexCount) X(GrannyGetMeshVertexType) X(GrannyGetMeshVertices) \
	X(GrannyGetMeshIndexCount) X(GrannyCopyMeshIndices) X(GrannyCopyMeshVertices) X(GrannyGetMeshTriangleGroupCount) \
	X(GrannyGetMeshTriangleGroups) X(GrannyGetTotalTypeSize) X(GrannyGetMaterialTextureByType) X(GrannyFindMatchingMember) \
	X(GrannyConvertSingleObject) X(GrannyNewMeshBinding) X(GrannyFreeMeshBinding) X(GrannyGetMeshBindingToBoneIndices) \
	X(GrannyNewMeshDeformer) X(GrannyFreeMeshDeformer) X(GrannyDeformVertices) X(GrannyInstantiateModel) \
	X(GrannyFreeModelInstance) X(GrannyGetSourceSkeleton) X(GrannyNewWorldPose) X(GrannyFreeWorldPose) \
	X(GrannyNewLocalPose) X(GrannyFreeLocalPose) X(GrannyGetWorldPose4x4) X(GrannyGetWorldPoseComposite4x4) \
	X(GrannyGetWorldPoseComposite4x4Array) X(GrannyBuildWorldPose) X(GrannySampleModelAnimations) \
	X(GrannySampleModelAnimationsAccelerated) X(GrannySetModelClock) X(GrannyFreeCompletedModelControls) \
	X(GrannyPlayControlledAnimation) X(GrannyFreeControl) X(GrannyFreeControlOnceUnused) X(GrannyFreeControlIfComplete) \
	X(GrannyCompleteControlAt) X(GrannyControlIsComplete) X(GrannySetControlSpeed) X(GrannyGetControlSpeed) \
	X(GrannySetControlLoopCount) X(GrannyGetControlLoopCount) X(GrannySetControlEaseIn) X(GrannySetControlEaseOut) \
	X(GrannySetControlEaseInCurve) X(GrannySetControlEaseOutCurve) X(GrannySetControlRawLocalClock) \
	X(GrannyGetControlRawLocalClock) X(GrannyGetControlLocalDuration) X(GrannyUpdateModelMatrix) X(GrannyFindBoneByName) \
	X(GrannySetLogCallback)

struct SApi
{
#define X(name) decltype(&::name) name;
	GRANNY_API_LIST(X)
#undef X
	granny_data_type_definition ** ppPNT332;
	HMODULE hModule;
};

static FARPROC FindExport(HMODULE hModule, const char * c_szName)
{
	FARPROC p = GetProcAddress(hModule, c_szName);
	if (p)
		return p;
	// Cynex: x86 __stdcall isimleri _Ad@N
	const uint8_t * base = reinterpret_cast<const uint8_t *>(hModule);
	const IMAGE_DOS_HEADER * dos = reinterpret_cast<const IMAGE_DOS_HEADER *>(base);
	const IMAGE_NT_HEADERS * nt = reinterpret_cast<const IMAGE_NT_HEADERS *>(base + dos->e_lfanew);
	const IMAGE_DATA_DIRECTORY & dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	if (!dir.VirtualAddress)
		return NULL;
	const IMAGE_EXPORT_DIRECTORY * exp = reinterpret_cast<const IMAGE_EXPORT_DIRECTORY *>(base + dir.VirtualAddress);
	const DWORD * names = reinterpret_cast<const DWORD *>(base + exp->AddressOfNames);
	const size_t len = strlen(c_szName);
	for (DWORD i = 0; i < exp->NumberOfNames; ++i)
	{
		const char * n = reinterpret_cast<const char *>(base + names[i]);
		if (n[0] == '_' && strncmp(n + 1, c_szName, len) == 0 && n[1 + len] == '@')
			return GetProcAddress(hModule, n);
	}
	return NULL;
}

static bool LoadApi(const char * c_szPath, SApi & api)
{
	api.hModule = LoadLibraryA(c_szPath);
	if (!api.hModule)
	{
		fprintf(stderr, "yuklenemedi: %s (%lu)\n", c_szPath, GetLastError());
		return false;
	}
#define X(name) api.name = reinterpret_cast<decltype(api.name)>(FindExport(api.hModule, #name)); if (!api.name) { fprintf(stderr, "eksik: %s\n", #name); return false; }
	GRANNY_API_LIST(X)
#undef X
	api.ppPNT332 = reinterpret_cast<granny_data_type_definition **>(FindExport(api.hModule, "GrannyPNT332VertexType"));
	return api.ppPNT332 != NULL;
}

/////////////////////////////////////////////////////////////////////////////

struct SRecord
{
	std::string			strLabel;
	std::string			strText;
	std::vector<float>	vecData;
	int					iKind;	// 0 sayi, 1 metin, 2 tam sayi (birebir)
};

typedef std::vector<SRecord> TRecords;

struct SSink
{
	TRecords	kRecords;
	bool		bEnabled = true;

	void Floats(const std::string & label, const float * p, size_t n)
	{
		if (!bEnabled)
			return;
		SRecord r;
		r.strLabel = label;
		r.vecData.assign(p, p + n);
		r.iKind = 0;
		kRecords.push_back(std::move(r));
	}

	void Exact(const std::string & label, const void * p, size_t bytes)
	{
		if (!bEnabled)
			return;
		SRecord r;
		r.strLabel = label;
		r.vecData.resize((bytes + 3) / 4, 0.0f);
		memcpy(r.vecData.data(), p, bytes);
		r.iKind = 2;
		kRecords.push_back(std::move(r));
	}

	void Int(const std::string & label, int v)
	{
		Exact(label, &v, sizeof(v));
	}

	void Text(const std::string & label, const char * s)
	{
		if (!bEnabled)
			return;
		SRecord r;
		r.strLabel = label;
		r.strText = s ? s : "(null)";
		r.iKind = 1;
		kRecords.push_back(std::move(r));
	}
};

static bool ReadFileBytes(const std::string & path, std::vector<char> & out)
{
	FILE * fp = fopen(path.c_str(), "rb");
	if (!fp)
		return false;
	fseek(fp, 0, SEEK_END);
	const long n = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	out.resize(n > 0 ? (size_t) n : 0);
	const size_t got = n > 0 ? fread(out.data(), 1, (size_t) n, fp) : 0;
	fclose(fp);
	return got == out.size();
}

/////////////////////////////////////////////////////////////////////////////
// Cynex: EterGrnLib CGrannyModelInstance davranisinin kopyasi

struct SInstance
{
	granny_model *						pModel = NULL;
	granny_model_instance *				pInstance = NULL;
	granny_world_pose *					pWorldPose = NULL;
	granny_control *					pControl = NULL;
	std::vector<granny_mesh_binding *>	vecBindings;
	std::vector<granny_mesh_deformer *>	vecDeformers;
	float								fLocalTime = 0.0f;
	SInstance *							pSkeletonOwner = NULL;
};

struct SScenario
{
	const SApi &		api;
	SSink &				sink;
	granny_local_pose *	pSharedPose = NULL;
	int					iSharedBones = 0;

	SScenario(const SApi & a, SSink & s) : api(a), sink(s) {}

	granny_world_pose * WorldPose(SInstance & r)
	{
		return r.pSkeletonOwner ? r.pSkeletonOwner->pWorldPose : r.pWorldPose;
	}

	void Create(SInstance & r, granny_model * pModel, SInstance * pSkeletonOwner)
	{
		r.pModel = pModel;
		r.pInstance = api.GrannyInstantiateModel(pModel);
		r.pSkeletonOwner = pSkeletonOwner;
		granny_skeleton * pDstSkeleton = pSkeletonOwner ? pSkeletonOwner->pModel->Skeleton : pModel->Skeleton;
		if (!pSkeletonOwner)
			r.pWorldPose = api.GrannyNewWorldPose(api.GrannyGetSourceSkeleton(r.pInstance)->BoneCount);
		for (int m = 0; m < pModel->MeshBindingCount; ++m)
		{
			granny_mesh * pMesh = pModel->MeshBindings[m].Mesh;
			r.vecBindings.push_back(api.GrannyNewMeshBinding(pMesh, pModel->Skeleton, pDstSkeleton));
			granny_mesh_deformer * pDeformer = NULL;
			if (!api.GrannyMeshIsRigid(pMesh))
				pDeformer = api.GrannyNewMeshDeformer(api.GrannyGetMeshVertexType(pMesh), *api.ppPNT332, GrannyDeformPositionNormal, GrannyDontAllowUncopiedTail);
			r.vecDeformers.push_back(pDeformer);
		}
	}

	void Destroy(SInstance & r)
	{
		for (granny_mesh_binding * p : r.vecBindings)
			if (p)
				api.GrannyFreeMeshBinding(p);
		for (granny_mesh_deformer * p : r.vecDeformers)
			if (p)
				api.GrannyFreeMeshDeformer(p);
		if (r.pInstance)
			api.GrannyFreeModelInstance(r.pInstance);
		if (r.pWorldPose)
			api.GrannyFreeWorldPose(r.pWorldPose);
		r = SInstance();
	}

	void SetMotion(SInstance & r, granny_animation * pAnim, float fBlend, int iLoop, float fSpeed)
	{
		const float t = r.fLocalTime;
		bool isFirst = false;
		if (r.pControl)
		{
			api.GrannySetControlEaseOutCurve(r.pControl, t, t + fBlend, 1.0f, 1.0f, 0.0f, 0.0f);
			api.GrannySetControlEaseIn(r.pControl, false);
			api.GrannySetControlEaseOut(r.pControl, true);
			api.GrannyCompleteControlAt(r.pControl, t + fBlend);
			api.GrannyFreeControlIfComplete(r.pControl);
		}
		else
			isFirst = true;

		r.pControl = api.GrannyPlayControlledAnimation(t, pAnim, r.pInstance);
		sink.Int("play.null", r.pControl ? 0 : 1);
		if (!r.pControl)
			return;
		api.GrannySetControlSpeed(r.pControl, fSpeed);
		api.GrannySetControlLoopCount(r.pControl, iLoop);
		if (isFirst)
		{
			api.GrannySetControlEaseIn(r.pControl, false);
			api.GrannySetControlEaseOut(r.pControl, false);
		}
		else
		{
			api.GrannySetControlEaseIn(r.pControl, true);
			api.GrannySetControlEaseOut(r.pControl, false);
			if (fBlend > 0.0f)
				api.GrannySetControlEaseInCurve(r.pControl, t, t + fBlend, 0.0f, 0.0f, 1.0f, 1.0f);
		}
		api.GrannyFreeControlOnceUnused(r.pControl);
	}

	void ChangeMotion(SInstance & r, granny_animation * pAnim, int iLoop, float fSpeed)
	{
		const float t = r.fLocalTime - 0.3f;
		if (r.pControl)
		{
			api.GrannySetControlEaseIn(r.pControl, false);
			api.GrannySetControlEaseOut(r.pControl, false);
			api.GrannyCompleteControlAt(r.pControl, t);
			api.GrannyFreeControlIfComplete(r.pControl);
		}
		r.pControl = api.GrannyPlayControlledAnimation(t, pAnim, r.pInstance);
		sink.Int("change.null", r.pControl ? 0 : 1);
		if (!r.pControl)
			return;
		api.GrannySetControlSpeed(r.pControl, fSpeed);
		api.GrannySetControlLoopCount(r.pControl, iLoop);
		api.GrannySetControlEaseIn(r.pControl, false);
		api.GrannySetControlEaseOut(r.pControl, false);
		api.GrannyFreeControlOnceUnused(r.pControl);
	}

	void CopyMotion(SInstance & r, SInstance & src, granny_animation * pAnim)
	{
		if (!src.pControl)
			return;
		if (r.pControl)
			api.GrannyFreeControl(r.pControl);
		r.pControl = api.GrannyPlayControlledAnimation(r.fLocalTime, pAnim, r.pInstance);
		sink.Int("copy.null", r.pControl ? 0 : 1);
		if (!r.pControl)
			return;
		api.GrannySetControlSpeed(r.pControl, api.GrannyGetControlSpeed(src.pControl));
		api.GrannySetControlLoopCount(r.pControl, api.GrannyGetControlLoopCount(src.pControl));
		api.GrannySetControlEaseIn(r.pControl, true);
		api.GrannySetControlEaseOut(r.pControl, false);
		api.GrannySetControlRawLocalClock(r.pControl, api.GrannyGetControlRawLocalClock(src.pControl));
		api.GrannyFreeControlOnceUnused(r.pControl);
	}

	void Update(SInstance & r, const float * c_pOffset, const char * c_szTag)
	{
		api.GrannyFreeCompletedModelControls(r.pInstance);
		api.GrannySetModelClock(r.pInstance, r.fLocalTime);

		if (r.pSkeletonOwner)
			return;

		granny_skeleton * pSkeleton = api.GrannyGetSourceSkeleton(r.pInstance);
		if (!pSharedPose || iSharedBones < pSkeleton->BoneCount)
		{
			if (pSharedPose)
				api.GrannyFreeLocalPose(pSharedPose);
			iSharedBones = pSkeleton->BoneCount;
			pSharedPose = api.GrannyNewLocalPose(iSharedBones);
		}
		api.GrannySampleModelAnimationsAccelerated(r.pInstance, pSkeleton->BoneCount, c_pOffset, pSharedPose, r.pWorldPose);
		api.GrannyFreeCompletedModelControls(r.pInstance);

		std::string tag = c_szTag;
		std::vector<float> world(16 * (size_t) pSkeleton->BoneCount), comp(16 * (size_t) pSkeleton->BoneCount);
		for (int b = 0; b < pSkeleton->BoneCount; ++b)
		{
			memcpy(&world[16 * b], api.GrannyGetWorldPose4x4(r.pWorldPose, b), 64);
			memcpy(&comp[16 * b], api.GrannyGetWorldPoseComposite4x4(r.pWorldPose, b), 64);
		}
		sink.Floats(tag + ".world", world.data(), world.size());
		sink.Floats(tag + ".composite", comp.data(), comp.size());
		sink.Int(tag + ".outofrange", api.GrannyGetWorldPose4x4(r.pWorldPose, pSkeleton->BoneCount) ? 1 : 0);
	}

	void Deform(SInstance & r, const char * c_szTag)
	{
		granny_world_pose * pPose = WorldPose(r);
		if (!pPose)
			return;
		const float * pComposite = reinterpret_cast<const float *>(api.GrannyGetWorldPoseComposite4x4Array(pPose));
		for (size_t m = 0; m < r.vecDeformers.size(); ++m)
		{
			granny_mesh * pMesh = r.pModel->MeshBindings[m].Mesh;
			if (!r.vecDeformers[m])
			{
				// Cynex: rigid mesh dunya matrisi (EterGrnLib UpdateWorldMatrices)
				const granny_int32x * pBones = api.GrannyGetMeshBindingToBoneIndices(r.vecBindings[m]);
				sink.Floats(std::string(c_szTag) + ".rigidbone", pComposite + 16 * pBones[0], 16);
				continue;
			}
			const int n = api.GrannyGetMeshVertexCount(pMesh);
			std::vector<granny_pnt332_vertex> out((size_t) n);
			api.GrannyDeformVertices(r.vecDeformers[m], api.GrannyGetMeshBindingToBoneIndices(r.vecBindings[m]), pComposite, n, api.GrannyGetMeshVertices(pMesh), out.data());
			sink.Floats(std::string(c_szTag) + ".deform", reinterpret_cast<const float *>(out.data()), out.size() * 8);
		}
	}

	void RecordModelData(granny_file_info * pInfo, const char * c_szTag)
	{
		const std::string tag = c_szTag;
		sink.Int(tag + ".models", pInfo->ModelCount);
		sink.Int(tag + ".animations", pInfo->AnimationCount);
		for (int mi = 0; mi < pInfo->ModelCount; ++mi)
		{
			granny_model * pModel = pInfo->Models[mi];
			sink.Text(tag + ".model.name", pModel->Name);
			granny_skeleton * pSkeleton = pModel->Skeleton;
			sink.Int(tag + ".bones", pSkeleton->BoneCount);
			for (int b = 0; b < pSkeleton->BoneCount; ++b)
			{
				sink.Text(tag + ".bone", pSkeleton->Bones[b].Name);
				granny_int32x index = -1;
				const bool found = api.GrannyFindBoneByName(pSkeleton, pSkeleton->Bones[b].Name, &index);
				sink.Int(tag + ".findbone", found ? index : -100);
			}
			granny_int32x missing = -7;
			sink.Int(tag + ".findmissing", api.GrannyFindBoneByName(pSkeleton, "CynexYokKemik", &missing) ? 1 : 0);
			sink.Int(tag + ".findmissing.index", missing);

			for (int m = 0; m < pModel->MeshBindingCount; ++m)
			{
				granny_mesh * pMesh = pModel->MeshBindings[m].Mesh;
				sink.Text(tag + ".mesh.name", pMesh->Name);
				sink.Int(tag + ".mesh.rigid", api.GrannyMeshIsRigid(pMesh) ? 1 : 0);
				const int n = api.GrannyGetMeshVertexCount(pMesh);
				sink.Int(tag + ".mesh.vertices", n);
				granny_data_type_definition * pType = api.GrannyGetMeshVertexType(pMesh);
				sink.Int(tag + ".mesh.typesize", pType ? api.GrannyGetTotalTypeSize(pType) : -1);
				const int ic = api.GrannyGetMeshIndexCount(pMesh);
				sink.Int(tag + ".mesh.indices", ic);
				if (n > 0 && pType)
				{
					std::vector<granny_pnt332_vertex> v((size_t) n);
					api.GrannyCopyMeshVertices(pMesh, *api.ppPNT332, v.data());
					sink.Exact(tag + ".mesh.pnt332", v.data(), v.size() * sizeof(granny_pnt332_vertex));
					granny_data_type_definition pnt3322[5] =
					{
						{ GrannyReal32Member, GrannyVertexPositionName, 0, 3 },
						{ GrannyReal32Member, GrannyVertexNormalName, 0, 3 },
						{ GrannyReal32Member, GrannyVertexTextureCoordinatesName "0", 0, 2 },
						{ GrannyReal32Member, GrannyVertexTextureCoordinatesName "1", 0, 2 },
						{ GrannyEndMember },
					};
					std::vector<float> v2((size_t) n * 10);
					api.GrannyCopyMeshVertices(pMesh, pnt3322, v2.data());
					sink.Exact(tag + ".mesh.pnt3322", v2.data(), v2.size() * 4);
				}
				if (ic > 0)
				{
					std::vector<uint16_t> i16((size_t) ic);
					api.GrannyCopyMeshIndices(pMesh, 2, i16.data());
					sink.Exact(tag + ".mesh.idx16", i16.data(), i16.size() * 2);
					std::vector<uint32_t> i32((size_t) ic);
					api.GrannyCopyMeshIndices(pMesh, 4, i32.data());
					sink.Exact(tag + ".mesh.idx32", i32.data(), i32.size() * 4);
				}
				const int gc = api.GrannyGetMeshTriangleGroupCount(pMesh);
				sink.Int(tag + ".mesh.groups", gc);
				if (gc > 0)
					sink.Exact(tag + ".mesh.groupdata", api.GrannyGetMeshTriangleGroups(pMesh), (size_t) gc * sizeof(granny_tri_material_group));

				granny_mesh_binding * pBinding = api.GrannyNewMeshBinding(pMesh, pSkeleton, pSkeleton);
				const int bb = pMesh->BoneBindingCount > 0 ? pMesh->BoneBindingCount : 1;
				sink.Exact(tag + ".mesh.bindindices", api.GrannyGetMeshBindingToBoneIndices(pBinding), (size_t) bb * sizeof(granny_int32x));
				api.GrannyFreeMeshBinding(pBinding);

				for (int k = 0; k < pMesh->MaterialBindingCount; ++k)
				{
					granny_material * pMaterial = pMesh->MaterialBindings[k].Material;
					if (!pMaterial)
						continue;
					sink.Text(tag + ".mat.name", pMaterial->Name);
					granny_texture * pDiffuse = api.GrannyGetMaterialTextureByType(pMaterial, GrannyDiffuseColorTexture);
					granny_texture * pOpacity = api.GrannyGetMaterialTextureByType(pMaterial, GrannyOpacityTexture);
					sink.Text(tag + ".mat.diffuse", pDiffuse ? pDiffuse->FromFileName : NULL);
					sink.Text(tag + ".mat.opacity", pOpacity ? pOpacity->FromFileName : NULL);
					if (pMaterial->MapCount > 1)
					{
						granny_texture * pBlend = api.GrannyGetMaterialTextureByType(pMaterial->Maps[1].Material, GrannyDiffuseColorTexture);
						sink.Text(tag + ".mat.blend1", pBlend ? pBlend->FromFileName : NULL);
					}
					granny_variant kResult;
					memset(&kResult, 0xAB, sizeof(kResult));
					const bool ok = api.GrannyFindMatchingMember(pMaterial->ExtendedData.Type, pMaterial->ExtendedData.Object, "Two-sided", &kResult);
					granny_int32 twoSided = -5;
					granny_data_type_definition kTwoSided[] = { { GrannyInt32Member, "Two-sided" }, { GrannyEndMember } };
					if (kResult.Type)
						api.GrannyConvertSingleObject(kResult.Type, kResult.Object, kTwoSided, &twoSided, 0);
					sink.Int(tag + ".mat.twosided.ok", ok ? 1 : 0);
					sink.Int(tag + ".mat.twosided.null", kResult.Type ? 0 : 1);
					sink.Int(tag + ".mat.twosided", twoSided);
				}
			}
		}
	}
};

struct SCase
{
	std::string					strModel;
	std::string					strLink;	// ayni iskelete baglanan ek model (sac/zirh)
	std::string					strLod;		// CopyMotion hedefi
	std::string					strAttach;	// kemige takilan model (silah)
	std::vector<std::string>	vecAnims;
	float						fTimeOffset = 0.0f;
};

static std::vector<SCase> LoadCases(const char * c_szPath)
{
	std::vector<SCase> cases;
	FILE * fp = fopen(c_szPath, "r");
	if (!fp)
		return cases;
	char line[4096];
	while (fgets(line, sizeof(line), fp))
	{
		std::string s(line);
		while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
			s.pop_back();
		if (s.empty() || s[0] == '#')
			continue;
		SCase c;
		size_t pos = 0;
		while (pos < s.size())
		{
			size_t sep = s.find('|', pos);
			if (sep == std::string::npos)
				sep = s.size();
			std::string field = s.substr(pos, sep - pos);
			pos = sep + 1;
			const size_t colon = field.find(':');
			if (colon == std::string::npos)
				continue;
			const std::string key = field.substr(0, colon);
			const std::string value = field.substr(colon + 1);
			if (key == "model") c.strModel = value;
			else if (key == "link") c.strLink = value;
			else if (key == "lod") c.strLod = value;
			else if (key == "attach") c.strAttach = value;
			else if (key == "anim") c.vecAnims.push_back(value);
			else if (key == "toffset") c.fTimeOffset = (float) atof(value.c_str());
		}
		if (!c.strModel.empty())
			cases.push_back(c);
	}
	fclose(fp);
	return cases;
}

static void NoLog(granny_log_message_type, granny_log_message_origin, char const *, granny_int32x, char const *, void *)
{
}

static granny_file * LoadGranny(const SApi & api, const std::string & dir, const std::string & rel)
{
	std::vector<char> bytes;
	if (!ReadFileBytes(dir + "/" + rel, bytes))
		return NULL;
	return api.GrannyReadEntireFileFromMemory((granny_int32x) bytes.size(), bytes.data());
}

static void RunCase(const SApi & api, const std::string & dir, const SCase & c, SSink & sink)
{
	SScenario s(api, sink);
	granny_file * pModelFile = LoadGranny(api, dir, c.strModel);
	sink.Int("load.model", pModelFile ? 1 : 0);
	if (!pModelFile)
		return;
	granny_file_info * pModelInfo = api.GrannyGetFileInfo(pModelFile);
	if (!pModelInfo || pModelInfo->ModelCount <= 0)
	{
		sink.Int("model.empty", 1);
		api.GrannyFreeFile(pModelFile);
		return;
	}
	s.RecordModelData(pModelInfo, "main");

	std::vector<granny_file *> vecAnimFiles;
	std::vector<granny_animation *> vecAnims;
	for (const std::string & a : c.vecAnims)
	{
		granny_file * f = LoadGranny(api, dir, a);
		sink.Int("load.anim", f ? 1 : 0);
		if (!f)
			continue;
		granny_file_info * fi = api.GrannyGetFileInfo(f);
		if (!fi || fi->AnimationCount <= 0)
		{
			api.GrannyFreeFile(f);
			continue;
		}
		vecAnimFiles.push_back(f);
		vecAnims.push_back(fi->Animations[0]);
	}

	granny_file * pLinkFile = c.strLink.empty() ? NULL : LoadGranny(api, dir, c.strLink);
	granny_file * pLodFile = c.strLod.empty() ? NULL : LoadGranny(api, dir, c.strLod);
	granny_file * pAttachFile = c.strAttach.empty() ? NULL : LoadGranny(api, dir, c.strAttach);
	granny_file_info * pLinkInfo = pLinkFile ? api.GrannyGetFileInfo(pLinkFile) : NULL;
	granny_file_info * pLodInfo = pLodFile ? api.GrannyGetFileInfo(pLodFile) : NULL;
	granny_file_info * pAttachInfo = pAttachFile ? api.GrannyGetFileInfo(pAttachFile) : NULL;
	if (pLinkInfo) s.RecordModelData(pLinkInfo, "link");
	if (pAttachInfo) s.RecordModelData(pAttachInfo, "attach");

	// Cynex: oyun model yukledikten sonra bu bolumleri birakiyor
	api.GrannyFreeFileSection(pModelFile, GrannyStandardRigidVertexSection);
	api.GrannyFreeFileSection(pModelFile, GrannyStandardRigidIndexSection);
	api.GrannyFreeFileSection(pModelFile, GrannyStandardDeformableIndexSection);
	api.GrannyFreeFileSection(pModelFile, GrannyStandardTextureSection);

	SInstance main, link, lod, attach;
	s.Create(main, pModelInfo->Models[0], NULL);
	if (pLinkInfo && pLinkInfo->ModelCount > 0)
		s.Create(link, pLinkInfo->Models[0], &main);
	if (pAttachInfo && pAttachInfo->ModelCount > 0)
		s.Create(attach, pAttachInfo->Models[0], NULL);

	granny_int32x attachBone = 0;
	granny_skeleton * pMainSkeleton = main.pModel->Skeleton;
	if (!api.GrannyFindBoneByName(pMainSkeleton, "equip_right_hand", &attachBone))
		attachBone = pMainSkeleton->BoneCount > 1 ? pMainSkeleton->BoneCount - 1 : 0;

	const float c_afSteps[] = { 1.0f / 60.0f, 1.0f / 30.0f, 0.05f, 1.0f / 45.0f, 0.1f, 0.0123f };
	const int stepCount = (int) (sizeof(c_afSteps) / sizeof(c_afSteps[0]));
	const size_t animCount = vecAnims.size();
	auto Anim = [&](size_t i) { return vecAnims[i % animCount]; };

	struct SEvent { float t; int type; int anim; float blend; int loop; float speed; };
	const SEvent c_akEvents[] =
	{
		{ 0.00f, 0, 0, 0.00f, 0, 1.0f },
		{ 0.80f, 0, 1, 0.20f, 1, 1.3f },
		{ 1.90f, 0, 2, 0.15f, 2, 0.8f },
		{ 2.20f, 3, 0, 0.00f, 0, 1.0f },
		{ 2.60f, 1, 0, 0.00f, 0, 1.0f },
		{ 3.20f, 0, 1, 0.00f, 0, 1.0f },
		{ 3.55f, 2, 0, 0.00f, 0, 1.0f },
		{ 3.90f, 0, 3, 0.25f, 0, -0.6f },
	};
	const int eventCount = (int) (sizeof(c_akEvents) / sizeof(c_akEvents[0]));
	int nextEvent = 0;

	float fRot[16];
	const float a = 0.6458f;
	const float ca = cosf(a), sa = sinf(a);
	const float rot[16] = { ca, sa, 0, 0, -sa, ca, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
	memcpy(fRot, rot, sizeof(rot));

	int frame = 0;
	float t = c.fTimeOffset;
	while (t < c.fTimeOffset + 4.6f && animCount > 0)
	{
		while (nextEvent < eventCount && c.fTimeOffset + c_akEvents[nextEvent].t <= t + 1e-6f)
		{
			const SEvent & e = c_akEvents[nextEvent++];
			main.fLocalTime = t;
			switch (e.type)
			{
				case 0:
					s.SetMotion(main, Anim(e.anim), e.blend, e.loop, e.speed);
					if (attach.pInstance)
						s.SetMotion(attach, Anim(e.anim), e.blend, e.loop, e.speed);
					break;
				case 1:
					s.ChangeMotion(main, Anim(e.anim), e.loop, e.speed);
					break;
				case 2:
					if (main.pControl)
						api.GrannySetControlRawLocalClock(main.pControl, api.GrannyGetControlLocalDuration(main.pControl));
					break;
				case 3:
					if (pLodInfo && pLodInfo->ModelCount > 0 && !lod.pInstance)
					{
						s.Create(lod, pLodInfo->Models[0], NULL);
						lod.fLocalTime = t;
						s.CopyMotion(lod, main, main.pControl ? Anim(1) : Anim(0));
					}
					break;
			}
		}

		const float dt = c_afSteps[frame % stepCount];
		main.fLocalTime = t;
		char tag[64];
		snprintf(tag, sizeof(tag), "f%03d.main", frame);
		s.Update(main, NULL, tag);
		s.Deform(main, tag);

		if (main.pControl)
		{
			const float state[3] = { api.GrannyGetControlRawLocalClock(main.pControl), api.GrannyGetControlLocalDuration(main.pControl), api.GrannyGetControlSpeed(main.pControl) };
			sink.Floats(std::string(tag) + ".ctrl", state, 3);
			sink.Int(std::string(tag) + ".complete", api.GrannyControlIsComplete(main.pControl) ? 1 : 0);
		}

		float m[16];
		api.GrannyUpdateModelMatrix(main.pInstance, dt, fRot, m, false);
		sink.Floats(std::string(tag) + ".modelmatrix", m, 16);
		api.GrannyUpdateModelMatrix(main.pInstance, dt * 7.0f, fRot, m, true);
		sink.Floats(std::string(tag) + ".modelmatrixinv", m, 16);

		if (link.pInstance)
		{
			link.fLocalTime = t;
			snprintf(tag, sizeof(tag), "f%03d.link", frame);
			s.Update(link, NULL, tag);
			s.Deform(link, tag);
		}
		if (lod.pInstance)
		{
			lod.fLocalTime = t;
			snprintf(tag, sizeof(tag), "f%03d.lod", frame);
			s.Update(lod, NULL, tag);
			s.Deform(lod, tag);
		}
		if (attach.pInstance)
		{
			attach.fLocalTime = t;
			snprintf(tag, sizeof(tag), "f%03d.attach", frame);
			s.Update(attach, api.GrannyGetWorldPose4x4(main.pWorldPose, attachBone), tag);
			s.Deform(attach, tag);
		}

		// Cynex: hizlandirilmamis yol + ofset
		if (frame % 7 == 3 && s.pSharedPose)
		{
			const granny_skeleton * pSkeleton = api.GrannyGetSourceSkeleton(main.pInstance);
			const float offset[16] = { 1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 12.5f, -3.0f, 40.0f, 1 };
			api.GrannySampleModelAnimations(main.pInstance, 0, pSkeleton->BoneCount, s.pSharedPose);
			granny_world_pose * pPose = api.GrannyNewWorldPose(pSkeleton->BoneCount);
			api.GrannyBuildWorldPose(pSkeleton, 0, pSkeleton->BoneCount, s.pSharedPose, offset, pPose);
			std::vector<float> world(16 * (size_t) pSkeleton->BoneCount);
			for (int b = 0; b < pSkeleton->BoneCount; ++b)
				memcpy(&world[16 * b], api.GrannyGetWorldPose4x4(pPose, b), 64);
			sink.Floats(std::string(tag) + ".plainworld", world.data(), world.size());
			api.GrannyFreeWorldPose(pPose);
		}

		t += dt;
		++frame;
	}

	s.Destroy(attach);
	s.Destroy(lod);
	s.Destroy(link);
	s.Destroy(main);
	if (s.pSharedPose)
		api.GrannyFreeLocalPose(s.pSharedPose);
	for (granny_file * f : vecAnimFiles)
		api.GrannyFreeFile(f);
	if (pLinkFile) api.GrannyFreeFile(pLinkFile);
	if (pLodFile) api.GrannyFreeFile(pLodFile);
	if (pAttachFile) api.GrannyFreeFile(pAttachFile);
	api.GrannyFreeFile(pModelFile);
}

/////////////////////////////////////////////////////////////////////////////
// Cynex: karsilastirma

struct SStat
{
	double	fMaxAbs = 0.0;
	double	fMaxRel = 0.0;
	long long llCount = 0;
	std::string strWorst;
};

static std::string Category(const std::string & label)
{
	const size_t dot = label.find('.');
	if (label.size() > 1 && label[0] == 'f' && dot != std::string::npos)
		return label.substr(dot + 1);
	return label;
}

// Cynex: matrislerde donus (birimsiz) ve oteleme ayri, deformda konum/normal/uv ayri raporlanir
static std::string SubCategory(const std::string & category, size_t index)
{
	const bool isMatrix = category.find("world") != std::string::npos || category.find("composite") != std::string::npos
		|| category.find("modelmatrix") != std::string::npos || category.find("rigidbone") != std::string::npos;
	if (isMatrix)
	{
		const size_t e = index % 16;
		return category + (e >= 12 && e <= 14 ? ".t" : ".r");
	}
	if (category.find("deform") != std::string::npos)
	{
		const size_t e = index % 8;
		return category + (e < 3 ? ".pos" : (e < 6 ? ".nrm" : ".uv"));
	}
	return category;
}

// Cynex: senaryo senaryo karsilastirma (bellekte tum kayitlar tutulmaz)
struct SComparer
{
	std::map<std::string, SStat>	kStats;
	long long						llExactMismatch = 0;
	long long						llTextMismatch = 0;
	long long						llStructMismatch = 0;
	FILE *							pReport = stdout;

	void AddCase(const TRecords & ra, const TRecords & rb, const std::string & caseName)
	{
		if (ra.size() != rb.size())
		{
			++llStructMismatch;
			fprintf(pReport, "YAPI FARKI %s: kayit %zu != %zu\n", caseName.c_str(), ra.size(), rb.size());
		}
		static const double s_fVerbose = getenv("CYNEX_HARNESS_VERBOSE") ? atof(getenv("CYNEX_HARNESS_VERBOSE")) : 0.0;
		const size_t n = std::min(ra.size(), rb.size());
		for (size_t i = 0; i < n; ++i)
		{
			const SRecord & x = ra[i];
			const SRecord & y = rb[i];
			if (x.strLabel != y.strLabel || x.iKind != y.iKind || x.vecData.size() != y.vecData.size())
			{
				++llStructMismatch;
				fprintf(pReport, "YAPI FARKI %s #%zu: %s(%zu) != %s(%zu)\n", caseName.c_str(), i, x.strLabel.c_str(), x.vecData.size(), y.strLabel.c_str(), y.vecData.size());
				break;
			}
			if (x.iKind == 1)
			{
				if (x.strText != y.strText)
				{
					++llTextMismatch;
					fprintf(pReport, "METIN FARKI %s %s: '%s' != '%s'\n", caseName.c_str(), x.strLabel.c_str(), x.strText.c_str(), y.strText.c_str());
				}
				continue;
			}
			if (x.iKind == 2)
			{
				if (memcmp(x.vecData.data(), y.vecData.data(), x.vecData.size() * 4) != 0)
				{
					++llExactMismatch;
					size_t first = 0;
					for (size_t k = 0; k < x.vecData.size(); ++k)
						if (memcmp(&x.vecData[k], &y.vecData[k], 4)) { first = k; break; }
					int32_t vx, vy;
					memcpy(&vx, &x.vecData[first], 4);
					memcpy(&vy, &y.vecData[first], 4);
					fprintf(pReport, "BIREBIR FARK %s %s [%zu]: %d != %d\n", caseName.c_str(), x.strLabel.c_str(), first, vx, vy);
				}
				continue;
			}
			const std::string category = Category(x.strLabel);
			double recordMax = 0.0;
			size_t recordIndex = 0;
			for (size_t k = 0; k < x.vecData.size(); ++k)
			{
				const double fx = x.vecData[k];
				const double fy = y.vecData[k];
				const double d = std::isfinite(fx) && std::isfinite(fy) ? fabs(fx - fy) : ((std::isnan(fx) && std::isnan(fy)) || fx == fy ? 0.0 : 1e30);
				const double rel = d / std::max(1.0, std::max(fabs(fx), fabs(fy)));
				SStat & st = kStats[SubCategory(category, k)];
				++st.llCount;
				if (rel > st.fMaxRel)
					st.fMaxRel = rel;
				if (rel > recordMax)
				{
					recordMax = rel;
					recordIndex = k;
				}
				if (d > st.fMaxAbs)
				{
					st.fMaxAbs = d;
					char sz[512];
					snprintf(sz, sizeof(sz), "%s %s[%zu] %.7g vs %.7g", caseName.c_str(), x.strLabel.c_str(), k, fx, fy);
					st.strWorst = sz;
				}
			}
			if (s_fVerbose > 0.0 && recordMax > s_fVerbose)
				fprintf(pReport, "  ayrinti %s %s[%zu] %.7g vs %.7g (goreli %.3g)\n", caseName.c_str(), x.strLabel.c_str(), recordIndex, x.vecData[recordIndex], y.vecData[recordIndex], recordMax);
		}
	}

	void Print()
	{
		fprintf(pReport, "\nsayisal karsilastirma (kategori: max mutlak / max goreli (|x|>1 icin) / deger sayisi / en kotu)\n");
		for (const auto & kv : kStats)
			fprintf(pReport, "  %-28s %12.4g %12.4g %12lld   %s\n", kv.first.c_str(), kv.second.fMaxAbs, kv.second.fMaxRel, kv.second.llCount, kv.second.strWorst.c_str());
		fprintf(pReport, "birebir fark: %lld, metin fark: %lld, yapi fark: %lld\n", llExactMismatch, llTextMismatch, llStructMismatch);
	}
};

static void WriteCase(FILE * fp, const std::string & name, const TRecords & recs)
{
	const uint32_t nameSize = (uint32_t) name.size();
	const uint32_t n = (uint32_t) recs.size();
	fwrite(&nameSize, 4, 1, fp);
	fwrite(name.data(), 1, name.size(), fp);
	fwrite(&n, 4, 1, fp);
	for (const SRecord & r : recs)
	{
		const uint32_t header[4] = { (uint32_t) r.iKind, (uint32_t) r.strLabel.size(), (uint32_t) r.strText.size(), (uint32_t) r.vecData.size() };
		fwrite(header, 4, 4, fp);
		fwrite(r.strLabel.data(), 1, r.strLabel.size(), fp);
		fwrite(r.strText.data(), 1, r.strText.size(), fp);
		if (!r.vecData.empty())
			fwrite(r.vecData.data(), 4, r.vecData.size(), fp);
	}
}

static bool ReadCase(FILE * fp, std::string & name, TRecords & recs)
{
	uint32_t nameSize = 0, n = 0;
	if (fread(&nameSize, 4, 1, fp) != 1)
		return false;
	name.resize(nameSize);
	if (nameSize && fread(&name[0], 1, nameSize, fp) != nameSize)
		return false;
	if (fread(&n, 4, 1, fp) != 1)
		return false;
	recs.clear();
	recs.resize(n);
	for (SRecord & r : recs)
	{
		uint32_t header[4];
		if (fread(header, 4, 4, fp) != 4)
			return false;
		r.iKind = (int) header[0];
		r.strLabel.resize(header[1]);
		r.strText.resize(header[2]);
		r.vecData.resize(header[3]);
		if (header[1] && fread(&r.strLabel[0], 1, header[1], fp) != header[1]) return false;
		if (header[2] && fread(&r.strText[0], 1, header[2], fp) != header[2]) return false;
		if (header[3] && fread(r.vecData.data(), 4, header[3], fp) != header[3]) return false;
	}
	return true;
}

static void SilenceLog(const SApi & api)
{
	granny_log_callback kLog = { NoLog, NULL };
	api.GrannySetLogCallback(&kLog);
}

static void FinishReport(SComparer & cmp, const char * c_szPath)
{
	cmp.Print();
	if (cmp.pReport != stdout && c_szPath)
	{
		fclose(cmp.pReport);
		FILE * fp = fopen(c_szPath, "r");
		char line[4096];
		while (fp && fgets(line, sizeof(line), fp))
			if (strncmp(line, "  ayrinti", 9) != 0)
				fputs(line, stdout);
		if (fp)
			fclose(fp);
	}
}

int main(int argc, char ** argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	if (argc >= 7 && !strcmp(argv[1], "compare"))
	{
		SApi oracle, mine;
		if (!LoadApi(argv[2], oracle) || !LoadApi(argv[4], mine))
			return 1;
		SilenceLog(oracle);
		SilenceLog(mine);
		const std::vector<SCase> cases = LoadCases(argv[6]);
		printf("%zu senaryo\n", cases.size());
		SComparer cmp;
		if (argc > 7)
			cmp.pReport = fopen(argv[7], "w");
		if (!cmp.pReport)
			cmp.pReport = stdout;
		for (size_t i = 0; i < cases.size(); ++i)
		{
			SSink a, b;
			RunCase(oracle, argv[3], cases[i], a);
			RunCase(mine, argv[5], cases[i], b);
			cmp.AddCase(a.kRecords, b.kRecords, cases[i].strModel);
			if ((i + 1) % 20 == 0)
				printf("[%zu/%zu]\n", i + 1, cases.size());
		}
		FinishReport(cmp, argc > 7 ? argv[7] : NULL);
		return 0;
	}
	if (argc >= 6 && !strcmp(argv[1], "dump"))
	{
		SApi api;
		if (!LoadApi(argv[2], api))
			return 1;
		SilenceLog(api);
		const std::vector<SCase> cases = LoadCases(argv[4]);
		FILE * fp = fopen(argv[5], "wb");
		if (!fp)
			return 1;
		for (const SCase & c : cases)
		{
			SSink sink;
			RunCase(api, argv[3], c, sink);
			WriteCase(fp, c.strModel, sink.kRecords);
		}
		fclose(fp);
		return 0;
	}
	if (argc >= 4 && !strcmp(argv[1], "diff"))
	{
		FILE * fa = fopen(argv[2], "rb");
		FILE * fb = fopen(argv[3], "rb");
		if (!fa || !fb)
			return 1;
		SComparer cmp;
		if (argc > 4)
			cmp.pReport = fopen(argv[4], "w");
		if (!cmp.pReport)
			cmp.pReport = stdout;
		std::string na, nb;
		TRecords ra, rb;
		size_t count = 0;
		while (ReadCase(fa, na, ra) && ReadCase(fb, nb, rb))
		{
			cmp.AddCase(ra, rb, na);
			++count;
		}
		fclose(fa);
		fclose(fb);
		printf("%zu senaryo\n", count);
		FinishReport(cmp, argc > 4 ? argv[4] : NULL);
		return 0;
	}
	if (argc >= 9 && !strcmp(argv[1], "debug"))
	{
		// Cynex: debug <oracle.dll> <origdir> <cynex.dll> <convdir> <model> <anim> <clock> [loop] [speed] [start]
		SApi oracle, mine;
		if (!LoadApi(argv[2], oracle) || !LoadApi(argv[4], mine))
			return 1;
		SilenceLog(oracle);
		SilenceLog(mine);
		const float fClock = (float) atof(argv[8]);
		const int loop = argc > 9 ? atoi(argv[9]) : 0;
		const float fSpeed = argc > 10 ? (float) atof(argv[10]) : 1.0f;
		const float fStart = argc > 11 ? (float) atof(argv[11]) : 0.0f;
		const SApi * apis[2] = { &oracle, &mine };
		const char * dirs[2] = { argv[3], argv[5] };
		std::vector<float> world[2];
		granny_skeleton * skeletons[2] = { NULL, NULL };
		granny_animation * anims[2] = { NULL, NULL };
		for (int i = 0; i < 2; ++i)
		{
			const SApi & api = *apis[i];
			granny_file * fm = LoadGranny(api, dirs[i], argv[6]);
			granny_file * fa = LoadGranny(api, dirs[i], argv[7]);
			if (!fm || !fa)
			{
				printf("yuklenemedi (%d)\n", i);
				return 1;
			}
			granny_model * pModel = api.GrannyGetFileInfo(fm)->Models[0];
			anims[i] = api.GrannyGetFileInfo(fa)->Animations[0];
			skeletons[i] = pModel->Skeleton;
			granny_model_instance * pInstance = api.GrannyInstantiateModel(pModel);
			granny_control * c = api.GrannyPlayControlledAnimation(fStart, anims[i], pInstance);
			if (c)
			{
				api.GrannySetControlLoopCount(c, loop);
				api.GrannySetControlSpeed(c, fSpeed);
			}
			// Cynex: istege bagli ikinci animasyon - 0.8'de 0.2 sn karisimla gecis (senaryodaki gibi)
			if (argc > 12 && c)
			{
				granny_file * fb = LoadGranny(api, dirs[i], argv[12]);
				granny_animation * pSecond = fb ? api.GrannyGetFileInfo(fb)->Animations[0] : NULL;
				api.GrannySetModelClock(pInstance, 0.8f);
				api.GrannySetControlEaseOutCurve(c, 0.8f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);
				api.GrannySetControlEaseIn(c, false);
				api.GrannySetControlEaseOut(c, true);
				api.GrannyCompleteControlAt(c, 1.0f);
				granny_control * c2 = pSecond ? api.GrannyPlayControlledAnimation(0.8f, pSecond, pInstance) : NULL;
				if (c2)
				{
					api.GrannySetControlSpeed(c2, 1.3f);
					api.GrannySetControlLoopCount(c2, 1);
					api.GrannySetControlEaseIn(c2, true);
					api.GrannySetControlEaseOut(c2, false);
					api.GrannySetControlEaseInCurve(c2, 0.8f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
				}
			}
			api.GrannySetModelClock(pInstance, fClock);
			granny_local_pose * pLocal = api.GrannyNewLocalPose(pModel->Skeleton->BoneCount);
			granny_world_pose * pWorld = api.GrannyNewWorldPose(pModel->Skeleton->BoneCount);
			api.GrannySampleModelAnimations(pInstance, 0, pModel->Skeleton->BoneCount, pLocal);
			api.GrannyBuildWorldPose(pModel->Skeleton, 0, pModel->Skeleton->BoneCount, pLocal, NULL, pWorld);
			for (int b = 0; b < pModel->Skeleton->BoneCount; ++b)
			{
				const float * m = api.GrannyGetWorldPose4x4(pWorld, b);
				world[i].insert(world[i].end(), m, m + 16);
			}
			printf("%s: control=%p raw=%g\n", i ? "cynex" : "oracle", (void *) c, c ? api.GrannyGetControlRawLocalClock(c) : 0.0f);
		}
		granny_skeleton * sk = skeletons[0];
		granny_track_group * g = anims[0]->TrackGroups[0];
		for (int tg = 0; tg < anims[0]->TrackGroupCount; ++tg)
			if (!_stricmp(anims[0]->TrackGroups[tg]->Name, sk->Name))
				g = anims[0]->TrackGroups[tg];
		printf("anim duration %g, tracks %d, flags 0x%X\n", anims[0]->Duration, g->TransformTrackCount, g->Flags);
		int shown = 0;
		for (int b = 0; b < sk->BoneCount && shown < 6; ++b)
		{
			float d = 0;
			for (int k = 0; k < 16; ++k)
				d = std::max(d, fabsf(world[0][16 * b + k] - world[1][16 * b + k]));
			const int parent = sk->Bones[b].ParentIndex;
			float pd = 0;
			if (parent >= 0)
				for (int k = 0; k < 16; ++k)
					pd = std::max(pd, fabsf(world[0][16 * parent + k] - world[1][16 * parent + k]));
			static const float s_fEps = getenv("CYNEX_DEBUG_EPS") ? (float) atof(getenv("CYNEX_DEBUG_EPS")) : 1e-3f;
			if (d > s_fEps && pd <= s_fEps)
			{
				++shown;
				printf("bone %d '%s' parent %d diff %g\n", b, sk->Bones[b].Name, parent, d);
				for (int t = 0; t < g->TransformTrackCount; ++t)
				{
					granny_transform_track & tr = g->TransformTracks[t];
					if (strcmp(tr.Name, sk->Bones[b].Name))
						continue;
					const granny_curve2 * curves[3] = { &tr.PositionCurve, &tr.OrientationCurve, &tr.ScaleShearCurve };
					const char * names[3] = { "pos", "ori", "ss" };
					for (int ci = 0; ci < 3; ++ci)
					{
						const granny_curve2 & cv = *curves[ci];
						if (!cv.CurveData.Object)
						{
							printf("   %s: bos\n", names[ci]);
							continue;
						}
						const granny_curve_data_header * h = (const granny_curve_data_header *) cv.CurveData.Object;
						printf("   %s: format %d degree %d", names[ci], h->Format, h->Degree);
						if (h->Format == 1)
						{
							const granny_curve_data_da_k32f_c32f * dk = (const granny_curve_data_da_k32f_c32f *) cv.CurveData.Object;
							printf(" knots %d controls %d first %g last %g", dk->KnotCount, dk->ControlCount, dk->KnotCount ? dk->Knots[0] : 0.0f, dk->KnotCount ? dk->Knots[dk->KnotCount - 1] : 0.0f);
						}
						printf("\n");
					}
				}
				for (int r = 0; r < 3; ++r)
					printf("   row%d oracle %9.5f %9.5f %9.5f | cynex %9.5f %9.5f %9.5f\n", r, world[0][16 * b + 4 * r], world[0][16 * b + 4 * r + 1], world[0][16 * b + 4 * r + 2], world[1][16 * b + 4 * r], world[1][16 * b + 4 * r + 1], world[1][16 * b + 4 * r + 2]);
				printf("   oracle row3 %g %g %g | cynex %g %g %g\n", world[0][16 * b + 12], world[0][16 * b + 13], world[0][16 * b + 14], world[1][16 * b + 12], world[1][16 * b + 13], world[1][16 * b + 14]);
			}
		}
		return 0;
	}
	if (argc >= 5 && !strcmp(argv[1], "bench"))
	{
		SApi api;
		if (!LoadApi(argv[2], api))
			return 1;
		const std::vector<SCase> cases = LoadCases(argv[4]);
		const int repeat = argc > 5 ? atoi(argv[5]) : 3;
		SilenceLog(api);
		LARGE_INTEGER freq, t0, t1;
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&t0);
		for (int r = 0; r < repeat; ++r)
			for (const SCase & c : cases)
			{
				SSink sink;
				sink.bEnabled = false;
				RunCase(api, argv[3], c, sink);
			}
		QueryPerformanceCounter(&t1);
		printf("%s: %d tekrar, %zu senaryo, %.1f ms\n", argv[2], repeat, cases.size(), (t1.QuadPart - t0.QuadPart) * 1000.0 / freq.QuadPart);
		return 0;
	}
	fprintf(stderr, "kullanim:\n  granny_harness compare <oracle.dll> <origdir> <cynex.dll> <convdir> <cases.txt> [rapor]\n  granny_harness dump <dll> <datadir> <cases.txt> <cikti.bin>\n  granny_harness diff <a.bin> <b.bin> [rapor]\n  granny_harness bench <dll> <datadir> <cases.txt> [tekrar]\n");
	return 2;
}
