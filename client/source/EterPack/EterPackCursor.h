#ifndef __INC_ETERPACKCURSOR_H__
#define __INC_ETERPACKCURSOR_H__

#include "EterPack.h"

class CEterPackCursor
{
	public:
		CEterPackCursor(CEterPack * pack);
		~CEterPackCursor();

		bool		Open(const char* filename);
		void		Close();
		void		Seek(LONG offset);
		bool		Read(LPVOID data, LONG size);
		LONG		Size();

	private:
		CEterPack * m_pPack;
		CMappedFile m_file;
		LPCVOID 	m_pData;
		LONG		m_ReadPoint;
};

#endif
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
