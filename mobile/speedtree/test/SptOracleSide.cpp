// Cynex: gercek SpeedTreeRT.dll ile senaryo (sadece Win32)
#include <windows.h>
#include <SpeedTreeRT.h>

#include "SptScenario.h"

static bool RunGuarded(CSptResult& r, const unsigned char* pBlock, unsigned int uiSize, bool& bCrashed)
{
	__try
	{
		return SptScenario::Run<CSpeedTreeRT>(r, pBlock, uiSize);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		bCrashed = true;
		return false;
	}
}

bool RunOracle(CSptResult& r, const unsigned char* pBlock, unsigned int uiSize, bool& bCrashed)
{
	bCrashed = false;
	return RunGuarded(r, pBlock, uiSize, bCrashed);
}
