// Cynex: oyunun SpeedTreeLib'te kullandigi cagrilarin tamami; .so baglanabiliyor mu
#include <SpeedTreeRT.h>

extern "C" __attribute__((visibility("default"))) int CynexSptLinkTest(const unsigned char* pBlock, unsigned int uiSize)
{
	const float afLight[16] = { -0.707f, -0.3f, 0.707f, 1, 1, 1, 0.5f, 0.5f, 0.5f, 1, 1, 1, 0, 1, 0, 0 };
	CSpeedTreeRT::SetNumWindMatrices(4);
	CSpeedTreeRT::SetLightAttributes(0, afLight);
	CSpeedTreeRT::SetLightState(0, true);
	CSpeedTreeRT* pTree = new CSpeedTreeRT;
	pTree->SetWindStrength(1.0f);
	pTree->SetLocalMatrices(0, 4);
	pTree->SetTextureFlip(true);
	if (!pTree->LoadTree(pBlock, uiSize) && !pTree->LoadTree("tree.spt"))
	{
		delete pTree;
		return -1;
	}
	pTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
	pTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
	pTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
	pTree->SetBranchWindMethod(CSpeedTreeRT::WIND_NONE);
	pTree->SetLeafWindMethod(CSpeedTreeRT::WIND_NONE);
	pTree->SetFrondWindMethod(CSpeedTreeRT::WIND_NONE);
	pTree->SetNumLeafRockingGroups(1);
	if (!pTree->Compute(NULL, 1))
		return -2;
	float afBox[6];
	pTree->GetBoundingBox(afBox);
	pTree->SetLeafRockingState(true);
	CSpeedTreeRT::SetDropToBillboard(true);
	const float* pMat = pTree->GetBranchMaterial();
	pMat = pTree->GetFrondMaterial();
	pMat = pTree->GetLeafMaterial();
	pTree->SetLodLimits(afBox[5] * 2.0f, afBox[5] * 9.0f);
	CSpeedTreeRT::STextures kTex;
	pTree->GetTextures(kTex);
	CSpeedTreeRT::SGeometry* pGeom = new CSpeedTreeRT::SGeometry;
	pTree->SetLodLevel(1.0f);
	pTree->GetGeometry(*pGeom);
	int iSum = pTree->GetNumBranchLodLevels() + pTree->GetNumFrondLodLevels() + pTree->GetNumLeafLodLevels();
	CSpeedTreeRT* pInst = pTree->MakeInstance();
	pInst->SetTreePosition(1, 2, 3);
	const float afPos[3] = { 0, 0, 0 }, afDir[3] = { 1, 0, 0 };
	CSpeedTreeRT::SetCamera(afPos, afDir);
	CSpeedTreeRT::SetTime(1.0f);
	pInst->ComputeLodLevel();
	pInst->SetLodLevel(1.0f);
	pInst->ComputeWindEffects(true, true, true);
	pInst->GetGeometry(*pGeom, SpeedTree_LeafGeometry);
	pInst->GetGeometry(*pGeom, SpeedTree_BillboardGeometry);
	unsigned int n = 0;
	pTree->GetLeafBillboardTable(n);
	pTree->GetLeafLodSizeAdjustments();
	for (unsigned int i = 0; i < pTree->GetCollisionObjectCount(); ++i)
	{
		CSpeedTreeRT::ECollisionObjectType eType;
		float p[3], d[3];
		pTree->GetCollisionObject(i, eType, p, d);
	}
	float fSize, fVar;
	pTree->GetTreeSize(fSize, fVar);
	iSum += (int) (pMat[0] + fSize + n) + (CSpeedTreeRT::GetCurrentError()[0] != 0);
	CSpeedTreeRT::SetWindMatrix(0, afLight);
	pTree->DeleteTransientData();
	pInst->SetWindStrength(0.5f);
	iSum += (int) pInst->GetLeafLightingAdjustment();
	delete pGeom;
	delete pInst;
	delete pTree;
	return iSum;
}
