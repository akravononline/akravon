// Cynex: d3dx8 tampon, matris yigini, mesh + kure/silindir
#include "cd3d_device.h"
#include <math.h>

using namespace cd3d;

namespace {

// ---------------------------------------------------------------- ID3DXBuffer

class XBuffer final : public ID3DXBuffer
{
public:
	explicit XBuffer(DWORD size) : m_size(size) { m_data = calloc(1, size ? size : 1); }
	~XBuffer() override { free(m_data); }
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppv) override
	{
		if (!ppv) return E_POINTER;
		if (riid == IID_IUnknown) { *ppv = this; AddRef(); return S_OK; }
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	ULONG STDMETHODCALLTYPE AddRef() override { return ++m_ref; }
	ULONG STDMETHODCALLTYPE Release() override { ULONG r = --m_ref; if (!r) delete this; return r; }
	LPVOID STDMETHODCALLTYPE GetBufferPointer() override { return m_data; }
	DWORD STDMETHODCALLTYPE GetBufferSize() override { return m_size; }
private:
	std::atomic<ULONG> m_ref{ 1 };
	void* m_data;
	DWORD m_size;
};

// ---------------------------------------------------------------- ID3DXMatrixStack

class XMatrixStack final : public ID3DXMatrixStack
{
public:
	XMatrixStack() { m_stack.resize(1); D3DXMatrixIdentity(&m_stack[0]); }
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppv) override
	{
		if (!ppv) return E_POINTER;
		if (riid == IID_IUnknown || riid == IID_ID3DXMatrixStack) { *ppv = this; AddRef(); return S_OK; }
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	ULONG STDMETHODCALLTYPE AddRef() override { return ++m_ref; }
	ULONG STDMETHODCALLTYPE Release() override { ULONG r = --m_ref; if (!r) delete this; return r; }

	HRESULT STDMETHODCALLTYPE Pop() override
	{
		if (m_stack.size() > 1)
			m_stack.pop_back();
		return D3D_OK;
	}
	HRESULT STDMETHODCALLTYPE Push() override
	{
		D3DXMATRIX t = m_stack.back();
		m_stack.push_back(t);
		return D3D_OK;
	}
	HRESULT STDMETHODCALLTYPE LoadIdentity() override { D3DXMatrixIdentity(&m_stack.back()); return D3D_OK; }
	HRESULT STDMETHODCALLTYPE LoadMatrix(CONST D3DXMATRIX* m) override { if (!m) return D3DERR_INVALIDCALL; m_stack.back() = *m; return D3D_OK; }
	// Cynex: MultMatrix = top * m, Local = m * top
	HRESULT STDMETHODCALLTYPE MultMatrix(CONST D3DXMATRIX* m) override { if (!m) return D3DERR_INVALIDCALL; D3DXMatrixMultiply(&m_stack.back(), &m_stack.back(), m); return D3D_OK; }
	HRESULT STDMETHODCALLTYPE MultMatrixLocal(CONST D3DXMATRIX* m) override { if (!m) return D3DERR_INVALIDCALL; D3DXMatrixMultiply(&m_stack.back(), m, &m_stack.back()); return D3D_OK; }
	HRESULT STDMETHODCALLTYPE RotateAxis(CONST D3DXVECTOR3* v, FLOAT a) override
	{
		if (!v) return D3DERR_INVALIDCALL;
		D3DXMATRIX t; D3DXMatrixRotationAxis(&t, v, a);
		return MultMatrix(&t);
	}
	HRESULT STDMETHODCALLTYPE RotateAxisLocal(CONST D3DXVECTOR3* v, FLOAT a) override
	{
		if (!v) return D3DERR_INVALIDCALL;
		D3DXMATRIX t; D3DXMatrixRotationAxis(&t, v, a);
		return MultMatrixLocal(&t);
	}
	HRESULT STDMETHODCALLTYPE RotateYawPitchRoll(FLOAT y, FLOAT p, FLOAT r) override
	{
		D3DXMATRIX t; D3DXMatrixRotationYawPitchRoll(&t, y, p, r);
		return MultMatrix(&t);
	}
	HRESULT STDMETHODCALLTYPE RotateYawPitchRollLocal(FLOAT y, FLOAT p, FLOAT r) override
	{
		D3DXMATRIX t; D3DXMatrixRotationYawPitchRoll(&t, y, p, r);
		return MultMatrixLocal(&t);
	}
	HRESULT STDMETHODCALLTYPE Scale(FLOAT x, FLOAT y, FLOAT z) override { D3DXMATRIX t; D3DXMatrixScaling(&t, x, y, z); return MultMatrix(&t); }
	HRESULT STDMETHODCALLTYPE ScaleLocal(FLOAT x, FLOAT y, FLOAT z) override { D3DXMATRIX t; D3DXMatrixScaling(&t, x, y, z); return MultMatrixLocal(&t); }
	HRESULT STDMETHODCALLTYPE Translate(FLOAT x, FLOAT y, FLOAT z) override { D3DXMATRIX t; D3DXMatrixTranslation(&t, x, y, z); return MultMatrix(&t); }
	HRESULT STDMETHODCALLTYPE TranslateLocal(FLOAT x, FLOAT y, FLOAT z) override { D3DXMATRIX t; D3DXMatrixTranslation(&t, x, y, z); return MultMatrixLocal(&t); }
	D3DXMATRIX* STDMETHODCALLTYPE GetTop() override { return &m_stack.back(); }

private:
	std::atomic<ULONG> m_ref{ 1 };
	std::vector<D3DXMATRIX> m_stack;
};

// ---------------------------------------------------------------- ID3DXMesh

class XMesh final : public ID3DXMesh
{
public:
	XMesh(IDirect3DDevice8* dev, DWORD faces, DWORD verts, DWORD options, DWORD fvf)
		: m_dev(dev), m_faces(faces), m_verts(verts), m_options(options), m_fvf(fvf)
	{
		m_attr.resize(faces, 0);
	}
	~XMesh() override
	{
		if (m_vb) m_vb->Release();
		if (m_ib) m_ib->Release();
	}
	bool Create()
	{
		const UINT stride = D3DXGetFVFVertexSize(m_fvf);
		const bool i32 = (m_options & D3DXMESH_32BIT) != 0;
		if (FAILED(m_dev->CreateVertexBuffer(stride * m_verts, 0, m_fvf, D3DPOOL_MANAGED, &m_vb)))
			return false;
		if (FAILED(m_dev->CreateIndexBuffer(m_faces * 3 * (i32 ? 4 : 2), 0, i32 ? D3DFMT_INDEX32 : D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_ib)))
			return false;
		return true;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppv) override
	{
		if (!ppv) return E_POINTER;
		if (riid == IID_IUnknown) { *ppv = this; AddRef(); return S_OK; }
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	ULONG STDMETHODCALLTYPE AddRef() override { return ++m_ref; }
	ULONG STDMETHODCALLTYPE Release() override { ULONG r = --m_ref; if (!r) delete this; return r; }

	HRESULT STDMETHODCALLTYPE DrawSubset(DWORD attrib) override
	{
		if (attrib != 0)
			return D3D_OK;
		m_dev->SetVertexShader(m_fvf);
		m_dev->SetStreamSource(0, m_vb, D3DXGetFVFVertexSize(m_fvf));
		m_dev->SetIndices(m_ib, 0);
		return m_dev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, m_verts, 0, m_faces);
	}
	DWORD STDMETHODCALLTYPE GetNumFaces() override { return m_faces; }
	DWORD STDMETHODCALLTYPE GetNumVertices() override { return m_verts; }
	DWORD STDMETHODCALLTYPE GetFVF() override { return m_fvf; }
	HRESULT STDMETHODCALLTYPE GetDeclaration(DWORD decl[MAX_FVF_DECL_SIZE]) override { return D3DXDeclaratorFromFVF(m_fvf, decl); }
	DWORD STDMETHODCALLTYPE GetOptions() override { return m_options; }
	HRESULT STDMETHODCALLTYPE GetDevice(LPDIRECT3DDEVICE8* pp) override
	{
		if (!pp) return D3DERR_INVALIDCALL;
		*pp = m_dev;
		m_dev->AddRef();
		return D3D_OK;
	}
	HRESULT STDMETHODCALLTYPE CloneMeshFVF(DWORD, DWORD, LPDIRECT3DDEVICE8, LPD3DXMESH* pp) override { if (pp) *pp = NULL; return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE CloneMesh(DWORD, CONST DWORD*, LPDIRECT3DDEVICE8, LPD3DXMESH* pp) override { if (pp) *pp = NULL; return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE GetVertexBuffer(LPDIRECT3DVERTEXBUFFER8* pp) override
	{
		if (!pp) return D3DERR_INVALIDCALL;
		*pp = m_vb;
		m_vb->AddRef();
		return D3D_OK;
	}
	HRESULT STDMETHODCALLTYPE GetIndexBuffer(LPDIRECT3DINDEXBUFFER8* pp) override
	{
		if (!pp) return D3DERR_INVALIDCALL;
		*pp = m_ib;
		m_ib->AddRef();
		return D3D_OK;
	}
	HRESULT STDMETHODCALLTYPE LockVertexBuffer(DWORD f, BYTE** pp) override { return m_vb->Lock(0, 0, pp, f); }
	HRESULT STDMETHODCALLTYPE UnlockVertexBuffer() override { return m_vb->Unlock(); }
	HRESULT STDMETHODCALLTYPE LockIndexBuffer(DWORD f, BYTE** pp) override { return m_ib->Lock(0, 0, pp, f); }
	HRESULT STDMETHODCALLTYPE UnlockIndexBuffer() override { return m_ib->Unlock(); }
	HRESULT STDMETHODCALLTYPE GetAttributeTable(D3DXATTRIBUTERANGE* tab, DWORD* size) override
	{
		if (!size) return D3DERR_INVALIDCALL;
		if (tab && *size >= 1)
		{
			tab[0].AttribId = 0;
			tab[0].FaceStart = 0;
			tab[0].FaceCount = m_faces;
			tab[0].VertexStart = 0;
			tab[0].VertexCount = m_verts;
		}
		*size = 1;
		return D3D_OK;
	}
	HRESULT STDMETHODCALLTYPE ConvertPointRepsToAdjacency(CONST DWORD*, DWORD*) override { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE ConvertAdjacencyToPointReps(CONST DWORD*, DWORD*) override { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE GenerateAdjacency(FLOAT, DWORD*) override { return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE LockAttributeBuffer(DWORD, DWORD** pp) override
	{
		if (!pp) return D3DERR_INVALIDCALL;
		*pp = m_attr.data();
		return D3D_OK;
	}
	HRESULT STDMETHODCALLTYPE UnlockAttributeBuffer() override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE Optimize(DWORD, CONST DWORD*, DWORD*, DWORD*, LPD3DXBUFFER*, LPD3DXMESH* pp) override { if (pp) *pp = NULL; return E_NOTIMPL; }
	HRESULT STDMETHODCALLTYPE OptimizeInplace(DWORD, CONST DWORD*, DWORD*, DWORD*, LPD3DXBUFFER*) override { return D3D_OK; }

	IDirect3DVertexBuffer8* m_vb = nullptr;
	IDirect3DIndexBuffer8* m_ib = nullptr;

private:
	std::atomic<ULONG> m_ref{ 1 };
	IDirect3DDevice8* m_dev;
	DWORD m_faces, m_verts, m_options, m_fvf;
	std::vector<DWORD> m_attr;
};

struct PN { float p[3]; float n[3]; };

HRESULT FinishMesh(IDirect3DDevice8* dev, const std::vector<PN>& v, const std::vector<WORD>& idx, LPD3DXMESH* ppMesh, LPD3DXBUFFER* ppAdj)
{
	XMesh* m = new XMesh(dev, (DWORD)(idx.size() / 3), (DWORD)v.size(), D3DXMESH_MANAGED, D3DFVF_XYZ | D3DFVF_NORMAL);
	if (!m->Create())
	{
		m->Release();
		return E_OUTOFMEMORY;
	}
	BYTE* p;
	m->m_vb->Lock(0, 0, &p, 0);
	memcpy(p, v.data(), v.size() * sizeof(PN));
	m->m_vb->Unlock();
	m->m_ib->Lock(0, 0, &p, 0);
	memcpy(p, idx.data(), idx.size() * sizeof(WORD));
	m->m_ib->Unlock();
	*ppMesh = m;
	if (ppAdj)
	{
		// Cynex: komsuluk hesaplanmiyor, -1 ile dolu tampon
		XBuffer* b = new XBuffer((DWORD)(idx.size() * sizeof(DWORD)));
		memset(b->GetBufferPointer(), 0xFF, b->GetBufferSize());
		*ppAdj = b;
	}
	return D3D_OK;
}

}	// namespace

// ---------------------------------------------------------------- disa acik

extern "C" HRESULT WINAPI D3DXCreateBuffer(DWORD NumBytes, LPD3DXBUFFER* ppBuffer)
{
	if (!ppBuffer) return D3DERR_INVALIDCALL;
	*ppBuffer = new XBuffer(NumBytes);
	return D3D_OK;
}

HRESULT WINAPI D3DXCreateMatrixStack(DWORD, LPD3DXMATRIXSTACK* ppStack)
{
	if (!ppStack) return D3DERR_INVALIDCALL;
	*ppStack = new XMatrixStack();
	return D3D_OK;
}

UINT WINAPI D3DXGetFVFVertexSize(DWORD fvf)
{
	return FVFStride(fvf);
}

HRESULT WINAPI D3DXDeclaratorFromFVF(DWORD fvf, DWORD decl[MAX_FVF_DECL_SIZE])
{
	if (!decl) return D3DERR_INVALIDCALL;
	int n = 0;
	decl[n++] = D3DVSD_STREAM(0);
	DWORD pos = fvf & D3DFVF_POSITION_MASK;
	if (pos == D3DFVF_XYZRHW) decl[n++] = D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT4);
	else if (pos) decl[n++] = D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3);
	if (fvf & D3DFVF_NORMAL) decl[n++] = D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3);
	if (fvf & D3DFVF_PSIZE) decl[n++] = D3DVSD_REG(D3DVSDE_PSIZE, D3DVSDT_FLOAT1);
	if (fvf & D3DFVF_DIFFUSE) decl[n++] = D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR);
	if (fvf & D3DFVF_SPECULAR) decl[n++] = D3DVSD_REG(D3DVSDE_SPECULAR, D3DVSDT_D3DCOLOR);
	UINT nt = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
	for (UINT i = 0; i < nt && i < 8 && n < MAX_FVF_DECL_SIZE - 1; ++i)
	{
		DWORD f = (fvf >> (16 + i * 2)) & 3;
		DWORD t = f == D3DFVF_TEXTUREFORMAT1 ? D3DVSDT_FLOAT1 : f == D3DFVF_TEXTUREFORMAT3 ? D3DVSDT_FLOAT3 : f == D3DFVF_TEXTUREFORMAT4 ? D3DVSDT_FLOAT4 : D3DVSDT_FLOAT2;
		decl[n++] = D3DVSD_REG(D3DVSDE_TEXCOORD0 + i, t);
	}
	decl[n++] = D3DVSD_END();
	return D3D_OK;
}

HRESULT WINAPI D3DXCreateSphere(LPDIRECT3DDEVICE8 dev, FLOAT r, UINT slices, UINT stacks, LPD3DXMESH* ppMesh, LPD3DXBUFFER* ppAdj)
{
	if (!dev || !ppMesh || slices < 2 || stacks < 2)
		return D3DERR_INVALIDCALL;
	std::vector<PN> v;
	std::vector<WORD> idx;
	// Cynex: kutuplar z ekseninde, d3dx gibi
	v.push_back({ { 0, 0, r }, { 0, 0, 1 } });
	for (UINT i = 1; i < stacks; ++i)
	{
		float phi = (float)M_PI * i / stacks;
		float sp = sinf(phi), cp = cosf(phi);
		for (UINT j = 0; j < slices; ++j)
		{
			float th = 2.0f * (float)M_PI * j / slices;
			float nx = sp * cosf(th), ny = sp * sinf(th), nz = cp;
			v.push_back({ { r * nx, r * ny, r * nz }, { nx, ny, nz } });
		}
	}
	v.push_back({ { 0, 0, -r }, { 0, 0, -1 } });
	const WORD bottom = (WORD)(v.size() - 1);
	for (UINT j = 0; j < slices; ++j)
	{
		WORD a = (WORD)(1 + j), b = (WORD)(1 + (j + 1) % slices);
		idx.push_back(0); idx.push_back(b); idx.push_back(a);
	}
	for (UINT i = 0; i + 2 < stacks; ++i)
		for (UINT j = 0; j < slices; ++j)
		{
			WORD a = (WORD)(1 + i * slices + j), b = (WORD)(1 + i * slices + (j + 1) % slices);
			WORD c = (WORD)(a + slices), d = (WORD)(b + slices);
			idx.push_back(a); idx.push_back(b); idx.push_back(c);
			idx.push_back(b); idx.push_back(d); idx.push_back(c);
		}
	const WORD last = (WORD)(1 + (stacks - 2) * slices);
	for (UINT j = 0; j < slices; ++j)
	{
		WORD a = (WORD)(last + j), b = (WORD)(last + (j + 1) % slices);
		idx.push_back(bottom); idx.push_back(a); idx.push_back(b);
	}
	return FinishMesh(dev, v, idx, ppMesh, ppAdj);
}

HRESULT WINAPI D3DXCreateCylinder(LPDIRECT3DDEVICE8 dev, FLOAT r1, FLOAT r2, FLOAT len, UINT slices, UINT stacks, LPD3DXMESH* ppMesh, LPD3DXBUFFER* ppAdj)
{
	if (!dev || !ppMesh || slices < 2 || stacks < 1)
		return D3DERR_INVALIDCALL;
	std::vector<PN> v;
	std::vector<WORD> idx;
	const float z0 = -len * 0.5f;
	// Cynex: r1 -z ucunda, r2 +z ucunda; egim normale yansir
	const float slope = len != 0.0f ? (r1 - r2) / len : 0.0f;
	for (UINT i = 0; i <= stacks; ++i)
	{
		float t = (float)i / stacks;
		float z = z0 + len * t, r = r1 + (r2 - r1) * t;
		for (UINT j = 0; j < slices; ++j)
		{
			float th = 2.0f * (float)M_PI * j / slices;
			float c = cosf(th), s = sinf(th);
			float nl = sqrtf(1.0f + slope * slope);
			v.push_back({ { r * c, r * s, z }, { c / nl, s / nl, slope / nl } });
		}
	}
	for (UINT i = 0; i < stacks; ++i)
		for (UINT j = 0; j < slices; ++j)
		{
			WORD a = (WORD)(i * slices + j), b = (WORD)(i * slices + (j + 1) % slices);
			WORD c = (WORD)(a + slices), d = (WORD)(b + slices);
			idx.push_back(a); idx.push_back(c); idx.push_back(b);
			idx.push_back(b); idx.push_back(c); idx.push_back(d);
		}
	// Cynex: kapaklar
	for (int cap = 0; cap < 2; ++cap)
	{
		float z = cap ? -z0 : z0, r = cap ? r2 : r1, nz = cap ? 1.0f : -1.0f;
		WORD center = (WORD)v.size();
		v.push_back({ { 0, 0, z }, { 0, 0, nz } });
		WORD ring = (WORD)v.size();
		for (UINT j = 0; j < slices; ++j)
		{
			float th = 2.0f * (float)M_PI * j / slices;
			v.push_back({ { r * cosf(th), r * sinf(th), z }, { 0, 0, nz } });
		}
		for (UINT j = 0; j < slices; ++j)
		{
			WORD a = (WORD)(ring + j), b = (WORD)(ring + (j + 1) % slices);
			idx.push_back(center);
			if (cap) { idx.push_back(b); idx.push_back(a); }
			else { idx.push_back(a); idx.push_back(b); }
		}
	}
	return FinishMesh(dev, v, idx, ppMesh, ppAdj);
}
