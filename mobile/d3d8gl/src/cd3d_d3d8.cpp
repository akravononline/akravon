// Cynex: IDirect3D8 - adaptor, kip listesi, yetenekler
#include "cd3d_device.h"
#include "CynexPlatform.h"

namespace cd3d {

void FillCaps(D3DCAPS8* c)
{
	memset(c, 0, sizeof(*c));
	c->DeviceType = D3DDEVTYPE_HAL;
	c->AdapterOrdinal = 0;
	c->Caps = 0;
	c->Caps2 = D3DCAPS2_CANRENDERWINDOWED | D3DCAPS2_DYNAMICTEXTURES | D3DCAPS2_FULLSCREENGAMMA;
	c->Caps3 = D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD;
	c->PresentationIntervals = D3DPRESENT_INTERVAL_IMMEDIATE | D3DPRESENT_INTERVAL_ONE;
	c->CursorCaps = 0;
	c->DevCaps = D3DDEVCAPS_EXECUTESYSTEMMEMORY | D3DDEVCAPS_EXECUTEVIDEOMEMORY | D3DDEVCAPS_TLVERTEXSYSTEMMEMORY |
		D3DDEVCAPS_TLVERTEXVIDEOMEMORY | D3DDEVCAPS_TEXTUREVIDEOMEMORY | D3DDEVCAPS_DRAWPRIMTLVERTEX |
		D3DDEVCAPS_CANRENDERAFTERFLIP | D3DDEVCAPS_DRAWPRIMITIVES2 | D3DDEVCAPS_DRAWPRIMITIVES2EX |
		D3DDEVCAPS_HWTRANSFORMANDLIGHT | D3DDEVCAPS_HWRASTERIZATION;
	c->PrimitiveMiscCaps = D3DPMISCCAPS_MASKZ | D3DPMISCCAPS_CULLNONE | D3DPMISCCAPS_CULLCW | D3DPMISCCAPS_CULLCCW |
		D3DPMISCCAPS_COLORWRITEENABLE | D3DPMISCCAPS_CLIPTLVERTS | D3DPMISCCAPS_TSSARGTEMP | D3DPMISCCAPS_BLENDOP;
	c->RasterCaps = D3DPRASTERCAPS_DITHER | D3DPRASTERCAPS_ZTEST | D3DPRASTERCAPS_FOGVERTEX | D3DPRASTERCAPS_FOGTABLE |
		D3DPRASTERCAPS_ZBIAS | D3DPRASTERCAPS_FOGRANGE | D3DPRASTERCAPS_WFOG | D3DPRASTERCAPS_ZFOG;
	if (g_glcaps.aniso)
		c->RasterCaps |= D3DPRASTERCAPS_ANISOTROPY;
	const DWORD cmp = D3DPCMPCAPS_NEVER | D3DPCMPCAPS_LESS | D3DPCMPCAPS_EQUAL | D3DPCMPCAPS_LESSEQUAL |
		D3DPCMPCAPS_GREATER | D3DPCMPCAPS_NOTEQUAL | D3DPCMPCAPS_GREATEREQUAL | D3DPCMPCAPS_ALWAYS;
	c->ZCmpCaps = cmp;
	c->AlphaCmpCaps = cmp;
	const DWORD blend = D3DPBLENDCAPS_ZERO | D3DPBLENDCAPS_ONE | D3DPBLENDCAPS_SRCCOLOR | D3DPBLENDCAPS_INVSRCCOLOR |
		D3DPBLENDCAPS_SRCALPHA | D3DPBLENDCAPS_INVSRCALPHA | D3DPBLENDCAPS_DESTALPHA | D3DPBLENDCAPS_INVDESTALPHA |
		D3DPBLENDCAPS_DESTCOLOR | D3DPBLENDCAPS_INVDESTCOLOR | D3DPBLENDCAPS_SRCALPHASAT | D3DPBLENDCAPS_BOTHSRCALPHA |
		D3DPBLENDCAPS_BOTHINVSRCALPHA;
	c->SrcBlendCaps = blend;
	c->DestBlendCaps = blend;
	c->ShadeCaps = D3DPSHADECAPS_COLORGOURAUDRGB | D3DPSHADECAPS_SPECULARGOURAUDRGB | D3DPSHADECAPS_ALPHAGOURAUDBLEND | D3DPSHADECAPS_FOGGOURAUD;
	c->TextureCaps = D3DPTEXTURECAPS_PERSPECTIVE | D3DPTEXTURECAPS_ALPHA | D3DPTEXTURECAPS_PROJECTED | D3DPTEXTURECAPS_MIPMAP;
	DWORD filt = D3DPTFILTERCAPS_MINFPOINT | D3DPTFILTERCAPS_MINFLINEAR | D3DPTFILTERCAPS_MIPFPOINT | D3DPTFILTERCAPS_MIPFLINEAR |
		D3DPTFILTERCAPS_MAGFPOINT | D3DPTFILTERCAPS_MAGFLINEAR;
	if (g_glcaps.aniso)
		filt |= D3DPTFILTERCAPS_MINFANISOTROPIC | D3DPTFILTERCAPS_MAGFANISOTROPIC;
	c->TextureFilterCaps = filt;
	c->CubeTextureFilterCaps = 0;
	c->VolumeTextureFilterCaps = 0;
	// Cynex: border golgelendiricide taklit ediliyor
	c->TextureAddressCaps = D3DPTADDRESSCAPS_WRAP | D3DPTADDRESSCAPS_MIRROR | D3DPTADDRESSCAPS_CLAMP | D3DPTADDRESSCAPS_BORDER | D3DPTADDRESSCAPS_INDEPENDENTUV;
	c->VolumeTextureAddressCaps = 0;
	c->LineCaps = D3DLINECAPS_TEXTURE | D3DLINECAPS_ZTEST | D3DLINECAPS_BLEND | D3DLINECAPS_ALPHACMP | D3DLINECAPS_FOG;
	c->MaxTextureWidth = c->MaxTextureHeight = g_glcaps.maxTexSize;
	c->MaxVolumeExtent = 0;
	c->MaxTextureRepeat = 8192;
	c->MaxTextureAspectRatio = g_glcaps.maxTexSize;
	c->MaxAnisotropy = g_glcaps.aniso ? (DWORD)g_glcaps.maxAniso : 1;
	c->MaxVertexW = 1e10f;
	c->GuardBandLeft = c->GuardBandTop = -8192.0f;
	c->GuardBandRight = c->GuardBandBottom = 8192.0f;
	c->ExtentsAdjust = 0.0f;
	c->StencilCaps = D3DSTENCILCAPS_KEEP | D3DSTENCILCAPS_ZERO | D3DSTENCILCAPS_REPLACE | D3DSTENCILCAPS_INCRSAT |
		D3DSTENCILCAPS_DECRSAT | D3DSTENCILCAPS_INVERT | D3DSTENCILCAPS_INCR | D3DSTENCILCAPS_DECR;
	c->FVFCaps = 8 | D3DFVFCAPS_DONOTSTRIPELEMENTS;
	c->TextureOpCaps = D3DTEXOPCAPS_DISABLE | D3DTEXOPCAPS_SELECTARG1 | D3DTEXOPCAPS_SELECTARG2 | D3DTEXOPCAPS_MODULATE |
		D3DTEXOPCAPS_MODULATE2X | D3DTEXOPCAPS_MODULATE4X | D3DTEXOPCAPS_ADD | D3DTEXOPCAPS_ADDSIGNED |
		D3DTEXOPCAPS_ADDSIGNED2X | D3DTEXOPCAPS_SUBTRACT | D3DTEXOPCAPS_ADDSMOOTH | D3DTEXOPCAPS_BLENDDIFFUSEALPHA |
		D3DTEXOPCAPS_BLENDTEXTUREALPHA | D3DTEXOPCAPS_BLENDFACTORALPHA | D3DTEXOPCAPS_BLENDTEXTUREALPHAPM |
		D3DTEXOPCAPS_BLENDCURRENTALPHA | D3DTEXOPCAPS_PREMODULATE | D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR |
		D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA | D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR | D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA |
		D3DTEXOPCAPS_DOTPRODUCT3 | D3DTEXOPCAPS_MULTIPLYADD | D3DTEXOPCAPS_LERP;
	c->MaxTextureBlendStages = 8;
	c->MaxSimultaneousTextures = g_glcaps.maxTexUnits;
	c->VertexProcessingCaps = D3DVTXPCAPS_TEXGEN | D3DVTXPCAPS_MATERIALSOURCE7 | D3DVTXPCAPS_DIRECTIONALLIGHTS |
		D3DVTXPCAPS_POSITIONALLIGHTS | D3DVTXPCAPS_LOCALVIEWER;
	c->MaxActiveLights = 8;
	c->MaxUserClipPlanes = 0;
	c->MaxVertexBlendMatrices = 0;
	c->MaxVertexBlendMatrixIndex = 0;
	c->MaxPointSize = 1.0f;
	c->MaxPrimitiveCount = 0xFFFFFF;
	c->MaxVertexIndex = 0xFFFFFF;
	c->MaxStreams = 16;
	c->MaxStreamStride = 255;
	c->VertexShaderVersion = D3DVS_VERSION(0, 0);
	c->MaxVertexShaderConst = 0;
	c->PixelShaderVersion = D3DPS_VERSION(0, 0);
	c->MaxPixelShaderValue = 1.0f;
}

namespace {

struct Mode { UINT w, h; D3DFORMAT f; };

void ScreenSize(UINT* w, UINT* h)
{
	int sw = CynexPlatform_GetScreenWidth(), sh = CynexPlatform_GetScreenHeight();
	if (sw <= 0 || sh <= 0)
	{
		sw = Egl_HasWindow() ? Egl_SurfaceWidth() : CynexPlatform_GetWidth();
		sh = Egl_HasWindow() ? Egl_SurfaceHeight() : CynexPlatform_GetHeight();
	}
	*w = sw > 0 ? (UINT)sw : 1280;
	*h = sh > 0 ? (UINT)sh : 720;
}

// Cynex: oyunun cizim boyutu + ekran + standart kipler, x8r8g8b8 ve r5g6b5 icin
void ModeList(std::vector<Mode>& out)
{
	out.clear();
	std::vector<std::pair<UINT, UINT>> sizes;
	auto addSize = [&](UINT w, UINT h) {
		if (!w || !h) return;
		for (auto& s : sizes) if (s.first == w && s.second == h) return;
		sizes.push_back(std::make_pair(w, h));
	};
	int gw = CynexPlatform_GetWidth(), gh = CynexPlatform_GetHeight();
	if (gw > 0 && gh > 0) addSize((UINT)gw, (UINT)gh);
	UINT sw, sh;
	ScreenSize(&sw, &sh);
	addSize(sw, sh);
	static const UINT stdm[][2] = { { 800, 600 }, { 1024, 768 }, { 1280, 720 }, { 1280, 800 }, { 1366, 768 }, { 1600, 900 }, { 1920, 1080 } };
	for (auto& m : stdm) addSize(m[0], m[1]);
	for (auto& s : sizes) out.push_back({ s.first, s.second, D3DFMT_X8R8G8B8 });
	for (auto& s : sizes) out.push_back({ s.first, s.second, D3DFMT_R5G6B5 });
}

bool IsTexFormatOK(D3DFORMAT f)
{
	switch (f)
	{
	case D3DFMT_A8R8G8B8: case D3DFMT_X8R8G8B8: case D3DFMT_R5G6B5: case D3DFMT_A1R5G5B5:
	case D3DFMT_X1R5G5B5: case D3DFMT_A4R4G4B4: case D3DFMT_X4R4G4B4: case D3DFMT_A8:
	case D3DFMT_L8: case D3DFMT_A8L8: case D3DFMT_R8G8B8:
	case D3DFMT_DXT1: case D3DFMT_DXT2: case D3DFMT_DXT3: case D3DFMT_DXT4: case D3DFMT_DXT5:
		return true;
	default:
		return false;
	}
}

bool IsDepthOK(D3DFORMAT f)
{
	return f == D3DFMT_D16 || f == D3DFMT_D24S8 || f == D3DFMT_D24X8 || f == D3DFMT_D16_LOCKABLE;
}

bool IsDisplayOK(D3DFORMAT f)
{
	return f == D3DFMT_X8R8G8B8 || f == D3DFMT_A8R8G8B8 || f == D3DFMT_R5G6B5;
}

}	// namespace

class Direct3D8 final : public IDirect3D8
{
public:
	CD3D_IUNKNOWN_IMPL(IDirect3D8)
	ULONG STDMETHODCALLTYPE AddRef() override { return ++m_ref; }
	ULONG STDMETHODCALLTYPE Release() override
	{
		ULONG r = --m_ref;
		if (r == 0) delete this;
		return r;
	}

	HRESULT STDMETHODCALLTYPE RegisterSoftwareDevice(void*) override { return D3DERR_NOTAVAILABLE; }
	UINT STDMETHODCALLTYPE GetAdapterCount() override { return 1; }

	HRESULT STDMETHODCALLTYPE GetAdapterIdentifier(UINT a, DWORD, D3DADAPTER_IDENTIFIER8* id) override
	{
		if (a != 0 || !id) return D3DERR_INVALIDCALL;
		memset(id, 0, sizeof(*id));
		strncpy(id->Driver, "cynex_gles3", sizeof(id->Driver) - 1);
		snprintf(id->Description, sizeof(id->Description), "%s (GLES3)", g_glcaps.renderer);
		id->DriverVersion.QuadPart = ((uint64_t)0x00060000 << 32) | 0x00010001;
		id->VendorId = 0x5143;
		id->DeviceId = 0x0001;
		id->WHQLLevel = 1;
		return D3D_OK;
	}

	UINT STDMETHODCALLTYPE GetAdapterModeCount(UINT a) override
	{
		if (a != 0) return 0;
		std::vector<Mode> m;
		ModeList(m);
		return (UINT)m.size();
	}

	HRESULT STDMETHODCALLTYPE EnumAdapterModes(UINT a, UINT i, D3DDISPLAYMODE* pm) override
	{
		if (a != 0 || !pm) return D3DERR_INVALIDCALL;
		std::vector<Mode> m;
		ModeList(m);
		if (i >= m.size()) return D3DERR_INVALIDCALL;
		pm->Width = m[i].w;
		pm->Height = m[i].h;
		pm->RefreshRate = 60;
		pm->Format = m[i].f;
		return D3D_OK;
	}

	HRESULT STDMETHODCALLTYPE GetAdapterDisplayMode(UINT a, D3DDISPLAYMODE* pm) override
	{
		if (a != 0 || !pm) return D3DERR_INVALIDCALL;
		ScreenSize(&pm->Width, &pm->Height);
		pm->RefreshRate = 60;
		pm->Format = D3DFMT_X8R8G8B8;
		return D3D_OK;
	}

	HRESULT STDMETHODCALLTYPE CheckDeviceType(UINT a, D3DDEVTYPE t, D3DFORMAT disp, D3DFORMAT bb, BOOL) override
	{
		if (a != 0 || t != D3DDEVTYPE_HAL) return D3DERR_NOTAVAILABLE;
		if (!IsDisplayOK(disp) || (bb != D3DFMT_UNKNOWN && !IsDisplayOK(bb))) return D3DERR_NOTAVAILABLE;
		return D3D_OK;
	}

	HRESULT STDMETHODCALLTYPE CheckDeviceFormat(UINT a, D3DDEVTYPE t, D3DFORMAT, DWORD usage, D3DRESOURCETYPE rt, D3DFORMAT f) override
	{
		if (a != 0 || t != D3DDEVTYPE_HAL) return D3DERR_NOTAVAILABLE;
		if (usage & D3DUSAGE_DEPTHSTENCIL)
			return IsDepthOK(f) ? D3D_OK : D3DERR_NOTAVAILABLE;
		if (usage & D3DUSAGE_RENDERTARGET)
			return (f == D3DFMT_A8R8G8B8 || f == D3DFMT_X8R8G8B8 || f == D3DFMT_R5G6B5) ? D3D_OK : D3DERR_NOTAVAILABLE;
		if (rt == D3DRTYPE_CUBETEXTURE || rt == D3DRTYPE_VOLUMETEXTURE || rt == D3DRTYPE_VOLUME)
			return D3DERR_NOTAVAILABLE;
		if (rt == D3DRTYPE_VERTEXBUFFER || rt == D3DRTYPE_INDEXBUFFER)
			return D3D_OK;
		return IsTexFormatOK(f) ? D3D_OK : D3DERR_NOTAVAILABLE;
	}

	HRESULT STDMETHODCALLTYPE CheckDeviceMultiSampleType(UINT a, D3DDEVTYPE, D3DFORMAT, BOOL, D3DMULTISAMPLE_TYPE ms) override
	{
		if (a != 0) return D3DERR_INVALIDCALL;
		return ms == D3DMULTISAMPLE_NONE ? D3D_OK : D3DERR_NOTAVAILABLE;
	}

	HRESULT STDMETHODCALLTYPE CheckDepthStencilMatch(UINT a, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, D3DFORMAT ds) override
	{
		if (a != 0) return D3DERR_INVALIDCALL;
		return IsDepthOK(ds) ? D3D_OK : D3DERR_NOTAVAILABLE;
	}

	HRESULT STDMETHODCALLTYPE GetDeviceCaps(UINT a, D3DDEVTYPE t, D3DCAPS8* c) override
	{
		if (a != 0 || !c) return D3DERR_INVALIDCALL;
		FillCaps(c);
		if (t != D3DDEVTYPE_HAL)
		{
			// Cynex: REF cihazi sunma - detector yalniz HAL'i secsin
			c->DeviceType = t;
			c->DevCaps = 0;
		}
		return D3D_OK;
	}

	HMONITOR STDMETHODCALLTYPE GetAdapterMonitor(UINT) override { return (HMONITOR)1; }

	HRESULT STDMETHODCALLTYPE CreateDevice(UINT a, D3DDEVTYPE t, HWND hwnd, DWORD behavior, D3DPRESENT_PARAMETERS* pp, IDirect3DDevice8** ppDev) override
	{
		if (!ppDev || !pp || a != 0 || t != D3DDEVTYPE_HAL)
			return D3DERR_INVALIDCALL;
		*ppDev = NULL;
		if (g_device)
		{
			CD3D_LOGE("CreateDevice: zaten bir cihaz var");
			return D3DERR_NOTAVAILABLE;
		}
		if (!Egl_Init())
			return D3DERR_NOTAVAILABLE;
		Egl_SetRenderThread();
		ANativeWindow* w = CynexPlatform_GetWindow();
		if (w)
			Egl_AttachWindow(w);

		Device* d = new Device(this, hwnd, behavior, pp);
		if (!d->Init())
		{
			d->Release();
			return D3DERR_NOTAVAILABLE;
		}
		*pp = d->PP();
		*ppDev = d;
		return D3D_OK;
	}

private:
	std::atomic<ULONG> m_ref{ 1 };
};

}	// namespace cd3d

IDirect3D8* WINAPI Direct3DCreate8(UINT sdk)
{
	(void)sdk;
	return new cd3d::Direct3D8();
}
