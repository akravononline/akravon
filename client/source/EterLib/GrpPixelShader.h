#pragma once

#include "GrpBase.h"

class CPixelShader : public CGraphicBase
{
	public:
		CPixelShader();
		virtual ~CPixelShader();

		void Destroy();
		bool CreateFromDiskFile(const char* c_szFileName);

		void Set();

	protected:
		void Initialize();

	protected:
		DWORD m_handle;
};
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
