#pragma once

class CDirect3DXBuffer
{
	public:
		CDirect3DXBuffer();
		CDirect3DXBuffer(LPD3DXBUFFER lpd3dxBuffer);
		virtual ~CDirect3DXBuffer();

		void Destroy();
		void Create(LPD3DXBUFFER lpd3dxBuffer);

		void*GetPointer();
		int  GetSize();

	protected:
		LPD3DXBUFFER m_lpd3dxBuffer;
};
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
