// Cynex: d3d8 kaynaklari (doku, yuzey, tamponlar)
#pragma once
#include "cd3d_common.h"

namespace cd3d {

// Cynex: gl silme - baska iplikten gelirse kuyruga
void GLDeleteTexture(GLuint t);
void GLDeleteBuffer(GLuint b);
void GLDeleteRenderbuffer(GLuint r);
extern uint32_t g_frame;
// Cynex: gecici olcum sayaclari
#include <time.h>
struct PerfCtr { uint64_t drawNs = 0, upNs = 0, prepNs = 0, bindNs = 0, syncNs = 0, pushNs = 0, vsyncNs = 0; uint32_t draws = 0, uploads = 0, preps = 0, binds = 0, syncs = 0, pushes = 0, vsyncs = 0; uint64_t upBytes = 0; };
extern PerfCtr g_perf;
static inline uint64_t PerfNow() { timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); return (uint64_t)ts.tv_sec * 1000000000ull + ts.tv_nsec; }
struct PerfScope { uint64_t& acc; uint32_t& cnt; uint64_t t0; PerfScope(uint64_t& a, uint32_t& c) : acc(a), cnt(c), t0(PerfNow()) {} ~PerfScope() { acc += PerfNow() - t0; ++cnt; } };

// ---------------------------------------------------------------- doku

class Texture final : public IDirect3DTexture8
{
public:
	struct Level
	{
		UINT w = 0, h = 0, pitch = 0, size = 0;
		uint8_t* data = nullptr;
		bool dirty = false;
		bool fullDirty = false;
		RECT dirtyRect = { 0, 0, 0, 0 };
		int lockCount = 0;
		int lockTimes = 0;
	};

	Texture(Device* dev, UINT w, UINT h, UINT levels, DWORD usage, D3DFORMAT fmt, D3DPOOL pool);
	~Texture() override;

	CD3D_IUNKNOWN_IMPL(IDirect3DTexture8)
	ULONG STDMETHODCALLTYPE AddRef() override { return ++m_ref; }
	ULONG STDMETHODCALLTYPE Release() override;

	HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice8** ppDevice) override;
	HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, CONST void*, DWORD, DWORD) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, void*, DWORD*) override { return D3DERR_NOTFOUND; }
	HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID) override { return D3D_OK; }
	DWORD STDMETHODCALLTYPE SetPriority(DWORD p) override { DWORD o = m_priority; m_priority = p; return o; }
	DWORD STDMETHODCALLTYPE GetPriority() override { return m_priority; }
	void STDMETHODCALLTYPE PreLoad() override {}
	D3DRESOURCETYPE STDMETHODCALLTYPE GetType() override { return D3DRTYPE_TEXTURE; }
	DWORD STDMETHODCALLTYPE SetLOD(DWORD lod) override { DWORD o = m_lod; m_lod = lod; return o; }
	DWORD STDMETHODCALLTYPE GetLOD() override { return m_lod; }
	DWORD STDMETHODCALLTYPE GetLevelCount() override { return m_levels; }
	HRESULT STDMETHODCALLTYPE GetLevelDesc(UINT Level, D3DSURFACE_DESC* pDesc) override;
	HRESULT STDMETHODCALLTYPE GetSurfaceLevel(UINT Level, IDirect3DSurface8** ppSurfaceLevel) override;
	HRESULT STDMETHODCALLTYPE LockRect(UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) override;
	HRESULT STDMETHODCALLTYPE UnlockRect(UINT Level) override;
	HRESULT STDMETHODCALLTYPE AddDirtyRect(CONST RECT* pDirtyRect) override;

	// Cynex: ic kullanim
	bool Sync();					// kirli seviyeleri gl'ye yukle (cizim ipliginde)
	GLuint GL() const { return m_gl; }
	bool EnsureGL();
	uint8_t* LevelData(UINT level, bool alloc);
	void MarkLevelDirty(UINT level, const RECT* r);
	bool IsRenderTarget() const { return m_isRT; }
	D3DFORMAT Format() const { return m_fmt; }
	UINT Width() const { return m_w; }
	UINT Height() const { return m_h; }
	UINT Levels() const { return m_levels; }
	const Level& Lv(UINT i) const { return m_lv[i]; }
	void SetFromFile(bool autoMip) { m_fromFile = true; m_autoMip = autoMip; m_keepShadow = (m_pool == D3DPOOL_SYSTEMMEM || m_pool == D3DPOOL_SCRATCH); }
	bool TargetHasAlpha() const { return m_fmt == D3DFMT_A8R8G8B8 || m_fmt == D3DFMT_A4R4G4B4 || m_fmt == D3DFMT_A1R5G5B5; }

	Device* m_dev;

private:
	void FreeShadows();
	void UploadLevel(UINT i);

	std::atomic<ULONG> m_ref{ 1 };
	UINT m_w, m_h, m_levels;
	DWORD m_usage;
	D3DFORMAT m_fmt;
	D3DPOOL m_pool;
	DWORD m_priority = 0, m_lod = 0;
	std::vector<Level> m_lv;
	Surface** m_surfaces = nullptr;

	bool m_isRT = false;
	bool m_fromFile = false;
	bool m_autoMip = false;
	bool m_keepShadow = false;
	bool m_dirtyAny = false;
	bool m_mipsGenerated = false;

	GLuint m_gl = 0;
	GLUploadFmt m_up;
	int m_glLevels = 0;
	int m_convert = CONV_NONE;
	uint32_t m_validMask = 0;
	int m_maxLevelSet = -1;
};

// ---------------------------------------------------------------- yuzey

enum SurfKind
{
	SURF_TEXLEVEL,
	SURF_BACKBUFFER,
	SURF_DEPTH,
	SURF_AUTODEPTH,
	SURF_IMAGE,
	SURF_RENDERTARGET,
};

class Surface final : public IDirect3DSurface8
{
public:
	Surface(Device* dev, SurfKind kind, UINT w, UINT h, D3DFORMAT fmt, D3DPOOL pool, DWORD usage);
	Surface(Texture* container, UINT level);
	~Surface() override;

	CD3D_IUNKNOWN_IMPL(IDirect3DSurface8)
	ULONG STDMETHODCALLTYPE AddRef() override;
	ULONG STDMETHODCALLTYPE Release() override;

	HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice8** ppDevice) override;
	HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, CONST void*, DWORD, DWORD) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, void*, DWORD*) override { return D3DERR_NOTFOUND; }
	HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetContainer(REFIID riid, void** ppContainer) override;
	HRESULT STDMETHODCALLTYPE GetDesc(D3DSURFACE_DESC* pDesc) override;
	HRESULT STDMETHODCALLTYPE LockRect(D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) override;
	HRESULT STDMETHODCALLTYPE UnlockRect() override;

	// Cynex: ic kullanim
	GLuint Renderbuffer();			// derinlik / renk rb (tembel)
	bool HasStencil() const { return m_fmt == D3DFMT_D24S8 || m_fmt == D3DFMT_D24X4S4 || m_fmt == D3DFMT_D15S1; }
	bool ColorHasAlpha() const;
	uint8_t* CpuData(UINT* pitch);	// image yuzeyi / doku seviyesi

	Device* m_dev;
	SurfKind m_kind;
	Texture* m_container = nullptr;
	UINT m_level = 0;
	UINT m_w, m_h;
	D3DFORMAT m_fmt;
	D3DPOOL m_pool;
	DWORD m_usage;
	GLuint m_rb = 0;

private:
	std::atomic<ULONG> m_ref{ 1 };
	uint8_t* m_data = nullptr;		// image yuzeyi / geri okuma
	UINT m_pitch = 0;
	bool m_locked = false;
};

// ---------------------------------------------------------------- tamponlar

struct BufferCore
{
	uint8_t* data = nullptr;
	UINT size = 0;
	DWORD usage = 0;
	D3DPOOL pool = D3DPOOL_MANAGED;
	GLuint gl = 0;
	bool glAlloc = false;
	UINT dirtyMin = 0, dirtyMax = 0;	// [min,max)
	bool orphan = false;
	bool uploaded = false;
	uint32_t lastMod = 0xFFFFFF00u, prevMod = 0xFFFFFF00u;
	UINT lockOff = 0, lockLen = 0;
	DWORD lockFlags = 0;
	int lockCount = 0;

	bool Init(UINT sz, DWORD usage, D3DPOOL pool);
	void Free();
	HRESULT Lock(UINT off, UINT len, BYTE** pp, DWORD flags);
	HRESULT Unlock();
	bool Streaming() const;			// son karelerde surekli degisiyorsa halkadan akit
	GLuint Sync(GLenum target);		// kalici gl tamponunu guncelle
};

class VertexBuffer final : public IDirect3DVertexBuffer8
{
public:
	VertexBuffer(Device* dev, DWORD fvf) : m_dev(dev), m_fvf(fvf) {}
	~VertexBuffer() override;

	CD3D_IUNKNOWN_IMPL(IDirect3DVertexBuffer8)
	ULONG STDMETHODCALLTYPE AddRef() override { return ++m_ref; }
	ULONG STDMETHODCALLTYPE Release() override;

	HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice8** ppDevice) override;
	HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, CONST void*, DWORD, DWORD) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, void*, DWORD*) override { return D3DERR_NOTFOUND; }
	HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID) override { return D3D_OK; }
	DWORD STDMETHODCALLTYPE SetPriority(DWORD p) override { DWORD o = m_priority; m_priority = p; return o; }
	DWORD STDMETHODCALLTYPE GetPriority() override { return m_priority; }
	void STDMETHODCALLTYPE PreLoad() override {}
	D3DRESOURCETYPE STDMETHODCALLTYPE GetType() override { return D3DRTYPE_VERTEXBUFFER; }
	HRESULT STDMETHODCALLTYPE Lock(UINT OffsetToLock, UINT SizeToLock, BYTE** ppbData, DWORD Flags) override { return m_core.Lock(OffsetToLock, SizeToLock, ppbData, Flags); }
	HRESULT STDMETHODCALLTYPE Unlock() override { return m_core.Unlock(); }
	HRESULT STDMETHODCALLTYPE GetDesc(D3DVERTEXBUFFER_DESC* pDesc) override;

	Device* m_dev;
	DWORD m_fvf;
	BufferCore m_core;

private:
	std::atomic<ULONG> m_ref{ 1 };
	DWORD m_priority = 0;
};

class IndexBuffer final : public IDirect3DIndexBuffer8
{
public:
	IndexBuffer(Device* dev, D3DFORMAT fmt) : m_dev(dev), m_fmt(fmt) {}
	~IndexBuffer() override;

	CD3D_IUNKNOWN_IMPL(IDirect3DIndexBuffer8)
	ULONG STDMETHODCALLTYPE AddRef() override { return ++m_ref; }
	ULONG STDMETHODCALLTYPE Release() override;

	HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice8** ppDevice) override;
	HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, CONST void*, DWORD, DWORD) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, void*, DWORD*) override { return D3DERR_NOTFOUND; }
	HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID) override { return D3D_OK; }
	DWORD STDMETHODCALLTYPE SetPriority(DWORD p) override { DWORD o = m_priority; m_priority = p; return o; }
	DWORD STDMETHODCALLTYPE GetPriority() override { return m_priority; }
	void STDMETHODCALLTYPE PreLoad() override {}
	D3DRESOURCETYPE STDMETHODCALLTYPE GetType() override { return D3DRTYPE_INDEXBUFFER; }
	HRESULT STDMETHODCALLTYPE Lock(UINT OffsetToLock, UINT SizeToLock, BYTE** ppbData, DWORD Flags) override { return m_core.Lock(OffsetToLock, SizeToLock, ppbData, Flags); }
	HRESULT STDMETHODCALLTYPE Unlock() override { return m_core.Unlock(); }
	HRESULT STDMETHODCALLTYPE GetDesc(D3DINDEXBUFFER_DESC* pDesc) override;

	Device* m_dev;
	D3DFORMAT m_fmt;
	BufferCore m_core;

private:
	std::atomic<ULONG> m_ref{ 1 };
	DWORD m_priority = 0;
};

}	// namespace cd3d
