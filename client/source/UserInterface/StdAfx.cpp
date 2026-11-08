// stdafx.cpp : source file that includes just the standard includes
//	UserInterface.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#ifdef __AUTO_HUNT__
float GetDistanceNew(const TPixelPosition& PixelPosition, const TPixelPosition& c_rPixelPosition)
{
	const float fdx = PixelPosition.x - c_rPixelPosition.x;
	const float fdy = PixelPosition.y - c_rPixelPosition.y;
	return sqrtf((fdx * fdx) + (fdy * fdy));
}

#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp>
void split_argument(const char* argument, std::vector<std::string>& vecArgs, const char* arg)
{
	boost::split(vecArgs, argument, boost::is_any_of(arg), boost::token_compress_on);
}
#endif

#ifdef ENABLE_DUNGEON_BOSS_ICON_IN_MAP
#include "PythonNonPlayer.h"

bool IsBoss(DWORD race)
{
	const CPythonNonPlayer::TMobTable* pMobTable = CPythonNonPlayer::Instance().GetTable(race);
	if (pMobTable && pMobTable->bType  == 0 && pMobTable->bRank >= 4)
		return true;
	return false;
}

bool GetDungeonNPC(DWORD mobIndex)
{
	if (mobIndex >= 9860 && mobIndex <= 9871)// Zodiac
	{
		return true;
	}
	switch (mobIndex)
	{
		case 1093:
		case 2598:
		case 2493:
		case 6091:
		case 6191:
		case 2092:
		case 2093:
			return true;
	}
	return false;
}
#endif

//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
