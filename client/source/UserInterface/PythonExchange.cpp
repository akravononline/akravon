#include "stdafx.h"
#include "PythonExchange.h"

void CPythonExchange::SetSelfName(const char *name)
{
	strncpy(m_self.name, name, CHARACTER_NAME_MAX_LEN);
}

void CPythonExchange::SetTargetName(const char *name)
{
	strncpy(m_victim.name, name, CHARACTER_NAME_MAX_LEN);
}

char * CPythonExchange::GetNameFromSelf()
{
	return m_self.name;
}

char * CPythonExchange::GetNameFromTarget()
{
	return m_victim.name;
}

#ifdef ENABLE_LEVEL_IN_TRADE
void CPythonExchange::SetSelfLevel(DWORD level)
{
	m_self.level = level;
}

void CPythonExchange::SetTargetLevel(DWORD level)
{
	m_victim.level = level;
}

DWORD CPythonExchange::GetLevelFromSelf()
{
	return m_self.level;
}

DWORD CPythonExchange::GetLevelFromTarget()
{
	return m_victim.level;
}
#endif

#ifdef ENABLE_FULL_YANG
void CPythonExchange::SetElkToTarget(uint64_t elk)
{
	m_victim.elk = elk;
}

void CPythonExchange::SetElkToSelf(uint64_t elk)
{
	m_self.elk = elk;
}

uint64_t CPythonExchange::GetElkFromTarget()
{
	return m_victim.elk;
}

uint64_t CPythonExchange::GetElkFromSelf()
{
	return m_self.elk;
}
#else
void CPythonExchange::SetElkToTarget(DWORD	elk)
{
	m_victim.elk = elk;
}

void CPythonExchange::SetElkToSelf(DWORD elk)
{
	m_self.elk = elk;
}

DWORD CPythonExchange::GetElkFromTarget()
{
	return m_victim.elk;
}

DWORD CPythonExchange::GetElkFromSelf()
{
	return m_self.elk;
}
#endif

#ifdef ENABLE_NEW_MAX_COUNT
void CPythonExchange::SetItemToTarget(DWORD pos, DWORD vnum, short count)
#else
void CPythonExchange::SetItemToTarget(DWORD pos, DWORD vnum, BYTE count)
#endif
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_vnum[pos] = vnum;
	m_victim.item_count[pos] = count;
}

#ifdef ENABLE_NEW_MAX_COUNT
void CPythonExchange::SetItemToSelf(DWORD pos, DWORD vnum, short count)
#else
void CPythonExchange::SetItemToSelf(DWORD pos, DWORD vnum, BYTE count)
#endif
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_vnum[pos] = vnum;
	m_self.item_count[pos] = count;
}

void CPythonExchange::SetItemMetinSocketToTarget(int pos, int imetinpos, DWORD vnum)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_metin[pos][imetinpos] = vnum;
}

void CPythonExchange::SetItemMetinSocketToSelf(int pos, int imetinpos, DWORD vnum)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_metin[pos][imetinpos] = vnum;
}

void CPythonExchange::SetItemAttributeToTarget(int pos, int iattrpos, BYTE byType, short sValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_attr[pos][iattrpos].bType = byType;
	m_victim.item_attr[pos][iattrpos].sValue = sValue;
}

void CPythonExchange::SetItemAttributeToSelf(int pos, int iattrpos, BYTE byType, short sValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_attr[pos][iattrpos].bType = byType;
	m_self.item_attr[pos][iattrpos].sValue = sValue;
}

void CPythonExchange::DelItemOfTarget(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_vnum[pos] = 0;
	m_victim.item_count[pos] = 0;
}

void CPythonExchange::DelItemOfSelf(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_vnum[pos] = 0;
	m_self.item_count[pos] = 0;
}

DWORD CPythonExchange::GetItemVnumFromTarget(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_vnum[pos];
}

DWORD CPythonExchange::GetItemVnumFromSelf(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_vnum[pos];
}

#ifdef ENABLE_NEW_MAX_COUNT
short CPythonExchange::GetItemCountFromTarget(BYTE pos)
#else
BYTE CPythonExchange::GetItemCountFromTarget(BYTE pos)
#endif
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_count[pos];
}

#ifdef ENABLE_NEW_MAX_COUNT
short CPythonExchange::GetItemCountFromSelf(BYTE pos)
#else
BYTE CPythonExchange::GetItemCountFromSelf(BYTE pos)
#endif
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_count[pos];
}

DWORD CPythonExchange::GetItemMetinSocketFromTarget(BYTE pos, int iMetinSocketPos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_metin[pos][iMetinSocketPos];
}

DWORD CPythonExchange::GetItemMetinSocketFromSelf(BYTE pos, int iMetinSocketPos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_metin[pos][iMetinSocketPos];
}

void CPythonExchange::GetItemAttributeFromTarget(BYTE pos, int iAttrPos, BYTE * pbyType, short * psValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	*pbyType = m_victim.item_attr[pos][iAttrPos].bType;
	*psValue = m_victim.item_attr[pos][iAttrPos].sValue;
}

void CPythonExchange::GetItemAttributeFromSelf(BYTE pos, int iAttrPos, BYTE * pbyType, short * psValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	*pbyType = m_self.item_attr[pos][iAttrPos].bType;
	*psValue = m_self.item_attr[pos][iAttrPos].sValue;
}


#ifdef ENABLE_EVOLUTION_SYSTEM
void CPythonExchange::SetItemEvolutionToTarget(int pos, int evolution)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.evolution[pos] = evolution;
}

void CPythonExchange::SetItemEvolutionToSelf(int pos, int evolution)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.evolution[pos] = evolution;
}

int CPythonExchange::GetItemEvolutionFromTarget(int pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.evolution[pos];
}

int CPythonExchange::GetItemEvolutionFromSelf(int pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.evolution[pos];
}
#endif

#ifdef ENABLE_CHANGELOOK_SYSTEM
void CPythonExchange::SetItemTransmutation(int iPos, DWORD dwTransmutation, bool bSelf)
{
	if (iPos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	if (bSelf)
		m_self.transmutation[iPos] = dwTransmutation;
	else
		m_victim.transmutation[iPos] = dwTransmutation;
}

DWORD CPythonExchange::GetItemTransmutation(int iPos, bool bSelf)
{
	if (iPos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	DWORD dwTransmutation = bSelf == true ? m_self.transmutation[iPos] : m_victim.transmutation[iPos];
	return dwTransmutation;
}
#endif

void CPythonExchange::SetAcceptToTarget(BYTE Accept)
{
	m_victim.accept = Accept ? true : false;
}

void CPythonExchange::SetAcceptToSelf(BYTE Accept)
{
	m_self.accept = Accept ? true : false;
}

bool CPythonExchange::GetAcceptFromTarget()
{
	return m_victim.accept ? true : false;
}

bool CPythonExchange::GetAcceptFromSelf()
{
	return m_self.accept ? true : false;
}

bool CPythonExchange::GetElkMode()
{
	return m_elk_mode;
}

void CPythonExchange::SetElkMode(bool value)
{
	m_elk_mode = value;
}

void CPythonExchange::Start()
{
	m_isTrading = true;
}

void CPythonExchange::End()
{
	m_isTrading = false;
}

bool CPythonExchange::isTrading()
{
	return m_isTrading;
}

#ifdef ENABLE_EXCHANGE_RENEWAL
void CPythonExchange::ClearLocalItemData()
{
	m_ExchangeLocalItemData.clear();
}
void CPythonExchange::AddLocalItemData(TItemPos itemPos, BYTE bDisplayPos)
{
	if (GetLocalItemData(itemPos) == true)
		return;
	DelLocalItemData(itemPos);
	m_ExchangeLocalItemData.insert(std::make_pair(itemPos, bDisplayPos));
}

void CPythonExchange::DelLocalItemData(TItemPos itemPos)
{
	if (m_ExchangeLocalItemData.end() == m_ExchangeLocalItemData.find(itemPos))
		return;

	m_ExchangeLocalItemData.erase(itemPos);
}

bool CPythonExchange::GetLocalItemData(TItemPos itemPos)
{
	if (m_ExchangeLocalItemData.end() == m_ExchangeLocalItemData.find(itemPos))
		return false;

	return true;
}
#endif

void CPythonExchange::Clear()
{
	memset(&m_self, 0, sizeof(m_self));
	memset(&m_victim, 0, sizeof(m_victim));
#ifdef ENABLE_EXCHANGE_RENEWAL
	ClearLocalItemData();
#endif
}

CPythonExchange::CPythonExchange()
{
	Clear();
	m_isTrading = false;
	m_elk_mode = false;
}
CPythonExchange::~CPythonExchange()
{
}
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
