// Cynex: cynex_speedtree ile ayni senaryo (test derlemesinde sinif adi CCynexSpeedTreeRT)
#include <SpeedTreeRT.h>

#include <vector>

#include "SptScenario.h"

bool RunCynex(CSptResult& r, const unsigned char* pBlock, unsigned int uiSize)
{
	return SptScenario::Run<CSpeedTreeRT>(r, pBlock, uiSize);
}

// Cynex: bozuk dosya dayanikliligi: kesilmis / bit bozulmus girdide cokme olmamali
int FuzzCynex(const unsigned char* pBlock, unsigned int uiSize, unsigned int uiIterations)
{
	std::vector<unsigned char> vecData;
	unsigned int uiSeed = 12345, uiLoaded = 0;
	for (unsigned int it = 0; it < uiIterations; ++it)
	{
		vecData.assign(pBlock, pBlock + uiSize);
		uiSeed = uiSeed * 1103515245u + 12345u;
		if (it & 1)
			vecData.resize((uiSeed >> 8) % uiSize);
		else
			for (int k = 0; k < 8; ++k)
			{
				uiSeed = uiSeed * 1103515245u + 12345u;
				vecData[(uiSeed >> 4) % uiSize] ^= (unsigned char) (1u << ((uiSeed >> 24) & 7));
			}
		CSpeedTreeRT* pTree = new CSpeedTreeRT;
		if (!vecData.empty() && pTree->LoadTree(&vecData[0], (unsigned int) vecData.size()) && pTree->Compute())
		{
			++uiLoaded;
			CSpeedTreeRT::SGeometry g;
			for (int l = 0; l <= 10; ++l)
			{
				pTree->SetLodLevel(l * 0.1f);
				pTree->GetGeometry(g);
			}
			unsigned int n = 0;
			pTree->GetLeafBillboardTable(n);
		}
		delete pTree;
	}
	return (int) uiLoaded;
}
