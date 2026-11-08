#pragma once

#include "GrpVertexBuffer.h"

class CDynamicVertexBuffer : public CGraphicVertexBuffer
{
	public:
		CDynamicVertexBuffer();
		virtual ~CDynamicVertexBuffer();

		bool Create(int vtxCount, int fvf);



	protected:
		int m_vtxCount;
		int m_fvf;
};
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
