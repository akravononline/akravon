// Cynex: mesh sorgulari, vertex/index kopyalama, mesh binding, CPU deform, materyal dokusu
#include "granny_internal.h"

#include <cctype>

using namespace CynexGranny;

namespace granny
{
	struct mesh_binding
	{
		const granny_mesh *		Mesh;
		const granny_skeleton *	FromSkeleton;
		const granny_skeleton *	ToSkeleton;
		int						BoneCount;
		granny_int32x *			FromBoneIndices;
		granny_int32x *			ToBoneIndices;
	};

	struct mesh_deformer
	{
		enum
		{
			MAX_TAIL_OPS = 32,
			MAX_BONES_PER_VERTEX = 8,
		};

		granny_deformation_type			DeformationType;
		int								InputStride;
		int								OutputStride;
		int								PositionIn;
		int								NormalIn;
		int								WeightsIn;
		int								IndicesIn;
		granny_member_type				WeightType;
		granny_member_type				IndexType;
		int								BonesPerVertex;
		int								PositionOut;
		int								NormalOut;
		bool							IsPWNT3432ToPNT332;
		int								TailCount;
		CynexGranny::SVertexCopyOp		Tail[MAX_TAIL_OPS];
	};
}

namespace
{
	const granny_tri_topology * Topology(const granny_mesh * c_pMesh)
	{
		return c_pMesh ? c_pMesh->PrimaryTopology : NULL;
	}

	bool IsReal32x3(const granny_data_type_definition * c_pMember)
	{
		return c_pMember && c_pMember->Type == GrannyReal32Member && c_pMember->ArrayWidth >= 3;
	}

	int MemberOffset(const granny_data_type_definition * c_pType, const char * c_szName, const granny_data_type_definition ** ppMember)
	{
		int offset = 0;
		const granny_data_type_definition * pMember = FindMember(c_pType, c_szName, &offset);
		if (ppMember)
			*ppMember = pMember;
		return pMember ? offset : -1;
	}

	CYNEX_FORCEINLINE float ReadWeight(granny_member_type eType, const uint8_t * p, int i)
	{
		switch (eType)
		{
			case GrannyNormalUInt8Member:	return p[i] * (1.0f / 255.0f);
			case GrannyUInt8Member:			return p[i] * (1.0f / 255.0f);
			case GrannyNormalUInt16Member:	{ uint16_t v; memcpy(&v, p + 2 * i, 2); return v * (1.0f / 65535.0f); }
			case GrannyReal32Member:		{ float v; memcpy(&v, p + 4 * i, 4); return v; }
			default:						return 0.0f;
		}
	}

	CYNEX_FORCEINLINE int ReadIndex(granny_member_type eType, const uint8_t * p, int i)
	{
		switch (eType)
		{
			case GrannyUInt8Member:
			case GrannyInt8Member:			return p[i];
			case GrannyUInt16Member:
			case GrannyInt16Member:			{ uint16_t v; memcpy(&v, p + 2 * i, 2); return v; }
			case GrannyUInt32Member:
			case GrannyInt32Member:			{ uint32_t v; memcpy(&v, p + 4 * i, 4); return (int) v; }
			default:						return 0;
		}
	}

	CYNEX_FORCEINLINE void AccumulatePoint(const float * M, float w, const float * p, float * acc)
	{
		acc[0] += w * (p[0] * M[0] + p[1] * M[4] + p[2] * M[8] + M[12]);
		acc[1] += w * (p[0] * M[1] + p[1] * M[5] + p[2] * M[9] + M[13]);
		acc[2] += w * (p[0] * M[2] + p[1] * M[6] + p[2] * M[10] + M[14]);
	}

	CYNEX_FORCEINLINE void AccumulateVector(const float * M, float w, const float * v, float * acc)
	{
		acc[0] += w * (v[0] * M[0] + v[1] * M[4] + v[2] * M[8]);
		acc[1] += w * (v[0] * M[1] + v[1] * M[5] + v[2] * M[9]);
		acc[2] += w * (v[0] * M[2] + v[1] * M[6] + v[2] * M[10]);
	}

	// Cynex: oyunun deforme ettigi tek tip - PWNT3432 -> PNT332 hizli yol
	void DeformPWNT3432ToPNT332(const granny_int32x * c_piMatrixIndices, const float * c_pMatrices, int iVertexCount, const uint8_t * c_pSrc, uint8_t * pDst)
	{
		for (int v = 0; v < iVertexCount; ++v, c_pSrc += 40, pDst += 32)
		{
			float p[3], n[3];
			memcpy(p, c_pSrc, 12);
			memcpy(n, c_pSrc + 20, 12);
			const uint8_t * w = c_pSrc + 12;
			const uint8_t * bi = c_pSrc + 16;

			float accP[3] = { 0.0f, 0.0f, 0.0f };
			float accN[3] = { 0.0f, 0.0f, 0.0f };
			for (int k = 0; k < 4; ++k)
			{
				if (!w[k])
					continue;
				const float fWeight = w[k] * (1.0f / 255.0f);
				const int bone = c_piMatrixIndices ? c_piMatrixIndices[bi[k]] : bi[k];
				const float * M = c_pMatrices + 16 * bone;
				AccumulatePoint(M, fWeight, p, accP);
				AccumulateVector(M, fWeight, n, accN);
			}
			memcpy(pDst, accP, 12);
			memcpy(pDst + 12, accN, 12);
			memcpy(pDst + 24, c_pSrc + 32, 8);
		}
	}

	struct STextureUsage
	{
		const char *					szUsage;
		granny_material_texture_type	eType;
	};

	// Cynex: granny2.dll'den kara kutu ile cikarilan kullanim tablosu (buyuk/kucuk harf duyarsiz)
	const STextureUsage c_akTextureUsages[] =
	{
		{ "Ambient Color", GrannyAmbientColorTexture },
		{ "Umgebungsfarbe", GrannyAmbientColorTexture },
		{ "BaseColor", GrannyAmbientColorTexture },
		{ "ambience", GrannyAmbientColorTexture },
		{ "ambient", GrannyAmbientColorTexture },
		{ "color", GrannyDiffuseColorTexture },
		{ "Diffuse Color", GrannyDiffuseColorTexture },
		{ "RenderMaterial", GrannyDiffuseColorTexture },
		{ "Streufarben", GrannyDiffuseColorTexture },
		{ "BaseColor", GrannyDiffuseColorTexture },
		{ "diffuse", GrannyDiffuseColorTexture },
		{ "specularColor", GrannySpecularColorTexture },
		{ "Specular Color", GrannySpecularColorTexture },
		{ "Specular Level", GrannySpecularColorTexture },
		{ "Glanzfarben", GrannySpecularColorTexture },
		{ "Specularity", GrannySpecularColorTexture },
		{ "specular", GrannySpecularColorTexture },
		{ "incandescence", GrannySelfIlluminationTexture },
		{ "Self-Illumination", GrannySelfIlluminationTexture },
		{ "Luminosity", GrannySelfIlluminationTexture },
		{ "radiance", GrannySelfIlluminationTexture },
		{ "Opacity", GrannyOpacityTexture },
		{ "Transparency", GrannyOpacityTexture },
		{ "normalCamera", GrannyBumpHeightTexture },
		{ "Bump", GrannyBumpHeightTexture },
		{ "Relief", GrannyBumpHeightTexture },
		{ "3D-Verschiebung", GrannyBumpHeightTexture },
		{ "normspec", GrannyBumpHeightTexture },
		{ "Reflection", GrannyReflectionTexture },
		{ "Reflexion", GrannyReflectionTexture },
		{ "ReflectionImage", GrannyReflectionTexture },
		{ "reflectivity", GrannyReflectionTexture },
		{ "Refraction", GrannyRefractionTexture },
		{ "Refraktion", GrannyRefractionTexture },
		{ "RefractionImage", GrannyRefractionTexture },
		{ "index_of_refraction", GrannyRefractionTexture },
		{ "Displacement", GrannyDisplacementTexture },
	};

	bool EqualsNoCase(const char * a, const char * b)
	{
		for (;; ++a, ++b)
		{
			const int ca = tolower((unsigned char) *a);
			const int cb = tolower((unsigned char) *b);
			if (ca != cb)
				return false;
			if (!ca)
				return true;
		}
	}

	bool UsageMatches(const char * c_szUsage, granny_material_texture_type eType)
	{
		if (!c_szUsage)
			return false;
		for (const STextureUsage & r : c_akTextureUsages)
			if (r.eType == eType && EqualsNoCase(r.szUsage, c_szUsage))
				return true;
		return false;
	}

	granny_texture * FindAnyTexture(const granny_material * c_pMaterial, int iDepth)
	{
		if (!c_pMaterial || iDepth > 16)
			return NULL;
		if (c_pMaterial->Texture)
			return c_pMaterial->Texture;
		for (int i = 0; i < c_pMaterial->MapCount; ++i)
		{
			granny_texture * pTexture = FindAnyTexture(c_pMaterial->Maps[i].Material, iDepth + 1);
			if (pTexture)
				return pTexture;
		}
		return NULL;
	}
}

GRANNY_DYNLINK(bool) GrannyMeshIsRigid(granny_mesh const * Mesh)
{
	return Mesh ? Mesh->BoneBindingCount <= 1 : true;
}

GRANNY_DYNLINK(granny_int32x) GrannyGetMeshVertexCount(granny_mesh const * Mesh)
{
	return (Mesh && Mesh->PrimaryVertexData) ? Mesh->PrimaryVertexData->VertexCount : 0;
}

GRANNY_DYNLINK(granny_data_type_definition *) GrannyGetMeshVertexType(granny_mesh const * Mesh)
{
	return (Mesh && Mesh->PrimaryVertexData) ? Mesh->PrimaryVertexData->VertexType : NULL;
}

GRANNY_DYNLINK(void *) GrannyGetMeshVertices(granny_mesh const * Mesh)
{
	return (Mesh && Mesh->PrimaryVertexData) ? Mesh->PrimaryVertexData->Vertices : NULL;
}

GRANNY_DYNLINK(granny_int32x) GrannyGetMeshIndexCount(granny_mesh const * Mesh)
{
	const granny_tri_topology * pTopology = Topology(Mesh);
	if (!pTopology)
		return 0;
	return pTopology->Indices16 ? pTopology->Index16Count : pTopology->IndexCount;
}

GRANNY_DYNLINK(granny_int32x) GrannyGetMeshTriangleGroupCount(granny_mesh const * Mesh)
{
	const granny_tri_topology * pTopology = Topology(Mesh);
	return pTopology ? pTopology->GroupCount : 0;
}

GRANNY_DYNLINK(granny_tri_material_group *) GrannyGetMeshTriangleGroups(granny_mesh const * Mesh)
{
	const granny_tri_topology * pTopology = Topology(Mesh);
	return pTopology ? pTopology->Groups : NULL;
}

GRANNY_DYNLINK(void) GrannyCopyMeshIndices(granny_mesh const * Mesh, granny_int32x BytesPerIndex, void * DestIndices)
{
	const granny_tri_topology * pTopology = Topology(Mesh);
	if (!pTopology || !DestIndices)
		return;

	const int count = GrannyGetMeshIndexCount(Mesh);
	uint8_t * pDst = static_cast<uint8_t *>(DestIndices);
	for (int i = 0; i < count; ++i)
	{
		const uint32_t index = pTopology->Indices16 ? pTopology->Indices16[i] : (uint32_t) pTopology->Indices[i];
		switch (BytesPerIndex)
		{
			case 1: pDst[i] = (uint8_t) index; break;
			case 2: { const uint16_t v = (uint16_t) index; memcpy(pDst + 2 * i, &v, 2); break; }
			case 4: memcpy(pDst + 4 * i, &index, 4); break;
			default: return;
		}
	}
}

GRANNY_DYNLINK(void) GrannyCopyMeshVertices(granny_mesh const * Mesh, granny_data_type_definition const * VertexType, void * DestVertices)
{
	const granny_data_type_definition * pSrcType = GrannyGetMeshVertexType(Mesh);
	const uint8_t * pSrc = static_cast<const uint8_t *>(GrannyGetMeshVertices(Mesh));
	const int count = GrannyGetMeshVertexCount(Mesh);
	if (!pSrcType || !pSrc || !VertexType || !DestVertices || count <= 0)
		return;

	const int srcStride = GetObjectSize(pSrcType);
	const int dstStride = GetObjectSize(VertexType);
	uint8_t * pDst = static_cast<uint8_t *>(DestVertices);

	SVertexCopyOp akOps[64];
	const int opCount = BuildVertexCopyPlan(pSrcType, VertexType, akOps, 64, NULL, 0);
	for (int v = 0; v < count; ++v)
		RunVertexCopyPlan(akOps, opCount, pSrc + v * srcStride, pDst + v * dstStride);
}

GRANNY_DYNLINK(granny_mesh_binding *) GrannyNewMeshBinding(granny_mesh const * Mesh, granny_skeleton const * FromSkeleton, granny_skeleton const * ToSkeleton)
{
	if (!Mesh)
		return NULL;

	const int count = Mesh->BoneBindingCount > 0 ? Mesh->BoneBindingCount : 0;
	granny_mesh_binding * pBinding = static_cast<granny_mesh_binding *>(AlignedAlloc(sizeof(granny_mesh_binding), 16));
	pBinding->Mesh = Mesh;
	pBinding->FromSkeleton = FromSkeleton;
	pBinding->ToSkeleton = ToSkeleton;
	pBinding->BoneCount = count;
	// Cynex: kemiksiz mesh'te de [0] okunabilsin
	pBinding->FromBoneIndices = AllocArray<granny_int32x>(count > 0 ? count : 1);
	pBinding->ToBoneIndices = AllocArray<granny_int32x>(count > 0 ? count : 1);
	pBinding->FromBoneIndices[0] = 0;
	pBinding->ToBoneIndices[0] = 0;

	for (int i = 0; i < count; ++i)
	{
		const char * c_szBone = Mesh->BoneBindings[i].BoneName;
		granny_int32x from = 0;
		granny_int32x to = 0;
		if (!GrannyFindBoneByName(FromSkeleton, c_szBone, &from))
			from = 0;
		if (!GrannyFindBoneByName(ToSkeleton, c_szBone, &to))
		{
			to = 0;
			CYNEX_LOG(GrannyErrorLogMessage, GrannyMeshBindingLogMessage, "Unable to find bone \"%s\" (mesh \"%s\") in skeleton \"%s\"",
				c_szBone ? c_szBone : "", Mesh->Name ? Mesh->Name : "", (ToSkeleton && ToSkeleton->Name) ? ToSkeleton->Name : "");
		}
		pBinding->FromBoneIndices[i] = from;
		pBinding->ToBoneIndices[i] = to;
	}
	return pBinding;
}

GRANNY_DYNLINK(void) GrannyFreeMeshBinding(granny_mesh_binding * Binding)
{
	if (!Binding)
		return;
	AlignedFree(Binding->FromBoneIndices);
	AlignedFree(Binding->ToBoneIndices);
	AlignedFree(Binding);
}

GRANNY_DYNLINK(granny_int32x const *) GrannyGetMeshBindingToBoneIndices(granny_mesh_binding const * Binding)
{
	return Binding ? Binding->ToBoneIndices : NULL;
}

GRANNY_DYNLINK(granny_mesh_deformer *) GrannyNewMeshDeformer(granny_data_type_definition const * InputVertexLayout, granny_data_type_definition const * OutputVertexLayout, granny_deformation_type DeformationType, granny_deformer_tail_flags TailFlag)
{
	(void) TailFlag;
	if (!InputVertexLayout || !OutputVertexLayout)
		return NULL;

	const granny_data_type_definition * pPosIn = NULL;
	const granny_data_type_definition * pPosOut = NULL;
	const int posIn = MemberOffset(InputVertexLayout, GrannyVertexPositionName, &pPosIn);
	const int posOut = MemberOffset(OutputVertexLayout, GrannyVertexPositionName, &pPosOut);
	if (!IsReal32x3(pPosIn) || !IsReal32x3(pPosOut))
	{
		CYNEX_LOG(GrannyErrorLogMessage, GrannyDeformerLogMessage, "Deformer needs Real32[3] positions in both layouts");
		return NULL;
	}

	granny_mesh_deformer * pDeformer = static_cast<granny_mesh_deformer *>(AlignedAlloc(sizeof(granny_mesh_deformer), 16));
	memset(pDeformer, 0, sizeof(*pDeformer));
	pDeformer->DeformationType = DeformationType;
	pDeformer->InputStride = GetObjectSize(InputVertexLayout);
	pDeformer->OutputStride = GetObjectSize(OutputVertexLayout);
	pDeformer->PositionIn = posIn;
	pDeformer->PositionOut = posOut;

	const granny_data_type_definition * pNormalIn = NULL;
	const granny_data_type_definition * pNormalOut = NULL;
	pDeformer->NormalIn = MemberOffset(InputVertexLayout, GrannyVertexNormalName, &pNormalIn);
	pDeformer->NormalOut = MemberOffset(OutputVertexLayout, GrannyVertexNormalName, &pNormalOut);
	if (DeformationType == GrannyDeformPosition || !IsReal32x3(pNormalIn) || !IsReal32x3(pNormalOut))
	{
		pDeformer->NormalIn = -1;
		pDeformer->NormalOut = -1;
	}

	const granny_data_type_definition * pWeights = NULL;
	const granny_data_type_definition * pIndices = NULL;
	pDeformer->WeightsIn = MemberOffset(InputVertexLayout, GrannyVertexBoneWeightsName, &pWeights);
	pDeformer->IndicesIn = MemberOffset(InputVertexLayout, GrannyVertexBoneIndicesName, &pIndices);
	pDeformer->WeightType = pWeights ? pWeights->Type : GrannyReal32Member;
	pDeformer->IndexType = pIndices ? pIndices->Type : GrannyUInt8Member;
	int bones = pIndices ? (pIndices->ArrayWidth > 0 ? pIndices->ArrayWidth : 1) : 0;
	if (bones > granny_mesh_deformer::MAX_BONES_PER_VERTEX)
		bones = granny_mesh_deformer::MAX_BONES_PER_VERTEX;
	pDeformer->BonesPerVertex = bones;

	// Cynex: konum/normal disindaki uyeler isimle kopyalanir
	const char * const c_aszSkip[] = { GrannyVertexPositionName, GrannyVertexNormalName };
	const int skipCount = pDeformer->NormalOut >= 0 ? 2 : 1;
	pDeformer->TailCount = BuildVertexCopyPlan(InputVertexLayout, OutputVertexLayout, pDeformer->Tail, granny_mesh_deformer::MAX_TAIL_OPS, c_aszSkip, skipCount);

	pDeformer->IsPWNT3432ToPNT332 = pDeformer->InputStride == 40 && pDeformer->OutputStride == 32
		&& posIn == 0 && pDeformer->WeightsIn == 12 && pDeformer->IndicesIn == 16 && pDeformer->NormalIn == 20
		&& pWeights && pWeights->Type == GrannyNormalUInt8Member && pWeights->ArrayWidth == 4
		&& pIndices && pIndices->Type == GrannyUInt8Member && pIndices->ArrayWidth == 4
		&& posOut == 0 && pDeformer->NormalOut == 12
		&& pDeformer->TailCount == 1 && pDeformer->Tail[0].iSize == 8 && pDeformer->Tail[0].iSrcOffset == 32 && pDeformer->Tail[0].iDstOffset == 24;

	return pDeformer;
}

GRANNY_DYNLINK(void) GrannyFreeMeshDeformer(granny_mesh_deformer * Deformer)
{
	AlignedFree(Deformer);
}

GRANNY_DYNLINK(void) GrannyDeformVertices(granny_mesh_deformer const * Deformer, granny_int32x const * MatrixIndices, granny_real32 const * MatrixBuffer4x4, granny_int32x VertexCount, void const * SourceVertices, void * DestVertices)
{
	if (!Deformer || !MatrixBuffer4x4 || !SourceVertices || !DestVertices || VertexCount <= 0)
		return;

	const uint8_t * pSrc = static_cast<const uint8_t *>(SourceVertices);
	uint8_t * pDst = static_cast<uint8_t *>(DestVertices);

	if (Deformer->IsPWNT3432ToPNT332)
	{
		DeformPWNT3432ToPNT332(MatrixIndices, MatrixBuffer4x4, VertexCount, pSrc, pDst);
		return;
	}

	const bool hasNormal = Deformer->NormalIn >= 0;
	for (int v = 0; v < VertexCount; ++v, pSrc += Deformer->InputStride, pDst += Deformer->OutputStride)
	{
		float p[3], n[3] = { 0.0f, 0.0f, 0.0f };
		memcpy(p, pSrc + Deformer->PositionIn, 12);
		if (hasNormal)
			memcpy(n, pSrc + Deformer->NormalIn, 12);

		float accP[3] = { 0.0f, 0.0f, 0.0f };
		float accN[3] = { 0.0f, 0.0f, 0.0f };

		if (Deformer->BonesPerVertex == 0)
		{
			const int bone = MatrixIndices ? MatrixIndices[0] : 0;
			const float * M = MatrixBuffer4x4 + 16 * bone;
			AccumulatePoint(M, 1.0f, p, accP);
			AccumulateVector(M, 1.0f, n, accN);
		}
		else
		{
			const uint8_t * pIndices = pSrc + Deformer->IndicesIn;
			const uint8_t * pWeights = Deformer->WeightsIn >= 0 ? pSrc + Deformer->WeightsIn : NULL;
			for (int k = 0; k < Deformer->BonesPerVertex; ++k)
			{
				const float w = pWeights ? ReadWeight(Deformer->WeightType, pWeights, k) : (k == 0 ? 1.0f : 0.0f);
				if (w == 0.0f)
					continue;
				const int index = ReadIndex(Deformer->IndexType, pIndices, k);
				const int bone = MatrixIndices ? MatrixIndices[index] : index;
				const float * M = MatrixBuffer4x4 + 16 * bone;
				AccumulatePoint(M, w, p, accP);
				AccumulateVector(M, w, n, accN);
			}
		}

		memcpy(pDst + Deformer->PositionOut, accP, 12);
		if (Deformer->NormalOut >= 0)
			memcpy(pDst + Deformer->NormalOut, accN, 12);
		RunVertexCopyPlan(Deformer->Tail, Deformer->TailCount, pSrc, pDst);
	}
}

GRANNY_DYNLINK(granny_texture *) GrannyGetMaterialTextureByType(granny_material const * Material, granny_material_texture_type Type)
{
	if (!Material)
		return NULL;
	for (int i = 0; i < Material->MapCount; ++i)
	{
		const granny_material_map & rMap = Material->Maps[i];
		if (!UsageMatches(rMap.Usage, Type))
			continue;
		granny_texture * pTexture = FindAnyTexture(rMap.Material, 0);
		if (pTexture)
			return pTexture;
	}
	return NULL;
}
