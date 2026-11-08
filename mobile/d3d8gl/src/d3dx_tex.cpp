// Cynex: d3dx8 doku yukleme (dds/tga/bmp/jpg/png) ve yuzey kopyalama
#include "cd3d_device.h"
#include "stb_image.h"

using namespace cd3d;

namespace {

UINT FullMips(UINT w, UINT h)
{
	UINT n = 1, m = w > h ? w : h;
	while (m > 1) { m >>= 1; ++n; }
	return n;
}

bool IsDefault(UINT v) { return v == 0 || v == 0xFFFFFFFFu; }	// Cynex: D3DX_DEFAULT=ULONG_MAX, LP64de UINT kesilir

// Cynex: bgra8 olcekleme - kucultmede kutu, buyutmede ikili dogrusal
void ScaleBGRA8(const uint8_t* src, UINT sw, UINT sh, UINT spitch, uint8_t* dst, UINT dw, UINT dh, UINT dpitch, bool point)
{
	if (sw == dw && sh == dh)
	{
		for (UINT y = 0; y < dh; ++y)
			memcpy(dst + y * dpitch, src + y * spitch, dw * 4);
		return;
	}
	for (UINT y = 0; y < dh; ++y)
	{
		uint8_t* d = dst + y * dpitch;
		for (UINT x = 0; x < dw; ++x, d += 4)
		{
			if (point)
			{
				UINT sx = (UINT)(((uint64_t)x * sw) / dw), sy = (UINT)(((uint64_t)y * sh) / dh);
				memcpy(d, src + sy * spitch + sx * 4, 4);
				continue;
			}
			if (sw >= dw && sh >= dh)
			{
				UINT x0 = (UINT)(((uint64_t)x * sw) / dw), x1 = (UINT)(((uint64_t)(x + 1) * sw) / dw);
				UINT y0 = (UINT)(((uint64_t)y * sh) / dh), y1 = (UINT)(((uint64_t)(y + 1) * sh) / dh);
				if (x1 <= x0) x1 = x0 + 1;
				if (y1 <= y0) y1 = y0 + 1;
				uint32_t acc[4] = { 0, 0, 0, 0 }, n = 0;
				for (UINT yy = y0; yy < y1; ++yy)
					for (UINT xx = x0; xx < x1; ++xx, ++n)
					{
						const uint8_t* s = src + yy * spitch + xx * 4;
						acc[0] += s[0]; acc[1] += s[1]; acc[2] += s[2]; acc[3] += s[3];
					}
				for (int k = 0; k < 4; ++k) d[k] = (uint8_t)((acc[k] + n / 2) / n);
			}
			else
			{
				float fx = ((x + 0.5f) * sw) / dw - 0.5f, fy = ((y + 0.5f) * sh) / dh - 0.5f;
				if (fx < 0) fx = 0;
				if (fy < 0) fy = 0;
				UINT ix = (UINT)fx, iy = (UINT)fy;
				UINT ix1 = ix + 1 < sw ? ix + 1 : sw - 1, iy1 = iy + 1 < sh ? iy + 1 : sh - 1;
				float ax = fx - ix, ay = fy - iy;
				const uint8_t* p00 = src + iy * spitch + ix * 4;
				const uint8_t* p10 = src + iy * spitch + ix1 * 4;
				const uint8_t* p01 = src + iy1 * spitch + ix * 4;
				const uint8_t* p11 = src + iy1 * spitch + ix1 * 4;
				for (int k = 0; k < 4; ++k)
				{
					float v = (p00[k] * (1 - ax) + p10[k] * ax) * (1 - ay) + (p01[k] * (1 - ax) + p11[k] * ax) * ay;
					d[k] = (uint8_t)(v + 0.5f);
				}
			}
		}
	}
}

void ApplyColorKey(uint8_t* bgra, UINT w, UINT h, UINT pitch, D3DCOLOR key)
{
	if (!key)
		return;
	for (UINT y = 0; y < h; ++y)
	{
		uint32_t* p = (uint32_t*)(bgra + y * pitch);
		for (UINT x = 0; x < w; ++x)
			if (p[x] == key)
				p[x] = 0;
	}
}

// ---------------------------------------------------------------- dds

#pragma pack(push, 1)
struct DDSPixelFormat { uint32_t size, flags, fourCC, bits, rmask, gmask, bmask, amask; };
struct DDSHeader
{
	uint32_t size, flags, height, width, pitchOrLinear, depth, mipCount;
	uint32_t reserved1[11];
	DDSPixelFormat pf;
	uint32_t caps, caps2, caps3, caps4, reserved2;
};
#pragma pack(pop)

#define CD3D_FOURCC(a, b, c, d) ((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

D3DFORMAT DDSFormat(const DDSPixelFormat& pf)
{
	if (pf.flags & 0x4)	// DDPF_FOURCC
	{
		switch (pf.fourCC)
		{
		case CD3D_FOURCC('D', 'X', 'T', '1'): return D3DFMT_DXT1;
		case CD3D_FOURCC('D', 'X', 'T', '2'): return D3DFMT_DXT2;
		case CD3D_FOURCC('D', 'X', 'T', '3'): return D3DFMT_DXT3;
		case CD3D_FOURCC('D', 'X', 'T', '4'): return D3DFMT_DXT4;
		case CD3D_FOURCC('D', 'X', 'T', '5'): return D3DFMT_DXT5;
		default: return D3DFMT_UNKNOWN;
		}
	}
	const bool alpha = (pf.flags & 0x1) != 0;
	if (pf.bits == 32 && pf.rmask == 0xFF0000 && pf.gmask == 0xFF00 && pf.bmask == 0xFF)
		return (alpha && pf.amask) ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8;
	if (pf.bits == 24 && pf.rmask == 0xFF0000) return D3DFMT_R8G8B8;
	if (pf.bits == 16 && pf.rmask == 0xF800) return D3DFMT_R5G6B5;
	if (pf.bits == 16 && pf.rmask == 0x7C00) return (alpha && pf.amask) ? D3DFMT_A1R5G5B5 : D3DFMT_X1R5G5B5;
	if (pf.bits == 16 && pf.rmask == 0x0F00) return (alpha && pf.amask) ? D3DFMT_A4R4G4B4 : D3DFMT_X4R4G4B4;
	if (pf.bits == 8 && (pf.flags & 0x20000)) return D3DFMT_L8;			// DDPF_LUMINANCE
	if (pf.bits == 16 && (pf.flags & 0x20000)) return D3DFMT_A8L8;
	if (pf.bits == 8 && alpha && !pf.rmask) return D3DFMT_A8;
	return D3DFMT_UNKNOWN;
}

HRESULT LoadDDS(Device* dev, const uint8_t* data, UINT size, UINT reqW, UINT reqH, UINT reqMips, DWORD usage, D3DFORMAT reqFmt,
	D3DPOOL pool, DWORD filter, D3DCOLOR key, D3DXIMAGE_INFO* info, IDirect3DTexture8** ppTex)
{
	if (size < 4 + sizeof(DDSHeader))
		return D3DXERR_INVALIDDATA;
	DDSHeader h;
	memcpy(&h, data + 4, sizeof(h));
	D3DFORMAT ff = DDSFormat(h.pf);
	if (ff == D3DFMT_UNKNOWN || !h.width || !h.height)
		return D3DXERR_INVALIDDATA;
	UINT fileMips = (h.flags & 0x20000) && h.mipCount ? h.mipCount : 1;
	UINT full = FullMips(h.width, h.height);
	if (fileMips > full) fileMips = full;

	if (info)
	{
		info->Width = h.width;
		info->Height = h.height;
		info->Depth = 1;
		info->MipLevels = fileMips;
		info->Format = ff;
		info->ResourceType = D3DRTYPE_TEXTURE;
		info->ImageFileFormat = D3DXIFF_DDS;
	}

	const uint8_t* p = data + 4 + sizeof(DDSHeader);
	const uint8_t* end = data + size;

	if (IsCompressed(ff))
	{
		// Cynex: dxt oldugu gibi kopyalanir
		UINT mips = IsDefault(reqMips) ? fileMips : (reqMips < fileMips ? reqMips : fileMips);
		IDirect3DTexture8* t = nullptr;
		HRESULT hr = dev->CreateTexture(h.width, h.height, mips, usage, ff, pool, &t);
		if (FAILED(hr))
			return hr;
		Texture* tex = static_cast<Texture*>(t);
		for (UINT i = 0; i < mips; ++i)
		{
			UINT lsz = tex->Lv(i).size;
			if (p + lsz > end)
				break;
			memcpy(tex->LevelData(i, true), p, lsz);
			tex->MarkLevelDirty(i, nullptr);
			p += lsz;
		}
		tex->SetFromFile(false);
		*ppTex = t;
		return D3D_OK;
	}

	// Cynex: sikistirilmamis dds - bgra8 uzerinden hedef formata
	const FormatInfo* fi = GetFormatInfo(ff);
	UINT w = IsDefault(reqW) ? h.width : reqW;
	UINT hh = IsDefault(reqH) ? h.height : reqH;
	D3DFORMAT fmt = reqFmt;
	if (fmt == D3DFMT_UNKNOWN)
	{
		fmt = ff == D3DFMT_R8G8B8 ? D3DFMT_X8R8G8B8 : ff;
		if (key && !fi->hasAlpha) fmt = D3DFMT_A8R8G8B8;
	}
	UINT mips = IsDefault(reqMips) ? FullMips(w, hh) : reqMips;
	IDirect3DTexture8* t = nullptr;
	HRESULT hr = dev->CreateTexture(w, hh, mips, usage, fmt, pool, &t);
	if (FAILED(hr))
		return hr;
	Texture* tex = static_cast<Texture*>(t);
	const bool sameSize = w == h.width && hh == h.height;
	UINT filled = 0;
	std::vector<uint8_t> bgra, scaled;
	for (UINT i = 0; i < (sameSize ? fileMips : 1) && i < tex->Levels(); ++i)
	{
		UINT lw = h.width >> i, lh = h.height >> i;
		if (!lw) lw = 1;
		if (!lh) lh = 1;
		UINT spitch = lw * fi->bpp;
		if (p + spitch * lh > end)
			break;
		bgra.resize((size_t)lw * lh * 4);
		DecodeToBGRA8(ff, p, spitch, lw, lh, bgra.data(), lw * 4);
		p += spitch * lh;
		ApplyColorKey(bgra.data(), lw, lh, lw * 4, key);
		const Texture::Level& L = tex->Lv(i);
		const uint8_t* src = bgra.data();
		if (L.w != lw || L.h != lh)
		{
			scaled.resize((size_t)L.w * L.h * 4);
			ScaleBGRA8(bgra.data(), lw, lh, lw * 4, scaled.data(), L.w, L.h, L.w * 4, (filter & 0xFF) == D3DX_FILTER_POINT);
			src = scaled.data();
		}
		if (!EncodeFromBGRA8(fmt, src, L.w * 4, L.w, L.h, tex->LevelData(i, true), L.pitch))
		{
			t->Release();
			return D3DERR_INVALIDCALL;
		}
		tex->MarkLevelDirty(i, nullptr);
		++filled;
	}
	tex->SetFromFile(filled < tex->Levels());
	*ppTex = t;
	return D3D_OK;
}

}	// namespace

// ---------------------------------------------------------------- disa acik

HRESULT WINAPI D3DXCreateTexture(LPDIRECT3DDEVICE8 pDevice, UINT w, UINT h, UINT mips, DWORD usage, D3DFORMAT fmt, D3DPOOL pool, LPDIRECT3DTEXTURE8* ppTex)
{
	if (!pDevice || !ppTex)
		return D3DERR_INVALIDCALL;
	if (IsDefault(w)) w = 256;
	if (IsDefault(h)) h = 256;
	if (fmt == D3DFMT_UNKNOWN) fmt = D3DFMT_A8R8G8B8;
	UINT full = FullMips(w, h);
	if (IsDefault(mips) || mips > full) mips = full;
	HRESULT hr = pDevice->CreateTexture(w, h, mips, usage, fmt, pool, ppTex);
	if (FAILED(hr) && fmt != D3DFMT_A8R8G8B8 && !IsCompressed(fmt))
		hr = pDevice->CreateTexture(w, h, mips, usage, D3DFMT_A8R8G8B8, pool, ppTex);
	return hr;
}

HRESULT WINAPI D3DXCreateTextureFromFileInMemoryEx(LPDIRECT3DDEVICE8 pDevice, LPCVOID pSrcData, UINT srcSize,
	UINT reqW, UINT reqH, UINT reqMips, DWORD usage, D3DFORMAT reqFmt, D3DPOOL pool, DWORD filter, DWORD mipFilter,
	D3DCOLOR colorKey, D3DXIMAGE_INFO* pSrcInfo, PALETTEENTRY* pPalette, LPDIRECT3DTEXTURE8* ppTexture)
{
	(void)mipFilter;
	(void)pPalette;
	if (!pDevice || !pSrcData || !srcSize || !ppTexture)
		return D3DERR_INVALIDCALL;
	*ppTexture = NULL;
	Device* dev = static_cast<Device*>(pDevice);
	const uint8_t* data = (const uint8_t*)pSrcData;

	if (srcSize >= 4 && memcmp(data, "DDS ", 4) == 0)
		return LoadDDS(dev, data, srcSize, reqW, reqH, reqMips, usage, reqFmt, pool, filter, colorKey, pSrcInfo, ppTexture);

	D3DXIMAGE_FILEFORMAT iff = D3DXIFF_TGA;
	if (srcSize >= 2 && data[0] == 'B' && data[1] == 'M') iff = D3DXIFF_BMP;
	else if (srcSize >= 3 && data[0] == 0xFF && data[1] == 0xD8) iff = D3DXIFF_JPG;
	else if (srcSize >= 8 && data[0] == 0x89 && data[1] == 'P' && data[2] == 'N' && data[3] == 'G') iff = D3DXIFF_PNG;

	int w = 0, h = 0, comp = 0;
	stbi_uc* rgba = stbi_load_from_memory(data, (int)srcSize, &w, &h, &comp, 4);
	if (!rgba)
	{
		CD3D_LOGW("D3DX doku cozulemedi: %s", stbi_failure_reason());
		return D3DXERR_INVALIDDATA;
	}

	if (pSrcInfo)
	{
		pSrcInfo->Width = w;
		pSrcInfo->Height = h;
		pSrcInfo->Depth = 1;
		pSrcInfo->MipLevels = 1;
		pSrcInfo->Format = comp == 4 ? D3DFMT_A8R8G8B8 : comp == 3 ? D3DFMT_R8G8B8 : comp == 2 ? D3DFMT_A8L8 : D3DFMT_L8;
		pSrcInfo->ResourceType = D3DRTYPE_TEXTURE;
		pSrcInfo->ImageFileFormat = iff;
	}

	// Cynex: rgba -> bgra (d3d A8R8G8B8 bellek duzeni)
	const size_t npx = (size_t)w * h;
	for (size_t i = 0; i < npx; ++i)
	{
		uint8_t t = rgba[i * 4];
		rgba[i * 4] = rgba[i * 4 + 2];
		rgba[i * 4 + 2] = t;
	}
	ApplyColorKey(rgba, w, h, w * 4, colorKey);

	UINT tw = IsDefault(reqW) ? (UINT)w : reqW;
	UINT th = IsDefault(reqH) ? (UINT)h : reqH;
	D3DFORMAT fmt = reqFmt;
	if (fmt == D3DFMT_UNKNOWN)
		fmt = (comp == 4 || comp == 2 || (colorKey >> 24)) ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8;
	UINT mips = IsDefault(reqMips) ? FullMips(tw, th) : reqMips;

	IDirect3DTexture8* t = nullptr;
	HRESULT hr = dev->CreateTexture(tw, th, mips, usage, fmt, pool, &t);
	if (FAILED(hr) || !t)
	{
		stbi_image_free(rgba);
		return FAILED(hr) ? hr : D3DERR_INVALIDCALL;
	}
	Texture* tex = static_cast<Texture*>(t);
	const Texture::Level& L = tex->Lv(0);
	const uint8_t* src = rgba;
	std::vector<uint8_t> scaled;
	if (L.w != (UINT)w || L.h != (UINT)h)
	{
		scaled.resize((size_t)L.w * L.h * 4);
		ScaleBGRA8(rgba, w, h, w * 4, scaled.data(), L.w, L.h, L.w * 4, (filter & 0xFF) == D3DX_FILTER_POINT);
		src = scaled.data();
	}
	bool ok = EncodeFromBGRA8(fmt, src, L.w * 4, L.w, L.h, tex->LevelData(0, true), L.pitch);
	stbi_image_free(rgba);
	if (!ok)
	{
		t->Release();
		return D3DERR_INVALIDCALL;
	}
	tex->MarkLevelDirty(0, nullptr);
	// Cynex: alt seviyeler gpu'da glGenerateMipmap ile
	tex->SetFromFile(tex->Levels() > 1);
	*ppTexture = t;
	return D3D_OK;
}

HRESULT WINAPI D3DXCreateTextureFromFileInMemory(LPDIRECT3DDEVICE8 pDevice, LPCVOID pSrcData, UINT srcSize, LPDIRECT3DTEXTURE8* ppTexture)
{
	return D3DXCreateTextureFromFileInMemoryEx(pDevice, pSrcData, srcSize, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0,
		D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_FILTER_BOX, 0, NULL, NULL, ppTexture);
}

HRESULT WINAPI D3DXLoadSurfaceFromSurface(LPDIRECT3DSURFACE8 pDest, CONST PALETTEENTRY*, CONST RECT* pDestRect,
	LPDIRECT3DSURFACE8 pSrc, CONST PALETTEENTRY*, CONST RECT* pSrcRect, DWORD filter, D3DCOLOR colorKey)
{
	Surface* dst = static_cast<Surface*>(pDest);
	Surface* src = static_cast<Surface*>(pSrc);
	if (!dst || !src)
		return D3DERR_INVALIDCALL;

	UINT spitch = 0, dpitch = 0;
	const uint8_t* sdata = src->CpuData(&spitch);
	if (src->m_kind == SURF_TEXLEVEL && src->m_container->Lv(src->m_level).data == nullptr)
		sdata = nullptr;
	if (!sdata)
	{
		CD3D_LOGW("D3DXLoadSurfaceFromSurface: kaynak cpu verisi yok");
		return D3DERR_INVALIDCALL;
	}
	uint8_t* ddata = dst->CpuData(&dpitch);
	if (!ddata)
		return D3DERR_INVALIDCALL;

	RECT sr = pSrcRect ? *pSrcRect : RECT{ 0, 0, (LONG)src->m_w, (LONG)src->m_h };
	RECT dr = pDestRect ? *pDestRect : RECT{ 0, 0, (LONG)dst->m_w, (LONG)dst->m_h };
	const UINT sw = sr.right - sr.left, sh = sr.bottom - sr.top;
	const UINT dw = dr.right - dr.left, dh = dr.bottom - dr.top;
	if (!sw || !sh || !dw || !dh)
		return D3DERR_INVALIDCALL;

	// Cynex: kaynagin tamamini coz (dxt bloklari icin), sonra pencereyi al
	std::vector<uint8_t> full((size_t)src->m_w * src->m_h * 4), win((size_t)sw * sh * 4), out((size_t)dw * dh * 4);
	DecodeToBGRA8(src->m_fmt, sdata, spitch, src->m_w, src->m_h, full.data(), src->m_w * 4);
	for (UINT y = 0; y < sh; ++y)
		memcpy(win.data() + (size_t)y * sw * 4, full.data() + ((size_t)(sr.top + y) * src->m_w + sr.left) * 4, sw * 4);
	ApplyColorKey(win.data(), sw, sh, sw * 4, colorKey);
	const DWORD f = filter & 0xFF;
	ScaleBGRA8(win.data(), sw, sh, sw * 4, out.data(), dw, dh, dw * 4, f == D3DX_FILTER_POINT || f == D3DX_FILTER_NONE);

	if (IsCompressed(dst->m_fmt))
		return D3DERR_INVALIDCALL;
	const FormatInfo* fi = GetFormatInfo(dst->m_fmt);
	if (!fi || !EncodeFromBGRA8(dst->m_fmt, out.data(), dw * 4, dw, dh, ddata + dr.top * dpitch + dr.left * fi->bpp, dpitch))
		return D3DERR_INVALIDCALL;
	if (dst->m_kind == SURF_TEXLEVEL)
		dst->m_container->MarkLevelDirty(dst->m_level, &dr);
	return D3D_OK;
}
