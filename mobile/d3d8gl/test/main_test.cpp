// Cynex: d3d8gl test uygulamasi - kutuphane yalniz d3d8 com arayuzlerinden surulur
#include <d3d8.h>
#include <d3dx8.h>
#include <android_native_app_glue.h>
#include <android/asset_manager.h>
#include <android/log.h>
#include <sys/system_properties.h>
#include <time.h>
#include <math.h>
#include <vector>
#include <string>
#include "CynexPlatform.h"

#define TLOG(...) __android_log_print(ANDROID_LOG_INFO, "D3DTest", __VA_ARGS__)

// ---------------------------------------------------------------- platform taklidi

static ANativeWindow* g_window = nullptr;
static CynexWindowCallback g_winCb = nullptr;
static const char* g_dataPath = "";

extern "C" {
ANativeWindow* CynexPlatform_GetWindow(void) { return g_window; }
int CynexPlatform_GetWidth(void) { return g_window ? ANativeWindow_getWidth(g_window) : 0; }
int CynexPlatform_GetHeight(void) { return g_window ? ANativeWindow_getHeight(g_window) : 0; }
int CynexPlatform_GetScreenWidth(void) { return CynexPlatform_GetWidth(); }
int CynexPlatform_GetScreenHeight(void) { return CynexPlatform_GetHeight(); }
void CynexPlatform_SetWindowCallback(CynexWindowCallback cb) { g_winCb = cb; }
const char* CynexPlatform_GetDataPath(void) { return g_dataPath; }
}

// ---------------------------------------------------------------- yardimcilar

static AAssetManager* g_assets = nullptr;

static std::vector<uint8_t> LoadAsset(const char* name)
{
	std::vector<uint8_t> out;
	AAsset* a = AAssetManager_open(g_assets, name, AASSET_MODE_BUFFER);
	if (!a)
	{
		TLOG("asset yok: %s", name);
		return out;
	}
	out.resize(AAsset_getLength(a));
	memcpy(out.data(), AAsset_getBuffer(a), out.size());
	AAsset_close(a);
	return out;
}

static double NowSec()
{
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec + ts.tv_nsec * 1e-9;
}

struct PDTVertex { float x, y, z; DWORD c; float u, v; };
struct TLVertex { float x, y, z, rhw; DWORD c; float u, v; };
struct PNTVertex { float x, y, z; float nx, ny, nz; float u, v; };
struct PNVertex { float x, y, z; float nx, ny, nz; };
static const DWORD FVF_PDT = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
static const DWORD FVF_TL = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
static const DWORD FVF_PNT = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;

// ---------------------------------------------------------------- test durumu

struct App
{
	IDirect3D8* d3d = nullptr;
	IDirect3DDevice8* dev = nullptr;
	UINT W = 0, H = 0;
	int frame = 0;
	double t0 = 0, fpsT = 0;
	int fpsFrames = 0;
	bool checked = false;

	IDirect3DTexture8* texUI = nullptr;		// tga
	IDirect3DTexture8* texFace = nullptr;	// tga
	IDirect3DTexture8* tex12zi = nullptr;	// dxt5 (d3dx)
	IDirect3DTexture8* texGuild = nullptr;	// dxt3 (istemci yolu)
	IDirect3DTexture8* texStone = nullptr;	// dxt1 (istemci yolu)
	IDirect3DTexture8* texStone2 = nullptr;	// dxt1 (d3dx)
	IDirect3DTexture8* texBanner = nullptr;	// png
	IDirect3DTexture8* texJpg = nullptr;	// jpg
	IDirect3DTexture8* tex4444 = nullptr;	// LoadSurfaceFromSurface
	IDirect3DTexture8* texBlock = nullptr;	// alt dikdortgen kilidi
	IDirect3DTexture8* texGlyph = nullptr;	// a4r4g4b4 cizgi deseni
	IDirect3DTexture8* texAlphaMap = nullptr;	// a8r8g8b8 splat alfa
	IDirect3DTexture8* texDisc = nullptr;	// alfa testi
	IDirect3DTexture8* rtTex = nullptr;
	IDirect3DSurface8* rtSurf = nullptr;
	IDirect3DSurface8* rtDepth = nullptr;

	IDirect3DVertexBuffer8* pdtVB = nullptr;	// dinamik, DISCARD
	IDirect3DIndexBuffer8* rectIB = nullptr;
	IDirect3DVertexBuffer8* groundVB = nullptr;
	IDirect3DIndexBuffer8* groundIB = nullptr;
	UINT groundIdx = 0;
	IDirect3DVertexBuffer8* torusVB = nullptr;
	IDirect3DIndexBuffer8* torusIB = nullptr;
	UINT torusVerts = 0, torusIdx = 0;
	ID3DXMesh* sphere = nullptr;
	DWORD pntShader = 0;
	ID3DXMatrixStack* stack = nullptr;
};
static App g;

// Cynex: istemcinin CDXTCImage + CreateDDSTexture yolu: D3DXCreateTexture + LockRect + duz memcpy
static IDirect3DTexture8* LoadDDSClientWay(const char* name)
{
	std::vector<uint8_t> f = LoadAsset(name);
	if (f.size() < 128 || memcmp(f.data(), "DDS ", 4) != 0)
		return nullptr;
	uint32_t h = *(uint32_t*)&f[12], w = *(uint32_t*)&f[16], linear = *(uint32_t*)&f[20], mips = *(uint32_t*)&f[28];
	uint32_t fourcc = *(uint32_t*)&f[84];
	D3DFORMAT fmt = fourcc == MAKEFOURCC('D', 'X', 'T', '5') ? D3DFMT_DXT5 : fourcc == MAKEFOURCC('D', 'X', 'T', '3') ? D3DFMT_DXT3 : D3DFMT_DXT1;
	if (!mips) mips = 1;
	IDirect3DTexture8* t = nullptr;
	if (FAILED(D3DXCreateTexture(g.dev, w, h, mips, 0, fmt, D3DPOOL_MANAGED, &t)))
		return nullptr;
	const uint8_t* p = f.data() + 128;
	for (uint32_t i = 0, ls = linear; i < mips; ++i, ls >>= 2)
	{
		D3DLOCKED_RECT lr;
		if (SUCCEEDED(t->LockRect(i, &lr, NULL, 0)))
		{
			uint32_t n = ls;
			if (p + n > f.data() + f.size()) n = (uint32_t)(f.data() + f.size() - p);
			memcpy(lr.pBits, p, n);
			t->UnlockRect(i);
		}
		p += ls;
	}
	return t;
}

static IDirect3DTexture8* LoadD3DX(const char* name, D3DXIMAGE_INFO* info = nullptr)
{
	std::vector<uint8_t> f = LoadAsset(name);
	if (f.empty())
		return nullptr;
	IDirect3DTexture8* t = nullptr;
	D3DXIMAGE_INFO ii;
	HRESULT hr = D3DXCreateTextureFromFileInMemoryEx(g.dev, f.data(), (UINT)f.size(), D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0,
		D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0xffff00ff, &ii, NULL, &t);
	TLOG("D3DX %s -> hr=%x %ux%u fmt=%d mips=%u", name, (unsigned)hr, ii.Width, ii.Height, (int)ii.Format, t ? t->GetLevelCount() : 0);
	if (info) *info = ii;
	return t;
}

static void CreateResources()
{
	IDirect3DDevice8* d = g.dev;

	// Cynex: istemcinin varsayilan tepe bildirimi (PNT)
	DWORD decl[] = { D3DVSD_STREAM(0), D3DVSD_REG(0, D3DVSDT_FLOAT3), D3DVSD_REG(3, D3DVSDT_FLOAT3), D3DVSD_REG(7, D3DVSDT_FLOAT2), D3DVSD_END() };
	d->CreateVertexShader(decl, NULL, &g.pntShader, 0);
	TLOG("PNT shader tutamaci 0x%x", g.pntShader);
	D3DXCreateMatrixStack(0, &g.stack);
	D3DXCreateSphere(d, 1.0f, 16, 16, &g.sphere, NULL);

	g.texUI = LoadD3DX("equipment_bg_with_ring.tga");
	// Cynex: istemcinin dusuk bellek yolu: A4R4G4B4 kopya
	if (g.texUI)
	{
		D3DSURFACE_DESC sd;
		g.texUI->GetLevelDesc(0, &sd);
		if (SUCCEEDED(D3DXCreateTexture(d, sd.Width / 2, sd.Height / 2, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &g.tex4444)))
		{
			IDirect3DSurface8 *src = nullptr, *dst = nullptr;
			g.texUI->GetSurfaceLevel(0, &src);
			g.tex4444->GetSurfaceLevel(0, &dst);
			HRESULT hr = D3DXLoadSurfaceFromSurface(dst, NULL, NULL, src, NULL, NULL, D3DX_FILTER_LINEAR, 0);
			TLOG("LoadSurfaceFromSurface hr=%x", (unsigned)hr);
			dst->Release();
			src->Release();
		}
	}
	// Cynex: BlockTexture gibi tam zincir + alt dikdortgen kilitleri
	d->CreateTexture(64, 32, 0, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &g.texBlock);
	for (int i = 0; i < 4; ++i)
	{
		RECT rc = { i * 16, 0, i * 16 + 16, 32 };
		D3DLOCKED_RECT blr;
		if (SUCCEEDED(g.texBlock->LockRect(0, &blr, &rc, 0)))
		{
			static const DWORD cols[4] = { 0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFFFF };
			for (int y = 0; y < 32; ++y)
				for (int x = 0; x < 16; ++x)
					((DWORD*)((uint8_t*)blr.pBits + y * blr.Pitch))[x] = (y < 16 || x < 8) ? cols[i] : 0;
			g.texBlock->UnlockRect(0);
		}
	}
	g.texFace = LoadD3DX("warrior_m.tga");
	g.tex12zi = LoadD3DX("12zi.dds");
	g.texStone2 = LoadD3DX("stone_01.dds");
	g.texBanner = LoadD3DX("banner.png");
	g.texJpg = LoadD3DX("click.jpg");
	g.texGuild = LoadDDSClientWay("936_guild.dds");
	g.texStone = LoadDDSClientWay("12t_stone_tile_001.dds");

	// Cynex: yazi benzeri desen - 1px beyaz/seffaf sutunlar + harf
	d->CreateTexture(64, 16, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &g.texGlyph);
	D3DLOCKED_RECT lr;
	g.texGlyph->LockRect(0, &lr, NULL, 0);
	for (int y = 0; y < 16; ++y)
	{
		WORD* row = (WORD*)((uint8_t*)lr.pBits + y * lr.Pitch);
		for (int x = 0; x < 64; ++x)
		{
			bool on;
			if (x < 16) on = (x & 1) == 0;
			else if (x < 32) on = (y & 1) == 0;
			else
			{
				// Cynex: "H" harfi
				int gx = x - 36, gy = y - 2;
				on = (gy >= 0 && gy < 12) && (gx == 0 || gx == 7 || (gy == 5 && gx > 0 && gx < 7));
			}
			row[x] = on ? 0xFFFF : 0x0000;
		}
	}
	g.texGlyph->UnlockRect(0);

	// Cynex: arazi alfa haritasi (istemci gibi 5 seviye, yalniz alfa)
	d->CreateTexture(256, 256, 5, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &g.texAlphaMap);
	for (UINT lv = 0; lv < g.texAlphaMap->GetLevelCount(); ++lv)
	{
		D3DSURFACE_DESC sd;
		g.texAlphaMap->GetLevelDesc(lv, &sd);
		g.texAlphaMap->LockRect(lv, &lr, NULL, 0);
		for (UINT y = 0; y < sd.Height; ++y)
			for (UINT x = 0; x < sd.Width; ++x)
			{
				float fx = (x + 0.5f) / sd.Width - 0.5f, fy = (y + 0.5f) / sd.Height - 0.5f;
				float r = sqrtf(fx * fx + fy * fy);
				float a = r < 0.25f ? 1.0f : (r < 0.4f ? (0.4f - r) / 0.15f : 0.0f);
				((DWORD*)((uint8_t*)lr.pBits + y * lr.Pitch))[x] = ((DWORD)(a * 255) << 24);
			}
		g.texAlphaMap->UnlockRect(lv);
	}

	// Cynex: alfa testi icin yumusak disk
	d->CreateTexture(64, 64, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &g.texDisc);
	g.texDisc->LockRect(0, &lr, NULL, 0);
	for (int y = 0; y < 64; ++y)
		for (int x = 0; x < 64; ++x)
		{
			float fx = (x + 0.5f) / 32 - 1, fy = (y + 0.5f) / 32 - 1;
			float a = 1.0f - sqrtf(fx * fx + fy * fy);
			if (a < 0) a = 0;
			((DWORD*)((uint8_t*)lr.pBits + y * lr.Pitch))[x] = ((DWORD)(a * 255) << 24) | 0x00FFC040;
		}
	g.texDisc->UnlockRect(0);

	// Cynex: ekrana cizim hedefi (CRenderTarget gibi)
	d->CreateTexture(256, 256, 0, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &g.rtTex);
	g.rtTex->GetSurfaceLevel(0, &g.rtSurf);
	d->CreateDepthStencilSurface(256, 256, D3DFMT_D16, D3DMULTISAMPLE_NONE, &g.rtDepth);

	d->CreateVertexBuffer(sizeof(PDTVertex) * 1024, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, FVF_PDT, D3DPOOL_DEFAULT, &g.pdtVB);
	d->CreateIndexBuffer(6 * 2, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &g.rectIB);
	WORD* ip;
	g.rectIB->Lock(0, 0, (BYTE**)&ip, 0);
	const WORD rect[6] = { 0, 2, 1, 2, 3, 1 };
	memcpy(ip, rect, sizeof(rect));
	g.rectIB->Unlock();

	// Cynex: zemin izgarasi (XYZ|NORMAL, arazi gibi)
	const int N = 32;
	d->CreateVertexBuffer(sizeof(PNVertex) * (N + 1) * (N + 1), D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_NORMAL, D3DPOOL_MANAGED, &g.groundVB);
	PNVertex* gv;
	g.groundVB->Lock(0, 0, (BYTE**)&gv, 0);
	for (int y = 0; y <= N; ++y)
		for (int x = 0; x <= N; ++x)
		{
			PNVertex& v = gv[y * (N + 1) + x];
			v.x = -40.0f + 80.0f * x / N;
			v.y = -40.0f + 80.0f * y / N;
			v.z = 1.5f * sinf(v.x * 0.15f) * cosf(v.y * 0.12f);
			v.nx = -0.225f * cosf(v.x * 0.15f) * cosf(v.y * 0.12f);
			v.ny = 0.18f * sinf(v.x * 0.15f) * sinf(v.y * 0.12f);
			v.nz = 1.0f;
			float l = sqrtf(v.nx * v.nx + v.ny * v.ny + 1.0f);
			v.nx /= l; v.ny /= l; v.nz /= l;
		}
	g.groundVB->Unlock();
	g.groundIdx = N * N * 6;
	d->CreateIndexBuffer(g.groundIdx * 2, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &g.groundIB);
	g.groundIB->Lock(0, 0, (BYTE**)&ip, 0);
	for (int y = 0; y < N; ++y)
		for (int x = 0; x < N; ++x)
		{
			WORD a = (WORD)(y * (N + 1) + x), b = (WORD)(a + 1), c = (WORD)(a + N + 1), e = (WORD)(c + 1);
			*ip++ = a; *ip++ = c; *ip++ = b;
			*ip++ = b; *ip++ = c; *ip++ = e;
		}
	g.groundIB->Unlock();

	// Cynex: simit (PNT)
	const int R = 32, S = 16;
	g.torusVerts = (R + 1) * (S + 1);
	d->CreateVertexBuffer(sizeof(PNTVertex) * g.torusVerts, D3DUSAGE_WRITEONLY, FVF_PNT, D3DPOOL_MANAGED, &g.torusVB);
	PNTVertex* tv;
	g.torusVB->Lock(0, 0, (BYTE**)&tv, 0);
	for (int i = 0; i <= R; ++i)
		for (int j = 0; j <= S; ++j)
		{
			float u = 2 * D3DX_PI * i / R, v = 2 * D3DX_PI * j / S;
			float cx = cosf(u), cy = sinf(u);
			PNTVertex& p = tv[i * (S + 1) + j];
			p.nx = cosf(v) * cx; p.ny = cosf(v) * cy; p.nz = sinf(v);
			p.x = (4.0f + 1.5f * cosf(v)) * cx;
			p.y = (4.0f + 1.5f * cosf(v)) * cy;
			p.z = 1.5f * sinf(v);
			p.u = 3.0f * i / R; p.v = 1.0f * j / S;
		}
	g.torusVB->Unlock();
	g.torusIdx = R * S * 6;
	d->CreateIndexBuffer(g.torusIdx * 2, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &g.torusIB);
	g.torusIB->Lock(0, 0, (BYTE**)&ip, 0);
	for (int i = 0; i < R; ++i)
		for (int j = 0; j < S; ++j)
		{
			WORD a = (WORD)(i * (S + 1) + j), b = (WORD)(a + 1), c = (WORD)(a + S + 1), e = (WORD)(c + 1);
			*ip++ = a; *ip++ = c; *ip++ = b;
			*ip++ = b; *ip++ = c; *ip++ = e;
		}
	g.torusIB->Unlock();
}

// Cynex: StateManager::SetDefaultState'in ozeti
static void DefaultStates()
{
	IDirect3DDevice8* d = g.dev;
	d->SetRenderState(D3DRS_ALPHAREF, 1);
	d->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	d->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	d->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	d->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	d->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
	d->SetRenderState(D3DRS_LIGHTING, FALSE);
	d->SetRenderState(D3DRS_COLORVERTEX, FALSE);
	d->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
	d->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
	for (int s = 0; s < 8; ++s)
	{
		d->SetTextureStageState(s, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		d->SetTextureStageState(s, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		d->SetTextureStageState(s, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
		d->SetTextureStageState(s, D3DTSS_COLORARG2, s == 0 ? D3DTA_CURRENT : D3DTA_DIFFUSE);
	}
}

static void SetOrtho2D()
{
	D3DXMATRIX proj, id;
	D3DXMatrixOrthoOffCenterRH(&proj, 0, (float)g.W, (float)g.H, 0, 0, 400.0f);
	D3DXMatrixIdentity(&id);
	g.dev->SetTransform(D3DTS_PROJECTION, &proj);
	g.dev->SetTransform(D3DTS_VIEW, &id);
	g.dev->SetTransform(D3DTS_WORLD, &id);
}

// Cynex: SetPDTStream + DEFAULT_IB_FILL_RECT yolu (oyunun ui cizimi)
static void DrawPDTQuad(float sx, float sy, float ex, float ey, float su, float sv, float eu, float ev, DWORD color)
{
	PDTVertex* v;
	if (FAILED(g.pdtVB->Lock(0, sizeof(PDTVertex) * 4, (BYTE**)&v, D3DLOCK_DISCARD)))
		return;
	v[0] = { sx - 0.5f, sy - 0.5f, 0, color, su, sv };
	v[1] = { ex - 0.5f, sy - 0.5f, 0, color, eu, sv };
	v[2] = { sx - 0.5f, ey - 0.5f, 0, color, su, ev };
	v[3] = { ex - 0.5f, ey - 0.5f, 0, color, eu, ev };
	g.pdtVB->Unlock();
	g.dev->SetStreamSource(0, g.pdtVB, sizeof(PDTVertex));
	g.dev->SetIndices(g.rectIB, 0);
	g.dev->SetVertexShader(FVF_PDT);
	g.dev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 4, 0, 2);
}

static void DrawTLQuad(float x, float y, float w, float h, DWORD color)
{
	TLVertex v[4] = {
		{ x - 0.5f, y - 0.5f, 0.0f, 1.0f, color, 0, 0 },
		{ x + w - 0.5f, y - 0.5f, 0.0f, 1.0f, color, 1, 0 },
		{ x - 0.5f, y + h - 0.5f, 0.0f, 1.0f, color, 0, 1 },
		{ x + w - 0.5f, y + h - 0.5f, 0.0f, 1.0f, color, 1, 1 },
	};
	g.dev->SetVertexShader(FVF_TL);
	g.dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(TLVertex));
}

static void TexWH(IDirect3DTexture8* t, float* w, float* h)
{
	D3DSURFACE_DESC sd;
	t->GetLevelDesc(0, &sd);
	*w = (float)sd.Width;
	*h = (float)sd.Height;
}

static void SetupLights(float time)
{
	IDirect3DDevice8* d = g.dev;
	D3DLIGHT8 l;
	memset(&l, 0, sizeof(l));
	l.Type = D3DLIGHT_DIRECTIONAL;
	l.Diffuse.r = 1.0f; l.Diffuse.g = 0.95f; l.Diffuse.b = 0.85f;
	l.Ambient.r = l.Ambient.g = l.Ambient.b = 0.25f;
	l.Specular.r = l.Specular.g = l.Specular.b = 1.0f;
	D3DXVECTOR3 dir(-0.5f, 0.6f, -0.8f);
	D3DXVec3Normalize((D3DXVECTOR3*)&l.Direction, &dir);
	d->SetLight(0, &l);
	d->LightEnable(0, TRUE);

	memset(&l, 0, sizeof(l));
	l.Type = D3DLIGHT_POINT;
	l.Diffuse.r = 1.0f; l.Diffuse.g = 0.3f; l.Diffuse.b = 0.1f;
	l.Position.x = 10.0f * cosf(time); l.Position.y = 10.0f * sinf(time); l.Position.z = 4.0f;
	l.Range = 30.0f;
	l.Attenuation0 = 0.2f; l.Attenuation1 = 0.08f;
	d->SetLight(1, &l);
	d->LightEnable(1, TRUE);
	d->SetRenderState(D3DRS_AMBIENT, 0xFF404040);
}

static void RenderScene3D(float time, bool small)
{
	IDirect3DDevice8* d = g.dev;
	D3DXMATRIX proj, view, world, tm, inv;
	float aspect = small ? 1.0f : (float)g.W / g.H;
	D3DXMatrixPerspectiveFovRH(&proj, D3DXToRadian(55.0f), aspect, 1.0f, 500.0f);
	D3DXVECTOR3 eye(28.0f * cosf(time * 0.2f), -28.0f * sinf(time * 0.2f) - 6.0f, 16.0f), at(0, 0, 2), up(0, 0, 1);
	D3DXMatrixLookAtRH(&view, &eye, &at, &up);
	D3DXMatrixIdentity(&world);
	d->SetTransform(D3DTS_PROJECTION, &proj);
	d->SetTransform(D3DTS_VIEW, &view);
	d->SetTransform(D3DTS_WORLD, &world);

	d->SetRenderState(D3DRS_ZENABLE, TRUE);
	d->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	d->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	d->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	d->SetRenderState(D3DRS_LIGHTING, TRUE);
	d->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	D3DMATERIAL8 mat;
	memset(&mat, 0, sizeof(mat));
	mat.Diffuse.r = mat.Diffuse.g = mat.Diffuse.b = mat.Diffuse.a = 1.0f;
	mat.Ambient = mat.Diffuse;
	d->SetMaterial(&mat);
	SetupLights(time);

	// Cynex: sis (istemci: vertex lineer)
	float fs = 25.0f, fe = 75.0f;
	d->SetRenderState(D3DRS_FOGENABLE, small ? FALSE : TRUE);
	d->SetRenderState(D3DRS_FOGCOLOR, 0xFF8090A8);
	d->SetRenderState(D3DRS_FOGSTART, *(DWORD*)&fs);
	d->SetRenderState(D3DRS_FOGEND, *(DWORD*)&fe);

	// Cynex: zemin - kamera uzayi konumundan doku koordinati (MapOutdoorRenderHTP gibi)
	D3DXMatrixInverse(&inv, NULL, &view);
	D3DXMatrixScaling(&tm, 1.0f / 8.0f, 1.0f / 8.0f, 1.0f);
	D3DXMatrixMultiply(&tm, &inv, &tm);
	d->SetTransform(D3DTS_TEXTURE0, &tm);
	D3DXMATRIX am, sc, tr;
	D3DXMatrixScaling(&sc, 1.0f / 80.0f, -1.0f / 80.0f, 1.0f);
	D3DXMatrixTranslation(&tr, 0.5f, 0.5f, 0.0f);
	D3DXMatrixMultiply(&am, &inv, &sc);
	D3DXMatrixMultiply(&am, &am, &tr);
	d->SetTransform(D3DTS_TEXTURE1, &am);

	d->SetTexture(0, g.texStone);
	d->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
	d->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
	d->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	d->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	d->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	d->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	d->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	d->SetVertexShader(D3DFVF_XYZ | D3DFVF_NORMAL);
	d->SetStreamSource(0, g.groundVB, sizeof(PNVertex));
	d->SetIndices(g.groundIB, 0);
	d->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 33 * 33, 0, g.groundIdx / 3);

	// Cynex: splat katmani - 2 asama, alfa haritasi + harman
	d->SetTexture(0, g.texGuild);
	d->SetTexture(1, g.texAlphaMap);
	d->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	d->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	d->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
	d->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
	d->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	d->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	d->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	d->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	d->SetTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	d->SetTextureStageState(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	d->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	d->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	d->SetRenderState(D3DRS_ALPHAREF, 0);
	d->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	d->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	d->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 33 * 33, 0, g.groundIdx / 3);
	d->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	d->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	d->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	d->SetTexture(1, NULL);
	d->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	d->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	d->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
	d->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	d->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	d->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
	d->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	// Cynex: isikli dokulu simit, speculer, bildirim tutamaci ile
	D3DXMATRIX rot, trn;
	D3DXMatrixRotationYawPitchRoll(&rot, time, time * 0.7f, 0.3f);
	D3DXMatrixTranslation(&trn, 0, 0, 6.0f);
	D3DXMatrixMultiply(&world, &rot, &trn);
	d->SetTransform(D3DTS_WORLD, &world);
	mat.Specular.r = mat.Specular.g = mat.Specular.b = 1.0f;
	mat.Power = 24.0f;
	d->SetMaterial(&mat);
	d->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
	d->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	d->SetTexture(0, g.texStone2);
	d->SetVertexShader(g.pntShader);
	d->SetStreamSource(0, g.torusVB, sizeof(PNTVertex));
	d->SetIndices(g.torusIB, 0);
	d->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, g.torusVerts, 0, g.torusIdx / 3);
	d->SetRenderState(D3DRS_SPECULARENABLE, FALSE);

	// Cynex: d3dx kuresi, tfactor rengi
	D3DXMatrixScaling(&world, 2.0f, 2.0f, 2.0f);
	D3DXMatrixTranslation(&trn, 10.0f, 6.0f, 3.0f);
	D3DXMatrixMultiply(&world, &world, &trn);
	d->SetTransform(D3DTS_WORLD, &world);
	d->SetTexture(0, NULL);
	d->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFF30A0FF);
	d->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	d->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	d->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	d->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	g.sphere->DrawSubset(0);
	d->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	d->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);

	// Cynex: alfa testi - yumusak disk, kesin kenar
	D3DXMatrixIdentity(&world);
	d->SetTransform(D3DTS_WORLD, &world);
	d->SetRenderState(D3DRS_LIGHTING, FALSE);
	d->SetTexture(0, g.texDisc);
	d->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	d->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	d->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	d->SetRenderState(D3DRS_ALPHAREF, 0x80);
	d->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	PDTVertex disc[4] = {
		{ -14, -10, 1, 0xFFFFFFFF, 0, 0 }, { -14, -10, 9, 0xFFFFFFFF, 0, 1 },
		{ -6, -10, 1, 0xFFFFFFFF, 1, 0 }, { -6, -10, 9, 0xFFFFFFFF, 1, 1 } };
	d->SetVertexShader(FVF_PDT);
	d->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, disc, sizeof(PDTVertex));
	d->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	d->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	d->SetRenderState(D3DRS_FOGENABLE, FALSE);
}

static void RenderToTexture(float time)
{
	IDirect3DDevice8* d = g.dev;
	IDirect3DSurface8 *oldRT = nullptr, *oldDS = nullptr;
	d->GetRenderTarget(&oldRT);
	d->GetDepthStencilSurface(&oldDS);
	d->SetRenderTarget(g.rtSurf, g.rtDepth);
	d->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFF402020, 1.0f, 0);
	RenderScene3D(time + 1.0f, true);
	// Cynex: yon isareti - sol ust yesil kare (RHW)
	d->SetRenderState(D3DRS_ZENABLE, FALSE);
	d->SetRenderState(D3DRS_LIGHTING, FALSE);
	d->SetTexture(0, NULL);
	d->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	d->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	DrawTLQuad(0, 0, 48, 48, 0xFF00FF00);
	d->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	d->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	d->SetRenderTarget(oldRT, oldDS);
	if (oldRT) oldRT->Release();
	if (oldDS) oldDS->Release();
}

static void RenderUI(float time)
{
	IDirect3DDevice8* d = g.dev;
	D3DVIEWPORT8 vp = { 0, 0, g.W, g.H, 0.0f, 1.0f };
	d->SetViewport(&vp);
	SetOrtho2D();
	d->SetRenderState(D3DRS_ZENABLE, FALSE);
	d->SetRenderState(D3DRS_LIGHTING, FALSE);
	d->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	d->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	d->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	d->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	// Cynex: SetBlendOperation
	d->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	d->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	d->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	d->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	d->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	d->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	d->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

	float w, h;
	if (g.texUI) { TexWH(g.texUI, &w, &h); d->SetTexture(0, g.texUI); DrawPDTQuad(20, 20, 20 + w, 20 + h, 0, 0, 1, 1, 0xFFFFFFFF); }
	if (g.texFace) { d->SetTexture(0, g.texFace); DrawTLQuad(190, 20, 43, 43, 0xFFFFFFFF); }
	if (g.texGuild) { d->SetTexture(0, g.texGuild); DrawPDTQuad(190, 80, 190 + 128, 80 + 256, 0, 0, 1, 1, 0xFFFFFFFF); }
	if (g.tex12zi) { d->SetTexture(0, g.tex12zi); DrawPDTQuad(330, 20, 330 + 240, 20 + 296, 0, 0, 1, 1, 0xFFFFFFFF); }

	if (g.tex4444) { TexWH(g.tex4444, &w, &h); d->SetTexture(0, g.tex4444); DrawPDTQuad(600, 20, 600 + w, 20 + h, 0, 0, 1, 1, 0xFFFFFFFF); }
	if (g.texBlock) { d->SetTexture(0, g.texBlock); DrawPDTQuad(600, 120, 664, 152, 0, 0, 1, 1, 0xFFFFFFFF); }

	// Cynex: render-target dokusu (GrpRenderTargetTexture::Render gibi)
	float rx = (float)g.W - 290.0f, ry = 20.0f;
	d->SetTexture(0, g.rtTex);
	d->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
	DrawPDTQuad(rx, ry, rx + 256, ry + 256, 0, 0, 1, 1, 0xFFFFFFFF);
	d->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	// Cynex: 1px desen - RHW ve ortho, ikisi de birebir olmali
	d->SetTexture(0, g.texGlyph);
	d->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	d->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	DrawTLQuad(20, 400, 64, 16, 0xFFFFFFFF);
	DrawPDTQuad(100, 400, 164, 416, 0, 0, 1, 1, 0xFFFFFF00);

	// Cynex: NULL doku + MODULATE -> diffuse rengi (istemci RenderBar)
	d->SetTexture(0, NULL);
	DrawPDTQuad(20, 430, 120, 470, 0, 0, 1, 1, 0x80FF0000);
	DrawPDTQuad(60, 450, 160, 490, 0, 0, 1, 1, 0x8000FF00);

	// Cynex: cizgi listesi ve ucgen yelpazesi
	PDTVertex lines[8] = {
		{ rx - 2, ry - 2, 0, 0xFFFFFF00, 0, 0 }, { rx + 258, ry - 2, 0, 0xFFFFFF00, 0, 0 },
		{ rx + 258, ry - 2, 0, 0xFFFFFF00, 0, 0 }, { rx + 258, ry + 258, 0, 0xFFFFFF00, 0, 0 },
		{ rx + 258, ry + 258, 0, 0xFFFFFF00, 0, 0 }, { rx - 2, ry + 258, 0, 0xFFFFFF00, 0, 0 },
		{ rx - 2, ry + 258, 0, 0xFFFFFF00, 0, 0 }, { rx - 2, ry - 2, 0, 0xFFFFFF00, 0, 0 } };
	d->SetVertexShader(FVF_PDT);
	d->DrawPrimitiveUP(D3DPT_LINELIST, 4, lines, sizeof(PDTVertex));
	PDTVertex fan[18];
	float cx = 230, cy = 450;
	fan[0] = { cx, cy, 0, 0xFFFFFFFF, 0, 0 };
	for (int i = 0; i <= 16; ++i)
	{
		float a = time + 2 * D3DX_PI * i / 16;
		fan[i + 1] = { cx + 30 * cosf(a), cy + 30 * sinf(a), 0, (i & 1) ? 0xFF0080FF : 0xFFFF8000, 0, 0 };
	}
	d->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 16, fan, sizeof(PDTVertex));
	// Cynex: DrawIndexedPrimitiveUP (eski ui yolu)
	PDTVertex tri[4] = { { 280, 420, 0, 0xFFFF00FF, 0, 0 }, { 340, 420, 0, 0xFF00FFFF, 0, 0 }, { 280, 480, 0, 0xFFFFFF00, 0, 0 }, { 340, 480, 0, 0xFFFFFFFF, 0, 0 } };
	const WORD tidx[6] = { 0, 2, 1, 2, 3, 1 };
	d->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, 4, 2, tidx, D3DFMT_INDEX16, tri, sizeof(PDTVertex));

	// Cynex: cok asamali ui: doku * tfactor ADD + MODULATE2X
	if (g.texBanner)
	{
		TexWH(g.texBanner, &w, &h);
		float s = 360.0f / w;
		d->SetTexture(0, g.texBanner);
		DrawPDTQuad(20, 510, 20 + w * s, 510 + h * s, 0, 0, 1, 1, 0xFFFFFFFF);
		d->SetRenderState(D3DRS_TEXTUREFACTOR, 0xFF404040);
		d->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
		d->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);
		DrawPDTQuad(400, 510, 400 + w * s, 510 + h * s, 0, 0, 1, 1, 0xFFFFFFFF);
		d->SetTexture(1, g.texDisc);
		d->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0);
		d->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		d->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		d->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
		d->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		DrawPDTQuad(780, 510, 780 + 160, 510 + 160, 0, 0, 1, 1, 0xFFFFFFFF);
		d->SetTexture(1, NULL);
		d->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		d->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
		d->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	}

	// Cynex: matris yigini kullanimi
	g.stack->LoadIdentity();
	g.stack->Push();
	g.stack->Translate(1, 2, 3);
	g.stack->Pop();
}

// Cynex: geri okuma ile piksel denetimi (ekran goruntusu yolu)
static void CheckPixels()
{
	IDirect3DSurface8* bb = nullptr;
	if (FAILED(g.dev->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &bb)))
		return;
	D3DSURFACE_DESC sd;
	bb->GetDesc(&sd);
	D3DLOCKED_RECT lr;
	if (FAILED(bb->LockRect(&lr, NULL, D3DLOCK_READONLY)))
	{
		TLOG("CHECK backbuffer LockRect basarisiz");
		bb->Release();
		return;
	}
	auto px = [&](int x, int y) -> DWORD { return ((DWORD*)((uint8_t*)lr.pBits + y * lr.Pitch))[x]; };
	auto lum = [&](DWORD c) { int r = (c >> 16) & 255, g2 = (c >> 8) & 255, b = c & 255; int m = r > g2 ? r : g2; return m > b ? m : b; };

	int fails = 0;
	// Cynex: 1px sutunlar: RHW (20,400) ve ortho (100,400)
	for (int k = 0; k < 2; ++k)
	{
		int bx = k ? 100 : 20;
		int bad = 0;
		const DWORD oncol = k ? 0xFFFFFF00 : 0xFFFFFFFF;
		// Cynex: acik pikseller birebir renk, kapalilar arka plan (hic karisim yok)
		for (int x = 0; x < 16; ++x)
		{
			bool on = (x & 1) == 0;
			if (on ? px(bx + x, 405) != oncol : px(bx + x, 405) == oncol) ++bad;
		}
		for (int y = 0; y < 16; ++y)
		{
			bool on = (y & 1) == 0;
			if (on ? px(bx + 20, 400 + y) != oncol : px(bx + 20, 400 + y) == oncol) ++bad;
		}
		// Cynex: karisim olsaydi kapali pikseller acik rengin yarisina yaklasirdi
		for (int x = 1; x < 16; x += 2)
		{
			DWORD c = px(bx + x, 405);
			if (((c >> 16) & 255) > 200 && ((c >> 8) & 255) > 200) ++bad;
		}
		TLOG("CHECK 1px desen %s: %s (%d hata) ornek %08x %08x", k ? "ortho" : "rhw", bad ? "FAIL" : "PASS", bad, px(bx, 405), px(bx + 1, 405));
		fails += bad != 0;
	}
	// Cynex: render-target yonu: sol ust yesil
	int rx = (int)g.W - 290, ry = 20;
	DWORD tl = px(rx + 10, ry + 10), bl = px(rx + 10, ry + 256 - 10);
	bool rtok = ((tl >> 8) & 255) > 200 && ((tl >> 16) & 255) < 60 && !(((bl >> 8) & 255) > 200 && ((bl >> 16) & 255) < 60);
	TLOG("CHECK RT yonu: %s (ust %08x alt %08x)", rtok ? "PASS" : "FAIL", tl, bl);
	fails += !rtok;
	// Cynex: NULL doku = beyaz: kirmizi bar
	DWORD bar = px(30, 440);
	bool barok = ((bar >> 16) & 255) > 120 && ((bar >> 8) & 255) < 100;
	TLOG("CHECK NULL doku diffuse: %s (%08x)", barok ? "PASS" : "FAIL", bar);
	fails += !barok;
	// Cynex: kenar: tga ui sol ust pikseli sinirda tam
	TLOG("CHECK ozet: %s (%d)", fails ? "FAIL" : "PASS", fails);
	bb->UnlockRect();
	bb->Release();
}

static void InitDevice(android_app* app)
{
	g.d3d = Direct3DCreate8(D3D_SDK_VERSION);
	D3DDISPLAYMODE dm;
	g.d3d->GetAdapterDisplayMode(0, &dm);
	TLOG("ekran %ux%u modlar=%u", dm.Width, dm.Height, g.d3d->GetAdapterModeCount(0));
	D3DCAPS8 caps;
	g.d3d->GetDeviceCaps(0, D3DDEVTYPE_HAL, &caps);

	D3DPRESENT_PARAMETERS pp;
	memset(&pp, 0, sizeof(pp));
	pp.Windowed = TRUE;
	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pp.BackBufferFormat = dm.Format;
	pp.EnableAutoDepthStencil = TRUE;
	pp.AutoDepthStencilFormat = D3DFMT_D24S8;
	pp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	// Cynex: setprop debug.cynex.d3dtest.bb 1280x720 -> ekran disi arka tampon yolu
	char prop[PROP_VALUE_MAX] = { 0 };
	if (__system_property_get("debug.cynex.d3dtest.bb", prop) > 0)
		sscanf(prop, "%ux%u", &pp.BackBufferWidth, &pp.BackBufferHeight);
	HRESULT hr = g.d3d->CreateDevice(0, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &g.dev);
	TLOG("CreateDevice hr=%x bb=%ux%u", (unsigned)hr, pp.BackBufferWidth, pp.BackBufferHeight);
	if (FAILED(hr))
		return;
	g.W = pp.BackBufferWidth;
	g.H = pp.BackBufferHeight;
	g.dev->GetDeviceCaps(&caps);
	TLOG("caps: devcaps=%x maxtex=%u aniso=%u vs=%x", caps.DevCaps, caps.MaxTextureWidth, caps.MaxAnisotropy, caps.VertexShaderVersion);
	CreateResources();
	DefaultStates();
	g.t0 = g.fpsT = NowSec();
}

static void RenderFrame()
{
	if (!g.dev || g.dev->TestCooperativeLevel() != D3D_OK)
		return;
	float time = (float)(NowSec() - g.t0);
	char prop[PROP_VALUE_MAX] = { 0 };
	if (__system_property_get("debug.cynex.d3dtest.freeze", prop) > 0 && prop[0] == '1')
		time = 1.0f;

	g.dev->BeginScene();
	RenderToTexture(time);
	D3DVIEWPORT8 vp = { 0, 0, g.W, g.H, 0.0f, 1.0f };
	g.dev->SetViewport(&vp);
	g.dev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFF203040, 1.0f, 0);
	// Cynex: 3d sahne alt yarida bir viewport'ta
	D3DVIEWPORT8 vp3 = { 0, 0, g.W, g.H, 0.0f, 1.0f };
	g.dev->SetViewport(&vp3);
	RenderScene3D(time, false);
	RenderUI(time);
	g.dev->EndScene();

	++g.frame;
	if (g.frame == 45 && !g.checked)
	{
		CheckPixels();
		g.checked = true;
	}
	g.dev->Present(NULL, NULL, NULL, NULL);

	++g.fpsFrames;
	double now = NowSec();
	if (now - g.fpsT > 5.0)
	{
		TLOG("fps %.1f", g.fpsFrames / (now - g.fpsT));
		g.fpsT = now;
		g.fpsFrames = 0;
	}
}

static void OnCmd(android_app* app, int32_t cmd)
{
	switch (cmd)
	{
	case APP_CMD_INIT_WINDOW:
		g_window = app->window;
		if (!g.dev)
			InitDevice(app);
		else if (g_winCb)
			g_winCb(app->window, 1);
		break;
	case APP_CMD_TERM_WINDOW:
		if (g_winCb)
			g_winCb(app->window, 0);
		g_window = nullptr;
		break;
	default:
		break;
	}
}

void android_main(android_app* app)
{
	app->onAppCmd = OnCmd;
	g_assets = app->activity->assetManager;
	g_dataPath = app->activity->internalDataPath ? app->activity->internalDataPath : "";
	while (!app->destroyRequested)
	{
		int events;
		android_poll_source* src;
		while (ALooper_pollOnce(g_window ? 0 : -1, nullptr, &events, (void**)&src) >= 0)
		{
			if (src)
				src->process(app, src);
			if (app->destroyRequested)
				break;
		}
		if (g_window)
			RenderFrame();
	}
	if (g.dev)
	{
		g.dev->Release();
		g.dev = nullptr;
	}
}
