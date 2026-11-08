#pragma once

#include "GrpBase.h"

class CVertexShader : public CGraphicBase
{
	public:
		CVertexShader();
		virtual ~CVertexShader();

		void Destroy();
		bool CreateFromDiskFile(const char* c_szFileName, const DWORD* c_pdwVertexDecl);

		void Set();

	protected:
		void Initialize();

	protected:
		DWORD m_handle;
};
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
