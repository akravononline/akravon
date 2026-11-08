// Cynex: d3d piksel formatlari, donusumler, dxt cozucu
#include "cd3d_common.h"

namespace cd3d {

static const FormatInfo s_formats[] = {
	{ D3DFMT_A8R8G8B8, 4, 0, true, false },
	{ D3DFMT_X8R8G8B8, 4, 0, false, false },
	{ D3DFMT_R8G8B8, 3, 0, false, false },
	{ D3DFMT_R5G6B5, 2, 0, false, false },
	{ D3DFMT_X1R5G5B5, 2, 0, false, false },
	{ D3DFMT_A1R5G5B5, 2, 0, true, false },
	{ D3DFMT_A4R4G4B4, 2, 0, true, false },
	{ D3DFMT_X4R4G4B4, 2, 0, false, false },
	{ D3DFMT_R3G3B2, 1, 0, false, false },
	{ D3DFMT_A8R3G3B2, 2, 0, true, false },
	{ D3DFMT_A8, 1, 0, true, false },
	{ D3DFMT_L8, 1, 0, false, false },
	{ D3DFMT_A8L8, 2, 0, true, false },
	{ D3DFMT_A4L4, 1, 0, true, false },
	{ D3DFMT_A2B10G10R10, 4, 0, true, false },
	{ D3DFMT_G16R16, 4, 0, false, false },
	{ D3DFMT_P8, 1, 0, false, false },
	{ D3DFMT_DXT1, 0, 8, true, false },
	{ D3DFMT_DXT2, 0, 16, true, false },
	{ D3DFMT_DXT3, 0, 16, true, false },
	{ D3DFMT_DXT4, 0, 16, true, false },
	{ D3DFMT_DXT5, 0, 16, true, false },
	{ D3DFMT_D16, 2, 0, false, true },
	{ D3DFMT_D16_LOCKABLE, 2, 0, false, true },
	{ D3DFMT_D15S1, 2, 0, false, true },
	{ D3DFMT_D24S8, 4, 0, false, true },
	{ D3DFMT_D24X8, 4, 0, false, true },
	{ D3DFMT_D24X4S4, 4, 0, false, true },
	{ D3DFMT_D32, 4, 0, false, true },
};

const FormatInfo* GetFormatInfo(D3DFORMAT fmt)
{
	for (const FormatInfo& f : s_formats)
		if (f.fmt == fmt)
			return &f;
	return nullptr;
}

bool IsCompressed(D3DFORMAT fmt)
{
	const FormatInfo* f = GetFormatInfo(fmt);
	return f && f->blockBytes;
}

UINT SurfacePitch(D3DFORMAT fmt, UINT w)
{
	const FormatInfo* f = GetFormatInfo(fmt);
	if (!f)
		return w * 4;
	if (f->blockBytes)
		return ((w + 3) / 4) * f->blockBytes;
	return w * f->bpp;
}

UINT SurfaceRows(D3DFORMAT fmt, UINT h)
{
	return IsCompressed(fmt) ? (h + 3) / 4 : h;
}

UINT SurfaceSize(D3DFORMAT fmt, UINT w, UINT h)
{
	return SurfacePitch(fmt, w) * SurfaceRows(fmt, h);
}

bool GetGLUploadFmt(D3DFORMAT fmt, bool rt, GLUploadFmt* o)
{
	const GLint id[4] = { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
	memcpy(o->swizzle, id, sizeof(id));
	o->convert = CONV_NONE;
	o->compressed = false;
	switch (fmt)
	{
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
		o->internalFmt = GL_RGBA8; o->format = GL_RGBA; o->type = GL_UNSIGNED_BYTE; o->dstBpp = 4;
		if (!rt)
		{
			// Cynex: bellekte b,g,r,a - kopyasiz swizzle
			o->swizzle[0] = GL_BLUE; o->swizzle[2] = GL_RED;
		}
		if (fmt == D3DFMT_X8R8G8B8)
			o->swizzle[3] = GL_ONE;
		return true;
	case D3DFMT_R8G8B8:
		o->internalFmt = GL_RGBA8; o->format = GL_RGBA; o->type = GL_UNSIGNED_BYTE; o->dstBpp = 4;
		o->swizzle[0] = GL_BLUE; o->swizzle[2] = GL_RED; o->swizzle[3] = GL_ONE;
		o->convert = CONV_RGB24;
		return true;
	case D3DFMT_R5G6B5:
		o->internalFmt = GL_RGB565; o->format = GL_RGB; o->type = GL_UNSIGNED_SHORT_5_6_5; o->dstBpp = 2;
		return true;
	case D3DFMT_A1R5G5B5:
	case D3DFMT_X1R5G5B5:
		o->internalFmt = GL_RGB5_A1; o->format = GL_RGBA; o->type = GL_UNSIGNED_SHORT_5_5_5_1; o->dstBpp = 2;
		o->convert = fmt == D3DFMT_A1R5G5B5 ? CONV_1555 : CONV_X1555;
		return true;
	case D3DFMT_A4R4G4B4:
	case D3DFMT_X4R4G4B4:
		// Cynex: argb4444 bitleri gl'de r=a g=r b=g a=b okunur, swizzle ile cevir
		o->internalFmt = GL_RGBA4; o->format = GL_RGBA; o->type = GL_UNSIGNED_SHORT_4_4_4_4; o->dstBpp = 2;
		o->swizzle[0] = GL_GREEN; o->swizzle[1] = GL_BLUE; o->swizzle[2] = GL_ALPHA;
		o->swizzle[3] = fmt == D3DFMT_A4R4G4B4 ? GL_RED : GL_ONE;
		return true;
	case D3DFMT_A8:
		o->internalFmt = GL_R8; o->format = GL_RED; o->type = GL_UNSIGNED_BYTE; o->dstBpp = 1;
		o->swizzle[0] = o->swizzle[1] = o->swizzle[2] = GL_ZERO; o->swizzle[3] = GL_RED;
		return true;
	case D3DFMT_L8:
		o->internalFmt = GL_R8; o->format = GL_RED; o->type = GL_UNSIGNED_BYTE; o->dstBpp = 1;
		o->swizzle[0] = o->swizzle[1] = o->swizzle[2] = GL_RED; o->swizzle[3] = GL_ONE;
		return true;
	case D3DFMT_A8L8:
		o->internalFmt = GL_RG8; o->format = GL_RG; o->type = GL_UNSIGNED_BYTE; o->dstBpp = 2;
		o->swizzle[0] = o->swizzle[1] = o->swizzle[2] = GL_RED; o->swizzle[3] = GL_GREEN;
		return true;
	case D3DFMT_DXT1:
	case D3DFMT_DXT2:
	case D3DFMT_DXT3:
	case D3DFMT_DXT4:
	case D3DFMT_DXT5:
		if (g_glcaps.s3tc && !g_glcaps.forceCpuDxt)
		{
			o->compressed = true;
			o->internalFmt = fmt == D3DFMT_DXT1 ? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT :
				(fmt == D3DFMT_DXT2 || fmt == D3DFMT_DXT3) ? GL_COMPRESSED_RGBA_S3TC_DXT3_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			o->format = GL_RGBA; o->type = GL_UNSIGNED_BYTE; o->dstBpp = 0;
			return true;
		}
		o->internalFmt = GL_RGBA8; o->format = GL_RGBA; o->type = GL_UNSIGNED_BYTE; o->dstBpp = 4;
		o->convert = CONV_DXT_RGBA8;
		return true;
	default:
		return false;
	}
}

// ---------------------------------------------------------------- dxt

static inline void Expand565(uint16_t c, uint8_t* o)
{
	uint32_t r = (c >> 11) & 31, g = (c >> 5) & 63, b = c & 31;
	o[0] = (uint8_t)((r << 3) | (r >> 2));
	o[1] = (uint8_t)((g << 2) | (g >> 4));
	o[2] = (uint8_t)((b << 3) | (b >> 2));
	o[3] = 255;
}

static void DecodeColorBlock(const uint8_t* b, uint8_t out[64], bool dxt1)
{
	uint16_t c0 = (uint16_t)(b[0] | (b[1] << 8));
	uint16_t c1 = (uint16_t)(b[2] | (b[3] << 8));
	uint32_t idx = (uint32_t)b[4] | ((uint32_t)b[5] << 8) | ((uint32_t)b[6] << 16) | ((uint32_t)b[7] << 24);
	uint8_t pal[4][4];
	Expand565(c0, pal[0]);
	Expand565(c1, pal[1]);
	if (!dxt1 || c0 > c1)
	{
		for (int k = 0; k < 3; ++k)
		{
			pal[2][k] = (uint8_t)((2 * pal[0][k] + pal[1][k] + 1) / 3);
			pal[3][k] = (uint8_t)((pal[0][k] + 2 * pal[1][k] + 1) / 3);
		}
		pal[2][3] = pal[3][3] = 255;
	}
	else
	{
		for (int k = 0; k < 3; ++k)
			pal[2][k] = (uint8_t)((pal[0][k] + pal[1][k]) / 2);
		pal[2][3] = 255;
		pal[3][0] = pal[3][1] = pal[3][2] = pal[3][3] = 0;
	}
	for (int i = 0; i < 16; ++i)
		memcpy(out + i * 4, pal[(idx >> (2 * i)) & 3], 4);
}

void DecodeDXTBlock(D3DFORMAT fmt, const uint8_t* b, uint8_t out[64])
{
	if (fmt == D3DFMT_DXT1)
	{
		DecodeColorBlock(b, out, true);
		return;
	}
	DecodeColorBlock(b + 8, out, false);
	if (fmt == D3DFMT_DXT2 || fmt == D3DFMT_DXT3)
	{
		for (int i = 0; i < 16; ++i)
		{
			uint8_t a = (b[i >> 1] >> ((i & 1) * 4)) & 15;
			out[i * 4 + 3] = (uint8_t)(a * 17);
		}
	}
	else
	{
		uint32_t a0 = b[0], a1 = b[1];
		uint8_t pal[8];
		pal[0] = (uint8_t)a0; pal[1] = (uint8_t)a1;
		if (a0 > a1)
		{
			for (int k = 1; k < 7; ++k)
				pal[k + 1] = (uint8_t)(((7 - k) * a0 + k * a1 + 3) / 7);
		}
		else
		{
			for (int k = 1; k < 5; ++k)
				pal[k + 1] = (uint8_t)(((5 - k) * a0 + k * a1 + 2) / 5);
			pal[6] = 0; pal[7] = 255;
		}
		uint64_t bits = 0;
		for (int k = 0; k < 6; ++k)
			bits |= (uint64_t)b[2 + k] << (8 * k);
		for (int i = 0; i < 16; ++i)
			out[i * 4 + 3] = pal[(bits >> (3 * i)) & 7];
	}
}

bool DXT1HasTransparency(const uint8_t* data, UINT w, UINT h)
{
	UINT bw = (w + 3) / 4, bh = (h + 3) / 4;
	for (UINT i = 0; i < bw * bh; ++i)
	{
		const uint8_t* b = data + i * 8;
		uint16_t c0 = (uint16_t)(b[0] | (b[1] << 8));
		uint16_t c1 = (uint16_t)(b[2] | (b[3] << 8));
		if (c0 > c1)
			continue;
		uint32_t idx = (uint32_t)b[4] | ((uint32_t)b[5] << 8) | ((uint32_t)b[6] << 16) | ((uint32_t)b[7] << 24);
		for (int k = 0; k < 16; ++k)
			if (((idx >> (2 * k)) & 3) == 3)
				return true;
	}
	return false;
}

// Cynex: outKind CONV_DXT_RGBA8 -> 4 bayt rgba, CONV_DXT1_565 / CONV_DXT1_5551 -> 2 bayt
void DecodeDXTLevel(D3DFORMAT fmt, const uint8_t* src, UINT w, UINT h, uint8_t* dst, int outKind)
{
	const FormatInfo* fi = GetFormatInfo(fmt);
	UINT bw = (w + 3) / 4, bh = (h + 3) / 4;
	uint8_t px[64];
	const UINT dbpp = outKind == CONV_DXT_RGBA8 ? 4 : 2;
	for (UINT by = 0; by < bh; ++by)
	{
		for (UINT bx = 0; bx < bw; ++bx)
		{
			DecodeDXTBlock(fmt, src + (by * bw + bx) * fi->blockBytes, px);
			for (UINT y = 0; y < 4; ++y)
			{
				UINT yy = by * 4 + y;
				if (yy >= h)
					break;
				for (UINT x = 0; x < 4; ++x)
				{
					UINT xx = bx * 4 + x;
					if (xx >= w)
						break;
					const uint8_t* p = px + (y * 4 + x) * 4;
					uint8_t* d = dst + (yy * w + xx) * dbpp;
					if (outKind == CONV_DXT_RGBA8)
						memcpy(d, p, 4);
					else if (outKind == CONV_DXT_4444)
					{
						uint16_t v = (uint16_t)(((p[0] >> 4) << 12) | ((p[1] >> 4) << 8) | ((p[2] >> 4) << 4) | (p[3] >> 4));
						memcpy(d, &v, 2);
					}
					else if (outKind == CONV_DXT1_565)
					{
						uint16_t v = (uint16_t)(((p[0] >> 3) << 11) | ((p[1] >> 2) << 5) | (p[2] >> 3));
						memcpy(d, &v, 2);
					}
					else
					{
						uint16_t v = (uint16_t)(((p[0] >> 3) << 11) | ((p[1] >> 3) << 6) | ((p[2] >> 3) << 1) | (p[3] >= 128 ? 1 : 0));
						memcpy(d, &v, 2);
					}
				}
			}
		}
	}
}

// ---------------------------------------------------------------- bgra8 decode/encode

static inline uint8_t Ex5(uint32_t v) { return (uint8_t)((v << 3) | (v >> 2)); }
static inline uint8_t Ex6(uint32_t v) { return (uint8_t)((v << 2) | (v >> 4)); }
static inline uint8_t Ex4(uint32_t v) { return (uint8_t)(v * 17); }

void DecodeToBGRA8(D3DFORMAT fmt, const uint8_t* src, UINT srcPitch, UINT w, UINT h, uint8_t* dst, UINT dstPitch)
{
	if (IsCompressed(fmt))
	{
		const FormatInfo* fi = GetFormatInfo(fmt);
		UINT bw = (w + 3) / 4, bh = (h + 3) / 4;
		uint8_t px[64];
		for (UINT by = 0; by < bh; ++by)
			for (UINT bx = 0; bx < bw; ++bx)
			{
				DecodeDXTBlock(fmt, src + by * srcPitch + bx * fi->blockBytes, px);
				for (UINT y = 0; y < 4 && by * 4 + y < h; ++y)
					for (UINT x = 0; x < 4 && bx * 4 + x < w; ++x)
					{
						const uint8_t* p = px + (y * 4 + x) * 4;
						uint8_t* d = dst + (by * 4 + y) * dstPitch + (bx * 4 + x) * 4;
						d[0] = p[2]; d[1] = p[1]; d[2] = p[0]; d[3] = p[3];
					}
			}
		return;
	}

	for (UINT y = 0; y < h; ++y)
	{
		const uint8_t* s = src + y * srcPitch;
		uint8_t* d = dst + y * dstPitch;
		for (UINT x = 0; x < w; ++x, d += 4)
		{
			switch (fmt)
			{
			case D3DFMT_A8R8G8B8: memcpy(d, s + x * 4, 4); break;
			case D3DFMT_X8R8G8B8: memcpy(d, s + x * 4, 3); d[3] = 255; break;
			case D3DFMT_R8G8B8: memcpy(d, s + x * 3, 3); d[3] = 255; break;
			case D3DFMT_R5G6B5:
			{
				uint16_t v; memcpy(&v, s + x * 2, 2);
				d[2] = Ex5((v >> 11) & 31); d[1] = Ex6((v >> 5) & 63); d[0] = Ex5(v & 31); d[3] = 255;
				break;
			}
			case D3DFMT_X1R5G5B5:
			case D3DFMT_A1R5G5B5:
			{
				uint16_t v; memcpy(&v, s + x * 2, 2);
				d[2] = Ex5((v >> 10) & 31); d[1] = Ex5((v >> 5) & 31); d[0] = Ex5(v & 31);
				d[3] = (fmt == D3DFMT_X1R5G5B5 || (v & 0x8000)) ? 255 : 0;
				break;
			}
			case D3DFMT_A4R4G4B4:
			case D3DFMT_X4R4G4B4:
			{
				uint16_t v; memcpy(&v, s + x * 2, 2);
				d[2] = Ex4((v >> 8) & 15); d[1] = Ex4((v >> 4) & 15); d[0] = Ex4(v & 15);
				d[3] = fmt == D3DFMT_X4R4G4B4 ? 255 : Ex4(v >> 12);
				break;
			}
			case D3DFMT_A8: d[0] = d[1] = d[2] = 0; d[3] = s[x]; break;
			case D3DFMT_L8: d[0] = d[1] = d[2] = s[x]; d[3] = 255; break;
			case D3DFMT_A8L8: d[0] = d[1] = d[2] = s[x * 2]; d[3] = s[x * 2 + 1]; break;
			case D3DFMT_A4L4: d[0] = d[1] = d[2] = Ex4(s[x] & 15); d[3] = Ex4(s[x] >> 4); break;
			case D3DFMT_R3G3B2:
			{
				uint8_t v = s[x];
				d[2] = (uint8_t)(((v >> 5) & 7) * 255 / 7); d[1] = (uint8_t)(((v >> 2) & 7) * 255 / 7); d[0] = (uint8_t)((v & 3) * 85); d[3] = 255;
				break;
			}
			default: d[0] = d[1] = d[2] = d[3] = 0; break;
			}
		}
	}
}

bool EncodeFromBGRA8(D3DFORMAT fmt, const uint8_t* src, UINT srcPitch, UINT w, UINT h, uint8_t* dst, UINT dstPitch)
{
	switch (fmt)
	{
	case D3DFMT_A8R8G8B8: case D3DFMT_X8R8G8B8: case D3DFMT_R8G8B8: case D3DFMT_R5G6B5:
	case D3DFMT_X1R5G5B5: case D3DFMT_A1R5G5B5: case D3DFMT_A4R4G4B4: case D3DFMT_X4R4G4B4:
	case D3DFMT_A8: case D3DFMT_L8: case D3DFMT_A8L8:
		break;
	default:
		return false;
	}
	for (UINT y = 0; y < h; ++y)
	{
		const uint8_t* s = src + y * srcPitch;
		uint8_t* d = dst + y * dstPitch;
		for (UINT x = 0; x < w; ++x, s += 4)
		{
			const uint32_t b = s[0], g = s[1], r = s[2], a = s[3];
			switch (fmt)
			{
			case D3DFMT_A8R8G8B8: memcpy(d + x * 4, s, 4); break;
			case D3DFMT_X8R8G8B8: memcpy(d + x * 4, s, 3); d[x * 4 + 3] = 255; break;
			case D3DFMT_R8G8B8: memcpy(d + x * 3, s, 3); break;
			case D3DFMT_R5G6B5: { uint16_t v = (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)); memcpy(d + x * 2, &v, 2); break; }
			case D3DFMT_X1R5G5B5:
			case D3DFMT_A1R5G5B5:
			{
				uint16_t v = (uint16_t)(((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3));
				if (fmt == D3DFMT_X1R5G5B5 || a >= 128) v |= 0x8000;
				memcpy(d + x * 2, &v, 2);
				break;
			}
			case D3DFMT_A4R4G4B4:
			case D3DFMT_X4R4G4B4:
			{
				uint16_t v = (uint16_t)(((r >> 4) << 8) | ((g >> 4) << 4) | (b >> 4));
				v |= (uint16_t)((fmt == D3DFMT_X4R4G4B4 ? 15 : (a >> 4)) << 12);
				memcpy(d + x * 2, &v, 2);
				break;
			}
			case D3DFMT_A8: d[x] = (uint8_t)a; break;
			case D3DFMT_L8: d[x] = (uint8_t)((r * 77 + g * 151 + b * 28) >> 8); break;
			case D3DFMT_A8L8: d[x * 2] = (uint8_t)((r * 77 + g * 151 + b * 28) >> 8); d[x * 2 + 1] = (uint8_t)a; break;
			default: break;
			}
		}
	}
	return true;
}

}	// namespace cd3d
