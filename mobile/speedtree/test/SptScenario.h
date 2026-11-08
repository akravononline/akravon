///////////////////////////////////////////////////////////////////////
//	SptScenario.h
//
//	Cynex: dogrulama senaryosu. Ayni sablon hem gercek SpeedTreeRT.dll sinifi hem
//	Cynex: cynex_speedtree (test icin CCynexSpeedTreeRT adiyla) ile derlenir; her
//	Cynex: donen dizi anahtarli kayit olarak SptResult'a yazilir, sonra karsilastirilir.

#pragma once

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <string>

#include "SptResult.h"

namespace SptScenario
{
	static const float c_afGameLight[16] = { -0.707f, -0.300f, 0.707f, 1, 1, 1, 0.5f, 0.5f, 0.5f, 1, 1, 1, 0.0f, 1, 0, 0 };

	template <class TTree>
	void RecordIndexed(CSptResult& r, const char* pKey, const typename TTree::SGeometry::SIndexed& s, bool bArrays)
	{
		r.Int(pKey, "lod", s.m_nDiscreteLodLevel);
		r.Int(pKey, "strips", s.m_usNumStrips);
		for (unsigned int k = 0; k < s.m_usNumStrips && s.m_pStripLengths; ++k)
		{
			r.Int(pKey, "striplen", s.m_pStripLengths[k]);
			if (bArrays && s.m_pStrips && s.m_pStrips[k])
				r.U16s(pKey, "strip", s.m_pStrips[k], s.m_pStripLengths[k]);
		}
		r.Int(pKey, "vc", s.m_usVertexCount);
		const unsigned int n = s.m_usVertexCount;
		r.Ptr(pKey, "colors", s.m_pColors);
		r.Ptr(pKey, "normals", s.m_pNormals);
		r.Ptr(pKey, "binormals", s.m_pBinormals);
		r.Ptr(pKey, "tangents", s.m_pTangents);
		r.Ptr(pKey, "coords", s.m_pCoords);
		r.Ptr(pKey, "tex0", s.m_pTexCoords0);
		r.Ptr(pKey, "tex1", s.m_pTexCoords1);
		r.Ptr(pKey, "windw", s.m_pWindWeights);
		r.Ptr(pKey, "windi", s.m_pWindMatrixIndices);
		if (!bArrays || !n)
			return;
		if (s.m_pColors)			r.Colors(pKey, "colors", s.m_pColors, n);
		if (s.m_pNormals)			r.Floats(pKey, "normals", s.m_pNormals, n * 3);
		if (s.m_pBinormals)			r.Floats(pKey, "binormals", s.m_pBinormals, n * 3);
		if (s.m_pTangents)			r.Floats(pKey, "tangents", s.m_pTangents, n * 3);
		if (s.m_pCoords)			r.Floats(pKey, "coords", s.m_pCoords, n * 3);
		if (s.m_pTexCoords0)		r.Floats(pKey, "tex0", s.m_pTexCoords0, n * 2);
		if (s.m_pTexCoords1)		r.Floats(pKey, "tex1", s.m_pTexCoords1, n * 2);
		if (s.m_pWindWeights)		r.Floats(pKey, "windw", s.m_pWindWeights, n);
		if (s.m_pWindMatrixIndices)	r.U8s(pKey, "windi", s.m_pWindMatrixIndices, n);
	}

	template <class TTree>
	void RecordLeaf(CSptResult& r, const char* pKey, const typename TTree::SGeometry::SLeaf& s, bool bArrays, bool bCorners)
	{
		r.Int(pKey, "active", s.m_bIsActive ? 1 : 0);
		if (!s.m_bIsActive)
			return;
		r.Float(pKey, "alpha", s.m_fAlphaTestValue);
		r.Int(pKey, "lod", s.m_nDiscreteLodLevel);
		r.Int(pKey, "count", s.m_usLeafCount);
		const unsigned int n = s.m_usLeafCount;
		if (!n)
			return;
		r.Ptr(pKey, "colors", s.m_pColors);
		r.Ptr(pKey, "normals", s.m_pNormals);
		r.Ptr(pKey, "binormals", s.m_pBinormals);
		r.Ptr(pKey, "tangents", s.m_pTangents);
		r.Ptr(pKey, "windw", s.m_pWindWeights);
		r.Ptr(pKey, "windi", s.m_pWindMatrixIndices);
		if (bArrays)
		{
			r.U8s(pKey, "map", s.m_pLeafMapIndices, n);
			r.U8s(pKey, "cluster", s.m_pLeafClusterIndices, n);
			r.Floats(pKey, "centers", s.m_pCenterCoords, n * 3);
			if (s.m_pColors)			r.Colors(pKey, "colors", s.m_pColors, n);
			if (s.m_pNormals)			r.Floats(pKey, "normals", s.m_pNormals, n * 3);
			if (s.m_pBinormals)			r.Floats(pKey, "binormals", s.m_pBinormals, n * 3);
			if (s.m_pTangents)			r.Floats(pKey, "tangents", s.m_pTangents, n * 3);
			if (s.m_pWindWeights)		r.Floats(pKey, "windw", s.m_pWindWeights, n);
			if (s.m_pWindMatrixIndices)	r.U8s(pKey, "windi", s.m_pWindMatrixIndices, n);
			for (unsigned int i = 0; i < n; ++i)
				r.Floats(pKey, "tex", s.m_pLeafMapTexCoords[i], 8);
		}
		if (bCorners)
			for (unsigned int i = 0; i < n; ++i)
				r.Floats(pKey, "corners", s.m_pLeafMapCoords[i], 16);
	}

	template <class TTree>
	void RecordBillboard(CSptResult& r, const char* pKey, const typename TTree::SGeometry::SBillboard& b)
	{
		r.Int(pKey, "active", b.m_bIsActive ? 1 : 0);
		if (!b.m_bIsActive)
			return;
		r.Float(pKey, "alpha", b.m_fAlphaTestValue);
		if (b.m_pCoords)
			r.Floats(pKey, "coords", b.m_pCoords, 12);
		if (b.m_pTexCoords)
			r.Floats(pKey, "tex", b.m_pTexCoords, 8);
	}

	// Cynex: her parca ayri ve taze SGeometry ile (dll tum parcalari birlikte isterken cokebiliyor)
	template <class TTree>
	void RecordParts(CSptResult& r, TTree* pTree, const char* pTag, bool bArrays, bool bCorners, short ob = -1, short of = -1, short ol = -1, bool bBillboard = true)
	{
		char key[256];
		{
			typename TTree::SGeometry g;
			pTree->GetGeometry(g, SpeedTree_BranchGeometry, ob, of, ol);
			snprintf(key, sizeof(key), "%s/branch", pTag);
			r.Float(key, "alpha", g.m_fBranchAlphaTestValue);
			RecordIndexed<TTree>(r, key, g.m_sBranches, bArrays);
		}
		if (pTree->GetNumFrondLodLevels() && (of < 0 || of < (short) pTree->GetNumFrondLodLevels()))
		{
			typename TTree::SGeometry g0;
			pTree->GetGeometry(g0, SpeedTree_FrondGeometry, -1, 0, -1);
			if (g0.m_sFronds.m_usVertexCount > 0 || of < 0)
			{
				typename TTree::SGeometry g;
				pTree->GetGeometry(g, SpeedTree_FrondGeometry, ob, of, ol);
				snprintf(key, sizeof(key), "%s/frond", pTag);
				// Cynex: dll frond billboard bandinda (indeks tasmasi) bazen istisna atiyor (tanimsiz davranis); karsilastirma disi
				if (of >= 0 || pTree->GetDiscreteFrondLodLevel() >= 0)
				{
					r.Float(key, "alpha", g.m_fFrondAlphaTestValue);
					RecordIndexed<TTree>(r, key, g.m_sFronds, bArrays);
				}
			}
		}
		{
			typename TTree::SGeometry g;
			pTree->GetGeometry(g, SpeedTree_LeafGeometry, ob, of, ol);
			snprintf(key, sizeof(key), "%s/leaf0", pTag);
			RecordLeaf<TTree>(r, key, g.m_sLeaves0, bArrays, bCorners);
			snprintf(key, sizeof(key), "%s/leaf1", pTag);
			RecordLeaf<TTree>(r, key, g.m_sLeaves1, bArrays, bCorners);
		}
		if (bBillboard)
		{
			typename TTree::SGeometry g;
			pTree->GetGeometry(g, SpeedTree_BillboardGeometry, ob, of, ol);
			snprintf(key, sizeof(key), "%s/bb0", pTag);
			RecordBillboard<TTree>(r, key, g.m_sBillboard0);
			snprintf(key, sizeof(key), "%s/bb1", pTag);
			RecordBillboard<TTree>(r, key, g.m_sBillboard1);
			snprintf(key, sizeof(key), "%s/bbh", pTag);
			RecordBillboard<TTree>(r, key, g.m_sHorizontalBillboard);
		}
	}

	template <class TTree>
	TTree* LoadGameTree(const unsigned char* pBlock, unsigned int uiSize, typename TTree::ELightingMethod eLight, typename TTree::EWindMethod eWind)
	{
		TTree* pTree = new TTree;
		pTree->SetWindStrength(1.0f);
		pTree->SetLocalMatrices(0, 4);
		TTree::SetTextureFlip(true);
		if (!pTree->LoadTree(pBlock, uiSize))
			return NULL;
		pTree->SetBranchLightingMethod(eLight);
		pTree->SetLeafLightingMethod(eLight);
		pTree->SetFrondLightingMethod(eLight);
		pTree->SetBranchWindMethod(eWind);
		pTree->SetLeafWindMethod(eWind);
		pTree->SetFrondWindMethod(eWind);
		pTree->SetNumLeafRockingGroups(1);
		if (!pTree->Compute(NULL, 1))
			return NULL;
		pTree->SetLeafRockingState(true);
		return pTree;
	}

	template <class TTree>
	void SetCam(float px, float py, float pz, float dx, float dy, float dz)
	{
		const float p[3] = { px, py, pz };
		const float d[3] = { dx, dy, dz };
		TTree::SetCamera(p, d);
	}

	///////////////////////////////////////////////////////////////////////
	//	ana senaryo

	template <class TTree>
	bool Run(CSptResult& r, const unsigned char* pBlock, unsigned int uiSize)
	{
		char key[256];
		TTree::SetNumWindMatrices(4);
		TTree::SetLightAttributes(0, c_afGameLight);
		TTree::SetLightState(0, true);
		TTree::SetDropToBillboard(false);
		TTree::SetTime(0.0f);
		SetCam<TTree>(0, 0, 0, 1, 0, 0);

		// Cynex: 1) oyunun LoadTree akisi
		TTree* pMain = LoadGameTree<TTree>(pBlock, uiSize, TTree::LIGHT_STATIC, TTree::WIND_NONE);
		if (!pMain)
		{
			r.Int("load", "ok", 0);
			return false;
		}
		r.Int("load", "ok", 1);
		float afBox[6];
		pMain->GetBoundingBox(afBox);
		r.Floats("info", "bbox", afBox, 6);
		TTree::SetDropToBillboard(true);
		r.Floats("info", "bmat", pMain->GetBranchMaterial(), 13);
		r.Floats("info", "fmat", pMain->GetFrondMaterial(), 13);
		r.Floats("info", "lmat", pMain->GetLeafMaterial(), 13);
		r.Float("info", "leafadj", pMain->GetLeafLightingAdjustment());
		r.Int("info", "style", pMain->GetStaticLightingStyle());
		r.Int("info", "lighting", pMain->GetBranchLightingMethod() * 100 + pMain->GetLeafLightingMethod() * 10 + pMain->GetFrondLightingMethod());
		r.Int("info", "wind", pMain->GetBranchWindMethod() * 100 + pMain->GetLeafWindMethod() * 10 + pMain->GetFrondWindMethod());
		r.Int("info", "rock", pMain->GetLeafRockingState() ? 1 : 0);
		r.Float("info", "windstrength", pMain->GetWindStrength());
		float fNear, fFar;
		pMain->GetLodLimits(fNear, fFar);
		r.Float("info", "lodnear", fNear);
		r.Float("info", "lodfar", fFar);
		const float fHeight = afBox[5] - afBox[2];
		pMain->SetLodLimits(fHeight * 2.0f, fHeight * 9.0f);
		float fSize, fVar;
		pMain->GetTreeSize(fSize, fVar);
		r.Float("info", "size", fSize);
		r.Float("info", "sizevar", fVar);
		r.Int("info", "seed", (long long) pMain->GetSeed());
		r.Int("info", "lods", pMain->GetNumBranchLodLevels() * 10000 + pMain->GetNumFrondLodLevels() * 100 + pMain->GetNumLeafLodLevels());
		unsigned int uiStart, uiSpan;
		pMain->GetLocalMatrices(uiStart, uiSpan);
		r.Int("info", "localmatrices", uiStart * 100 + uiSpan);

		typename TTree::STextures tex;
		pMain->GetTextures(tex);
		r.String("tex", "branch", tex.m_pBranchTextureFilename);
		r.String("tex", "composite", tex.m_pCompositeFilename);
		r.String("tex", "shadow", tex.m_pSelfShadowFilename);
		r.Int("tex", "leafcount", tex.m_uiLeafTextureCount);
		for (unsigned int i = 0; i < tex.m_uiLeafTextureCount; ++i)
			r.String("tex", "leaf", tex.m_pLeafTextureFilenames[i]);
		r.Int("tex", "frondcount", tex.m_uiFrondTextureCount);
		for (unsigned int i = 0; i < tex.m_uiFrondTextureCount; ++i)
			r.String("tex", "frond", tex.m_pFrondTextureFilenames[i]);
		r.String("info", "userdata", pMain->GetUserData());

		r.Int("coll", "count", pMain->GetCollisionObjectCount());
		for (unsigned int i = 0; i < pMain->GetCollisionObjectCount(); ++i)
		{
			typename TTree::ECollisionObjectType eType = TTree::CO_SPHERE;
			float afPos[3] = { 0, 0, 0 }, afDim[3] = { 0, 0, 0 };
			pMain->GetCollisionObject(i, eType, afPos, afDim);
			r.Int("coll", "type", eType);
			r.Floats("coll", "pos", afPos, 3);
			r.Floats("coll", "dim", afDim, 3);
		}
		const float* pSizes = pMain->GetLeafLodSizeAdjustments();
		if (pSizes)
			r.Floats("info", "leafsizes", pSizes, pMain->GetNumLeafLodLevels());

		// Cynex: 2) SetupBuffers: LOD 1, tum seviyeler override ile
		pMain->SetLodLevel(1.0f);
		SetCam<TTree>(0, 0, 0, -1, 0, 0);
		RecordParts<TTree>(r, pMain, "setup", true, true);
		for (unsigned short l = 0; l < pMain->GetNumBranchLodLevels(); ++l)
		{
			snprintf(key, sizeof(key), "setup/blod%u", l);
			typename TTree::SGeometry g;
			pMain->GetGeometry(g, SpeedTree_BranchGeometry, (short) l);
			RecordIndexed<TTree>(r, key, g.m_sBranches, true);
		}
		{
			typename TTree::SGeometry g0;
			pMain->GetGeometry(g0, SpeedTree_FrondGeometry);
			if (g0.m_sFronds.m_usVertexCount > 1)
				for (unsigned short l = 0; l < pMain->GetNumFrondLodLevels(); ++l)
				{
					snprintf(key, sizeof(key), "setup/flod%u", l);
					typename TTree::SGeometry g;
					pMain->GetGeometry(g, SpeedTree_FrondGeometry, -1, (short) l);
					RecordIndexed<TTree>(r, key, g.m_sFronds, true);
					const unsigned char* pMap = pMain->GetFrondGeometryMapIndexes(l);
					if (pMap && g.m_sFronds.m_usNumStrips)
						r.U8s(key, "mapidx", pMap, g.m_sFronds.m_usNumStrips);
				}
		}
		for (unsigned short l = 0; l < pMain->GetNumLeafLodLevels(); ++l)
		{
			snprintf(key, sizeof(key), "setup/llod%u", l);
			typename TTree::SGeometry g;
			pMain->GetGeometry(g, SpeedTree_LeafGeometry, -1, -1, (short) l);
			RecordLeaf<TTree>(r, key, g.m_sLeaves0, true, true);
		}
		unsigned int uiEntries = 0;
		const float* pTable = pMain->GetLeafBillboardTable(uiEntries);
		r.Int("setup", "lbtcount", uiEntries);
		if (pTable)
			r.Floats("setup", "lbt", pTable, uiEntries);

		// Cynex: 3) oyun dongusu: ornekler, kamera yorungesi, zaman; LOD hep 1
		TTree* apInst[3];
		const float afInstPos[3][3] = { { 0, 0, 0 }, { 1500, -300, 20 }, { -4000, 2500, -50 } };
		for (int i = 0; i < 3; ++i)
		{
			apInst[i] = pMain->MakeInstance();
			apInst[i]->SetTreePosition(afInstPos[i][0], afInstPos[i][1], afInstPos[i][2]);
		}
		r.Int("inst", "instof", apInst[1]->InstanceOf() == pMain ? 1 : 0);
		r.Floats("inst", "pos", apInst[1]->GetTreePosition(), 3);
		r.Float("inst", "windstrength", apInst[0]->GetWindStrength());
		const float afEnvWind[2] = { 0.2f, 0.65f };
		for (int frame = 0; frame < 48; ++frame)
		{
			const float t = frame < 24 ? frame * 0.033f : 0.8f + (frame - 24) * 1.37f;
			if (frame == 10)
				for (int i = 0; i < 3; ++i)
					apInst[i]->SetWindStrength(afEnvWind[0]);
			if (frame == 30)
				for (int i = 0; i < 3; ++i)
					apInst[i]->SetWindStrength(afEnvWind[1]);
			TTree::SetTime(t);
			const float az = 0.37f + frame * 0.29f;
			const float pitch = -0.05f - 0.6f * (float) fabs(sin(frame * 0.4));
			const float dist = 600.0f + 220.0f * frame;
			const float eye[3] = { -cosf(az) * cosf(pitch) * dist, -sinf(az) * cosf(pitch) * dist, -sinf(pitch) * dist + 180.0f };
			// Cynex: oyunda yon view*proj 3. sutunu: ~1.0002 olcekli, normalize degil
			const float q = 1.00021f;
			SetCam<TTree>(eye[0], eye[1], eye[2], cosf(az) * cosf(pitch) * q, sinf(az) * cosf(pitch) * q, sinf(pitch) * q);
			for (int i = 0; i < 3; ++i)
			{
				apInst[i]->ComputeLodLevel();
				snprintf(key, sizeof(key), "frame%02d/inst%d", frame, i);
				r.Float(key, "computedlod", apInst[i]->GetLodLevel());
				apInst[i]->SetLodLevel(1.0f);
				const bool bFull = (frame % 3) == 0 || i == 0;
				RecordParts<TTree>(r, apInst[i], key, false, bFull);
			}
			pTable = pMain->GetLeafBillboardTable(uiEntries);
			snprintf(key, sizeof(key), "frame%02d", frame);
			if (pTable)
				r.Floats(key, "lbt", pTable, uiEntries);
		}

		// Cynex: 4) LOD taramasi (drop acik/kapali), ornek 1 uzerinde
		for (int iDrop = 0; iDrop < 2; ++iDrop)
		{
			TTree::SetDropToBillboard(iDrop != 0);
			SetCam<TTree>(100, 200, 300, -0.6f, 0.8f, -0.1f);
			for (int s = 0; s <= 200; ++s)
			{
				const float f = s * 0.005f;
				apInst[1]->SetLodLevel(f);
				snprintf(key, sizeof(key), "lod%d/%03d", iDrop, s);
				RecordParts<TTree>(r, apInst[1], key, false, (s % 10) == 0);
				r.Int(key, "discb", apInst[1]->GetDiscreteBranchLodLevel(f));
				r.Int(key, "discf", apInst[1]->GetDiscreteFrondLodLevel(f));
				r.Int(key, "discl", apInst[1]->GetDiscreteLeafLodLevel(f));
				r.Int(key, "trib", apInst[1]->GetBranchTriangleCount(f));
				r.Int(key, "trif", apInst[1]->GetFrondTriangleCount(f));
			}
			// Cynex: override ile billboard bolgesi
			for (int s = 0; s <= 20; ++s)
			{
				const float f = s * 0.05f;
				apInst[1]->SetLodLevel(f);
				snprintf(key, sizeof(key), "lodov%d/%02d", iDrop, s);
				RecordParts<TTree>(r, apInst[1], key, false, false, 0, 0, 0);
			}
		}
		TTree::SetDropToBillboard(true);

		// Cynex: hedef alfa maskesi
		pMain->SetLeafTargetAlphaMask(0x70);
		for (int s = 0; s <= 20; ++s)
		{
			apInst[1]->SetLodLevel(s * 0.05f);
			snprintf(key, sizeof(key), "alphamask/%02d", s);
			RecordParts<TTree>(r, apInst[1], key, false, false);
		}
		pMain->SetLeafTargetAlphaMask();

		// Cynex: 5) 360 billboard / kamera acilari (billboard bolgesinde)
		apInst[2]->SetLodLevel(0.0f);
		for (int s = 0; s < 72; ++s)
		{
			const float a = s * 0.0872665f + 0.01f;
			const float p = (s % 4) * -0.25f;
			SetCam<TTree>(0, 0, 0, cosf(a) * cosf(p), sinf(a) * cosf(p), sinf(p));
			snprintf(key, sizeof(key), "bbang/%02d", s);
			typename TTree::SGeometry g;
			apInst[2]->GetGeometry(g, SpeedTree_BillboardGeometry);
			RecordBillboard<TTree>(r, (std::string(key) + "/bb0").c_str(), g.m_sBillboard0);
			RecordBillboard<TTree>(r, (std::string(key) + "/bb1").c_str(), g.m_sBillboard1);
			RecordBillboard<TTree>(r, (std::string(key) + "/bbh").c_str(), g.m_sHorizontalBillboard);
		}

		// Cynex: 6) ruzgar gucu / sallanma durum makinesi
		apInst[0]->SetLodLevel(1.0f);
		SetCam<TTree>(0, 0, 0, -1, 0, 0);
		TTree::SetTime(10.0f);
		r.Float("wind", "ret1", apInst[0]->SetWindStrength(0.5f));
		RecordParts<TTree>(r, apInst[0], "wind/a", false, true, -1, -1, -1, false);
		r.Float("wind", "ret2", apInst[0]->SetWindStrength(0.3f, 0.5f, 0.0f));
		r.Float("wind", "ret3", apInst[0]->SetWindStrength(0.3f, 0.5f, 2.0f));
		r.Float("wind", "ret4", apInst[0]->SetWindStrength(0.3f, 0.3f, 2.0f));
		r.Float("wind", "ret5", apInst[0]->SetWindStrength(0.3f));
		r.Float("wind", "ret6", apInst[0]->SetWindStrength(0.3f, -1.0f, 5.0f));
		TTree::SetTime(20.0f);
		r.Float("wind", "ret7", apInst[0]->SetWindStrength(0.6f, 0.3f, -1.0f));
		r.Float("wind", "get", pMain->GetWindStrength());
		RecordParts<TTree>(r, apInst[0], "wind/b", false, true, -1, -1, -1, false);
		TTree::SetTime(23.0f);
		apInst[0]->SetLeafRockingState(false);
		RecordParts<TTree>(r, apInst[0], "wind/c", false, true, -1, -1, -1, false);
		apInst[0]->SetLeafRockingState(true);
		RecordParts<TTree>(r, apInst[0], "wind/d", false, true, -1, -1, -1, false);
		TTree::SetTime(25.0f);
		RecordParts<TTree>(r, apInst[0], "wind/e", false, true, -1, -1, -1, false);
		apInst[0]->SetLeafRockingState(false);
		TTree::SetTime(27.0f);
		RecordParts<TTree>(r, apInst[0], "wind/f", false, true, -1, -1, -1, false);
		SetCam<TTree>(0, 0, 0, 0, -1, 0);
		RecordParts<TTree>(r, apInst[0], "wind/g", false, true, -1, -1, -1, false);
		r.Int("wind", "rockmain", pMain->GetLeafRockingState() ? 1 : 0);
		apInst[0]->SetLeafRockingState(true);

		// Cynex: 7) dokular
		{
			const float afTc[8] = { 0.11f, 0.23f, 0.35f, 0.41f, 0.52f, 0.68f, 0.07f, 0.89f };
			pMain->SetLeafTextureCoords(0, afTc);
			pMain->SetFrondTextureCoords(0, afTc);
			pMain->SetLodLevel(1.0f);
			typename TTree::SGeometry g;
			pMain->GetGeometry(g, SpeedTree_LeafGeometry);
			RecordLeaf<TTree>(r, "texset/leaf", g.m_sLeaves0, true, false);
			typename TTree::SGeometry gf;
			pMain->GetGeometry(gf, SpeedTree_FrondGeometry);
			if (gf.m_sFronds.m_usVertexCount && gf.m_sFronds.m_pTexCoords0)
				r.Floats("texset/frond", "tex0", gf.m_sFronds.m_pTexCoords0, gf.m_sFronds.m_usVertexCount * 2);
		}

		// Cynex: 8) dinamik isik + GPU ruzgar varyanti
		{
			TTree* pDyn = LoadGameTree<TTree>(pBlock, uiSize, TTree::LIGHT_DYNAMIC, TTree::WIND_GPU);
			if (pDyn)
			{
				pDyn->SetLodLevel(1.0f);
				RecordParts<TTree>(r, pDyn, "dyn", true, false, -1, -1, -1, false);
			}
		}

		// Cynex: 9) CPU ruzgar
		{
			TTree* pCpu = LoadGameTree<TTree>(pBlock, uiSize, TTree::LIGHT_STATIC, TTree::WIND_CPU);
			if (pCpu)
			{
				// Cynex: dll ruzgar matris indekslerini rand() ile dagitiyor; formul testi icin dort matris ayni
				for (int i = 0; i < 4; ++i)
				{
					float m[16];
					const float ax = 0.1f, ay = -0.07f;
					const float cx = cosf(ax), sx = sinf(ax), cy = cosf(ay), sy = sinf(ay);
					memset(m, 0, sizeof(m));
					m[0] = cy; m[2] = -sy; m[4] = sx * sy; m[5] = cx; m[6] = sx * cy; m[8] = sy * cx; m[9] = -sx; m[10] = cx * cy; m[15] = 1.0f;
					TTree::SetWindMatrix(i, m);
				}
				pCpu->SetLodLevel(1.0f);
				pCpu->ComputeWindEffects(true, true, true);
				RecordParts<TTree>(r, pCpu, "cpu", true, false, -1, -1, -1, false);
			}
		}
		return true;
	}
}
