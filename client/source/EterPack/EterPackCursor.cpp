#include "StdAfx.h"
#include "EterPackCursor.h"
#include "Inline.h"

CEterPackCursor::CEterPackCursor(CEterPack* pack) : m_pPack(pack), m_pData(NULL), m_ReadPoint(0)
{
}

CEterPackCursor::~CEterPackCursor()
{
	Close();
}

bool CEterPackCursor::Open(const char* filename)
{
	assert(m_pPack != NULL);

	char tmpFilename[MAX_PATH + 1];
	strncpy(tmpFilename, filename, MAX_PATH);
	inlineConvertPackFilename(tmpFilename);

	if (!m_pPack->Get(m_file, tmpFilename, &m_pData))
		return false;

	return true;
}

void CEterPackCursor::Close()
{
	m_file.Destroy();
	m_pData = NULL;
	m_ReadPoint = 0;
}

void CEterPackCursor::Seek(LONG offset)
{
	m_ReadPoint = max(0, min(Size(), offset));
}

bool CEterPackCursor::Read(LPVOID data, LONG size)
{
	if (m_file.IsNull())
		return false;

	if (m_ReadPoint + size > Size())
		return false;

	memcpy(data, (char*) m_pData + m_ReadPoint, size);
	m_ReadPoint += size;
	return true;
}

LONG CEterPackCursor::Size()
{
	if (m_file.IsNull())
		return 0;

	return m_file.Size();
}
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
