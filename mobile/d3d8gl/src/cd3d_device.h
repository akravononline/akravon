// Cynex: IDirect3DDevice8 uygulamasi
#pragma once
#include "cd3d_common.h"
#include "cd3d_resource.h"
#include "cd3d_ffp.h"

namespace cd3d {

class Direct3D8;

struct VElem
{
	uint8_t attrib;
	uint8_t stream;
	uint8_t comps;
	uint8_t normalized;
	uint16_t offset;
	GLenum type;
};

struct VLayout
{
	VElem el[16];
	int count = 0;
	uint8_t posRHW = 0, hasNormal = 0, hasDiffuse = 0, hasSpecular = 0;
	uint8_t texSize[8] = { 0 };
	uint16_t streamMask = 0;
	UINT fvfStride = 0;
};

bool BuildFVFLayout(DWORD fvf, VLayout* out);
UINT FVFStride(DWORD fvf);

struct VShaderDecl
{
	VLayout layout;
	std::vector<DWORD> tokens;
};

// Cynex: gl durum onbellegi - gereksiz cagri yok
struct GLCache
{
	GLuint program = 0;
	GLuint fbo = 0xFFFFFFFF;
	GLuint arrayBuf = 0, elemBuf = 0;
	int activeUnit = -1;
	GLuint tex[8] = { 0 };
	GLuint sampler[8] = { 0 };
	bool blend = false;
	GLenum bsrc = GL_ONE, bdst = GL_ZERO, bop = GL_FUNC_ADD;
	bool depthTest = false;
	GLenum depthFunc = GL_LESS;
	bool depthMask = true;
	bool cull = false;
	GLenum frontFace = GL_CCW;
	bool stencil = false;
	GLenum sfunc = GL_ALWAYS; GLint sref = 0; GLuint smask = 0xFFFFFFFF;
	GLenum sfail = GL_KEEP, szfail = GL_KEEP, spass = GL_KEEP;
	GLuint swmask = 0xFFFFFFFF;
	bool colorMask[4] = { true, true, true, true };
	bool scissor = false;
	bool polyOffset = false;
	float polyFactor = 0, polyUnits = 0;
	GLint vp[4] = { -1, -1, -1, -1 };
	float depthRange[2] = { -1, -1 };
	uint32_t attribEnabled = 0;
	struct Attr { GLuint buf; GLintptr off; GLsizei stride; GLenum type; GLint comps; GLboolean norm; } attr[ATTR_COUNT];
	void Invalidate();
};

// Cynex: akis halkasi (UP cizimleri, sik degisen tamponlar)
struct StreamRing
{
	GLuint buf = 0;
	GLenum target = GL_ARRAY_BUFFER;
	size_t size = 0, pos = 0;
	bool Init(GLenum target, size_t size);
	void Destroy();
	size_t Push(GLCache& c, const void* data, size_t bytes, size_t align);
	void* Map(GLCache& c, size_t bytes, size_t align, size_t* outOff);
	void Unmap();
	void Bind(GLCache& c);
	void EndFrame();	// Cynex: kalici eslenmis halka: kare sonu citi + sonraki dilime gec
private:
	static const int kSeg = 3;
	bool m_persist = false;
	uint8_t* m_base = nullptr;
	size_t m_segSize = 0, m_segStart = 0;
	int m_seg = 0;
	void* m_fence[3] = { nullptr, nullptr, nullptr };
	bool m_mapped = false;
	bool m_direct = false;
	uint8_t* m_tmp = nullptr;
	size_t m_tmpSize = 0, m_tmpOff = 0, m_tmpBytes = 0;
};

class Device final : public IDirect3DDevice8
{
public:
	Device(IDirect3D8* d3d, HWND hwnd, DWORD behavior, D3DPRESENT_PARAMETERS* pp);
	~Device() override;
	bool Init();

	CD3D_IUNKNOWN_IMPL(IDirect3DDevice8)
	ULONG STDMETHODCALLTYPE AddRef() override { return ++m_ref; }
	ULONG STDMETHODCALLTYPE Release() override;

	HRESULT STDMETHODCALLTYPE TestCooperativeLevel() override;
	UINT STDMETHODCALLTYPE GetAvailableTextureMem() override;
	HRESULT STDMETHODCALLTYPE ResourceManagerDiscardBytes(DWORD) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetDirect3D(IDirect3D8** ppD3D8) override;
	HRESULT STDMETHODCALLTYPE GetDeviceCaps(D3DCAPS8* pCaps) override;
	HRESULT STDMETHODCALLTYPE GetDisplayMode(D3DDISPLAYMODE* pMode) override;
	HRESULT STDMETHODCALLTYPE GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* p) override;
	HRESULT STDMETHODCALLTYPE SetCursorProperties(UINT, UINT, IDirect3DSurface8*) override { return D3D_OK; }
	void STDMETHODCALLTYPE SetCursorPosition(UINT, UINT, DWORD) override {}
	BOOL STDMETHODCALLTYPE ShowCursor(BOOL) override { return FALSE; }
	HRESULT STDMETHODCALLTYPE CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS*, IDirect3DSwapChain8** pp) override { if (pp) *pp = NULL; return D3DERR_NOTAVAILABLE; }
	HRESULT STDMETHODCALLTYPE Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) override;
	HRESULT STDMETHODCALLTYPE Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) override;
	HRESULT STDMETHODCALLTYPE GetBackBuffer(UINT BackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface8** ppBackBuffer) override;
	HRESULT STDMETHODCALLTYPE GetRasterStatus(D3DRASTER_STATUS* p) override;
	void STDMETHODCALLTYPE SetGammaRamp(DWORD Flags, CONST D3DGAMMARAMP* pRamp) override;
	void STDMETHODCALLTYPE GetGammaRamp(D3DGAMMARAMP* pRamp) override;
	HRESULT STDMETHODCALLTYPE CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture8** ppTexture) override;
	HRESULT STDMETHODCALLTYPE CreateVolumeTexture(UINT, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DVolumeTexture8** pp) override { if (pp) *pp = NULL; return D3DERR_NOTAVAILABLE; }
	HRESULT STDMETHODCALLTYPE CreateCubeTexture(UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DCubeTexture8** pp) override { if (pp) *pp = NULL; return D3DERR_NOTAVAILABLE; }
	HRESULT STDMETHODCALLTYPE CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer8** ppVertexBuffer) override;
	HRESULT STDMETHODCALLTYPE CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer8** ppIndexBuffer) override;
	HRESULT STDMETHODCALLTYPE CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, BOOL Lockable, IDirect3DSurface8** ppSurface) override;
	HRESULT STDMETHODCALLTYPE CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8** ppSurface) override;
	HRESULT STDMETHODCALLTYPE CreateImageSurface(UINT Width, UINT Height, D3DFORMAT Format, IDirect3DSurface8** ppSurface) override;
	HRESULT STDMETHODCALLTYPE CopyRects(IDirect3DSurface8* pSourceSurface, CONST RECT* pSourceRectsArray, UINT cRects, IDirect3DSurface8* pDestinationSurface, CONST POINT* pDestPointsArray) override;
	HRESULT STDMETHODCALLTYPE UpdateTexture(IDirect3DBaseTexture8* pSourceTexture, IDirect3DBaseTexture8* pDestinationTexture) override;
	HRESULT STDMETHODCALLTYPE GetFrontBuffer(IDirect3DSurface8* pDestSurface) override;
	HRESULT STDMETHODCALLTYPE SetRenderTarget(IDirect3DSurface8* pRenderTarget, IDirect3DSurface8* pNewZStencil) override;
	HRESULT STDMETHODCALLTYPE GetRenderTarget(IDirect3DSurface8** ppRenderTarget) override;
	HRESULT STDMETHODCALLTYPE GetDepthStencilSurface(IDirect3DSurface8** ppZStencilSurface) override;
	HRESULT STDMETHODCALLTYPE BeginScene() override;
	HRESULT STDMETHODCALLTYPE EndScene() override;
	HRESULT STDMETHODCALLTYPE Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) override;
	HRESULT STDMETHODCALLTYPE SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix) override;
	HRESULT STDMETHODCALLTYPE GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) override;
	HRESULT STDMETHODCALLTYPE MultiplyTransform(D3DTRANSFORMSTATETYPE, CONST D3DMATRIX*) override;
	HRESULT STDMETHODCALLTYPE SetViewport(CONST D3DVIEWPORT8* pViewport) override;
	HRESULT STDMETHODCALLTYPE GetViewport(D3DVIEWPORT8* pViewport) override;
	HRESULT STDMETHODCALLTYPE SetMaterial(CONST D3DMATERIAL8* pMaterial) override;
	HRESULT STDMETHODCALLTYPE GetMaterial(D3DMATERIAL8* pMaterial) override;
	HRESULT STDMETHODCALLTYPE SetLight(DWORD Index, CONST D3DLIGHT8*) override;
	HRESULT STDMETHODCALLTYPE GetLight(DWORD Index, D3DLIGHT8*) override;
	HRESULT STDMETHODCALLTYPE LightEnable(DWORD Index, BOOL Enable) override;
	HRESULT STDMETHODCALLTYPE GetLightEnable(DWORD Index, BOOL* pEnable) override;
	HRESULT STDMETHODCALLTYPE SetClipPlane(DWORD Index, CONST float* pPlane) override;
	HRESULT STDMETHODCALLTYPE GetClipPlane(DWORD Index, float* pPlane) override;
	HRESULT STDMETHODCALLTYPE SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) override;
	HRESULT STDMETHODCALLTYPE GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) override;
	HRESULT STDMETHODCALLTYPE BeginStateBlock() override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE EndStateBlock(DWORD*) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE ApplyStateBlock(DWORD) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE CaptureStateBlock(DWORD) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE DeleteStateBlock(DWORD) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE CreateStateBlock(D3DSTATEBLOCKTYPE, DWORD*) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE SetClipStatus(CONST D3DCLIPSTATUS8*) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetClipStatus(D3DCLIPSTATUS8* p) override { if (p) memset(p, 0, sizeof(*p)); return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetTexture(DWORD Stage, IDirect3DBaseTexture8** ppTexture) override;
	HRESULT STDMETHODCALLTYPE SetTexture(DWORD Stage, IDirect3DBaseTexture8* pTexture) override;
	HRESULT STDMETHODCALLTYPE GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) override;
	HRESULT STDMETHODCALLTYPE SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) override;
	HRESULT STDMETHODCALLTYPE ValidateDevice(DWORD* pNumPasses) override { if (pNumPasses) *pNumPasses = 1; return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetInfo(DWORD, void*, DWORD) override { return S_FALSE; }
	HRESULT STDMETHODCALLTYPE SetPaletteEntries(UINT, CONST PALETTEENTRY*) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetPaletteEntries(UINT, PALETTEENTRY*) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE SetCurrentTexturePalette(UINT) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetCurrentTexturePalette(UINT* p) override { if (p) *p = 0; return D3D_OK; }
	HRESULT STDMETHODCALLTYPE DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) override;
	HRESULT STDMETHODCALLTYPE DrawIndexedPrimitive(D3DPRIMITIVETYPE, UINT minIndex, UINT NumVertices, UINT startIndex, UINT primCount) override;
	HRESULT STDMETHODCALLTYPE DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) override;
	HRESULT STDMETHODCALLTYPE DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) override;
	HRESULT STDMETHODCALLTYPE ProcessVertices(UINT, UINT, UINT, IDirect3DVertexBuffer8*, DWORD) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE CreateVertexShader(CONST DWORD* pDeclaration, CONST DWORD* pFunction, DWORD* pHandle, DWORD Usage) override;
	HRESULT STDMETHODCALLTYPE SetVertexShader(DWORD Handle) override;
	HRESULT STDMETHODCALLTYPE GetVertexShader(DWORD* pHandle) override;
	HRESULT STDMETHODCALLTYPE DeleteVertexShader(DWORD Handle) override;
	HRESULT STDMETHODCALLTYPE SetVertexShaderConstant(DWORD Register, CONST void* pConstantData, DWORD ConstantCount) override;
	HRESULT STDMETHODCALLTYPE GetVertexShaderConstant(DWORD Register, void* pConstantData, DWORD ConstantCount) override;
	HRESULT STDMETHODCALLTYPE GetVertexShaderDeclaration(DWORD Handle, void* pData, DWORD* pSizeOfData) override;
	HRESULT STDMETHODCALLTYPE GetVertexShaderFunction(DWORD, void*, DWORD* pSize) override { if (pSize) *pSize = 0; return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer8* pStreamData, UINT Stride) override;
	HRESULT STDMETHODCALLTYPE GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer8** ppStreamData, UINT* pStride) override;
	HRESULT STDMETHODCALLTYPE SetIndices(IDirect3DIndexBuffer8* pIndexData, UINT BaseVertexIndex) override;
	HRESULT STDMETHODCALLTYPE GetIndices(IDirect3DIndexBuffer8** ppIndexData, UINT* pBaseVertexIndex) override;
	HRESULT STDMETHODCALLTYPE CreatePixelShader(CONST DWORD*, DWORD* pHandle) override { if (pHandle) *pHandle = 0; return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE SetPixelShader(DWORD Handle) override { m_ps = Handle; return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetPixelShader(DWORD* pHandle) override { if (pHandle) *pHandle = m_ps; return D3D_OK; }
	HRESULT STDMETHODCALLTYPE DeletePixelShader(DWORD) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE SetPixelShaderConstant(DWORD, CONST void*, DWORD) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetPixelShaderConstant(DWORD, void*, DWORD) override { return D3D_OK; }
	HRESULT STDMETHODCALLTYPE GetPixelShaderFunction(DWORD, void*, DWORD* pSize) override { if (pSize) *pSize = 0; return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE DrawRectPatch(UINT, CONST float*, CONST D3DRECTPATCH_INFO*) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE DrawTriPatch(UINT, CONST float*, CONST D3DTRIPATCH_INFO*) override { return D3DERR_INVALIDCALL; }
	HRESULT STDMETHODCALLTYPE DeletePatch(UINT) override { return D3DERR_INVALIDCALL; }

	// Cynex: ic kullanim
	void OnTextureDestroyed(Texture* t);
	void OnSurfaceDestroyed(Surface* s);
	void OnBufferDestroyed(void* b);
	void OnWindow(struct ANativeWindow* window, bool created);
	void QueueDelete(int kind, GLuint name);
	void FlushDeletes();
	bool ReadBackbuffer(uint8_t* dst, UINT pitch);	// bgra, ust satir once
	GLCache& Cache() { return m_gl; }
	void BindTextureUnit(int unit, GLuint tex);
	const D3DPRESENT_PARAMETERS& PP() const { return m_pp; }
	void ApplyPostFilter(int mode);	// AKRAVON: 0 kapali, 1 dogal, 2 canli - o ana kadar cizilen dunyaya uygulanir

private:
	enum
	{
		DIRTY_KEY = 1 << 0,
		DIRTY_RASTER = 1 << 1,
		DIRTY_FBO = 1 << 2,
		DIRTY_VIEWPORT = 1 << 3,
		DIRTY_LIGHTLIST = 1 << 4,
		DIRTY_SAMPLERS = 1 << 5,
	};

	struct LightSlot { D3DLIGHT8 l; bool enabled; bool valid; };
	struct Stream { VertexBuffer* vb; UINT stride; };

	void SetDefaultStates();
	void Bump(int g) { ++m_serial[g]; }
	const VLayout* CurrentLayout();
	void BuildKey(const VLayout* lay, bool isPoint, FFKey* key);
	bool PrepareDraw(D3DPRIMITIVETYPE type);
	void ApplyRaster();
	void ApplyFramebuffer();
	void ApplyViewport();
	void ApplyTextures(const FFKey& key);
	void UploadUniforms(FFProgram* p, const FFKey& key);
	void UpdateLightList();
	GLuint GetSampler(DWORD stage);
	GLuint GetFBO(Surface* color, Surface* depth, bool* hasDepth, bool* hasStencil);
	void EnsureOffscreenBackbuffer();
	void DestroyOffscreenBackbuffer();
	GLuint ResolveBackbuffer();
	bool TargetIsBackbuffer() const { return m_rt == m_backbuffer; }
	void TargetSize(UINT* w, UINT* h) const;
	bool BindVertexStreams(const VLayout* lay, UINT vmin, UINT vcount, int upStream, const void* upData, UINT upStride);
	void SetAttrib(int attr, GLuint buf, GLintptr off, GLsizei stride, GLenum type, GLint comps, GLboolean norm);
	void DisableUnusedAttribs(uint32_t usedMask);
	HRESULT DrawIndexedCommon(D3DPRIMITIVETYPE type, UINT minIndex, UINT numVertices, UINT primCount,
		const void* cpuIndices, GLuint glIb, size_t glIbOffset, bool idx32, int upStream, const void* upData, UINT upStride);

	IDirect3D8* m_d3d;
	std::atomic<ULONG> m_ref{ 1 };
	HWND m_hwnd;
	DWORD m_behavior;
	D3DPRESENT_PARAMETERS m_pp;
	bool m_inScene = false;

	// Cynex: d3d durumu
	DWORD m_rs[256];
	DWORD m_tss[8][32];
	D3DMATRIX m_world, m_view, m_proj, m_texMat[8];
	D3DMATRIX m_worldExtra[3];
	D3DMATERIAL8 m_mat;
	std::vector<LightSlot> m_lights;
	int m_activeLights[8];
	int m_numActiveLights = 0;
	D3DVIEWPORT8 m_vp;
	float m_clipPlanes[6][4];
	Texture* m_tex[8];
	Stream m_streams[16];
	IndexBuffer* m_ib = nullptr;
	UINT m_baseVertex = 0;
	DWORD m_vs = 0;
	DWORD m_ps = 0;
	std::vector<VShaderDecl*> m_vsTable;
	std::vector<float> m_vsConst;
	D3DGAMMARAMP m_gamma;

	Surface* m_rt = nullptr;
	Surface* m_ds = nullptr;
	Surface* m_backbuffer = nullptr;
	Surface* m_autoDepth = nullptr;

	// Cynex: turetilmis durum
	uint32_t m_dirty = 0xFFFFFFFF;
	uint32_t m_serial[SG_COUNT];
	uint32_t m_mvpSerial = 0;
	D3DMATRIX m_mvp, m_wv;
	float m_normalMat[9];
	FFKey m_key;
	FFProgram* m_prog = nullptr;
	FFCache m_ffp;
	std::unordered_map<DWORD, VLayout> m_fvfLayouts;
	DWORD m_lastLayoutVS = 0xFFFFFFFF;
	const VLayout* m_lastLayout = nullptr;
	bool m_flip = false;			// fbo hedefinde y ters
	bool m_targetAlpha = false;
	bool m_targetDepth = true;
	bool m_targetStencil = true;
	bool m_targetValid = false;
	UINT m_targetW = 0, m_targetH = 0;

	// Cynex: gl nesneleri
	GLCache m_gl;
	GLuint m_vao = 0;
	StreamRing m_vring, m_iring;
	std::unordered_map<uint64_t, GLuint> m_samplers;
	struct FboEntry { Surface* color; Surface* depth; GLuint fbo; GLuint privDepth; bool hasDepth, hasStencil; };
	std::vector<FboEntry> m_fbos;
	GLuint m_bbFbo = 0, m_bbColor = 0, m_bbDepth = 0;
	GLuint m_bbResolveFbo = 0, m_bbResolveColor = 0;	// AKRAVON: coklu ornekli tamponun cozuldugu tek ornekli kopya
	GLint m_bbSamples = 0;
	UINT m_bbW = 0, m_bbH = 0;		// AKRAVON: ekran disi arka tamponun fiziksel boyutu
	bool m_bbOffscreen = false;
	float m_vpScaleX = 1.0f, m_vpScaleY = 1.0f;	// AKRAVON: mantiksal (d3d) -> fiziksel piksel orani
	GLuint m_pfTex = 0, m_pfFbo = 0, m_pfProg = 0, m_pfVao = 0;
	GLint m_pfLocParam = -1, m_pfLocWarm = -1, m_pfLocSize = -1;
	UINT m_pfW = 0, m_pfH = 0;
	bool m_pfFailed = false;

public:
	float UIScale() const;

private:
	std::vector<std::pair<int, GLuint>> m_pendingDeletes;
	std::vector<uint8_t> m_scratch;
	std::vector<uint8_t> m_scratch2;
	bool m_lostLogged = false;
};

extern Device* g_device;

}	// namespace cd3d
