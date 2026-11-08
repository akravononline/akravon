#pragma once

#ifdef ENABLE_CHANGELOOK_SYSTEM
#include "Packet.h"

class CPythonChangeLook : public CSingleton<CPythonChangeLook>
{
	public:
		#ifdef ENABLE_FULL_YANG
		int64_t	dwCost;
		#else
		DWORD	dwCost;
		#endif
		typedef std::vector<TChangeLookMaterial> TChangeLookMaterials;

	public:
		CPythonChangeLook();
		virtual ~CPythonChangeLook();

		void	Clear();
		void	AddMaterial(BYTE bPos, TItemPos tPos);
		void	RemoveMaterial(BYTE bPos);
		void	RemoveAllMaterials();

		#ifdef ENABLE_FULL_YANG
		int64_t	GetCost() {return dwCost;}
		void	SetCost(int64_t dwCostR) {dwCost = dwCostR;}
		#else
		DWORD	GetCost() {return dwCost;}
		void	SetCost(DWORD dwCostR) {dwCost = dwCostR;}
		#endif

		bool	GetAttachedItem(BYTE bPos, BYTE & bHere, WORD & wCell);

	protected:
		TChangeLookMaterials	m_vMaterials;
};
#endif
