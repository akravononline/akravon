// Cynex: dds basligi icin ddraw yapilari - DDSURFACEDESC2 dosyadaki gibi 124 bayt kalmali
#pragma once
#include <windows.h>

typedef struct _DDCOLORKEY { DWORD dwColorSpaceLowValue; DWORD dwColorSpaceHighValue; } DDCOLORKEY, *LPDDCOLORKEY;
typedef struct _DDSCAPS2 { DWORD dwCaps; DWORD dwCaps2; DWORD dwCaps3; union { DWORD dwCaps4; DWORD dwVolumeDepth; }; } DDSCAPS2, *LPDDSCAPS2;

typedef struct _DDPIXELFORMAT
{
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwFourCC;
	union { DWORD dwRGBBitCount; DWORD dwYUVBitCount; DWORD dwZBufferBitDepth; DWORD dwAlphaBitDepth; DWORD dwLuminanceBitCount; DWORD dwBumpBitCount; DWORD dwPrivateFormatBitCount; };
	union { DWORD dwRBitMask; DWORD dwYBitMask; DWORD dwStencilBitDepth; DWORD dwLuminanceBitMask; DWORD dwBumpDuBitMask; DWORD dwOperations; };
	union { DWORD dwGBitMask; DWORD dwUBitMask; DWORD dwZBitMask; DWORD dwBumpDvBitMask; };
	union { DWORD dwBBitMask; DWORD dwVBitMask; DWORD dwStencilBitMask; DWORD dwBumpLuminanceBitMask; };
	union { DWORD dwRGBAlphaBitMask; DWORD dwYUVAlphaBitMask; DWORD dwLuminanceAlphaBitMask; DWORD dwRGBZBitMask; DWORD dwYUVZBitMask; };
} DDPIXELFORMAT, *LPDDPIXELFORMAT;

typedef struct _DDSURFACEDESC2
{
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwHeight;
	DWORD dwWidth;
	union { LONG lPitch; DWORD dwLinearSize; };
	union { DWORD dwBackBufferCount; DWORD dwDepth; };
	union { DWORD dwMipMapCount; DWORD dwRefreshRate; DWORD dwSrcVBHandle; };
	DWORD dwAlphaBitDepth;
	DWORD dwReserved;
	DWORD lpSurface;	// Cynex: dosyada 4 bayt, isaretci degil
	union { DDCOLORKEY ddckCKDestOverlay; DWORD dwEmptyFaceColor; };
	DDCOLORKEY ddckCKDestBlt;
	DDCOLORKEY ddckCKSrcOverlay;
	DDCOLORKEY ddckCKSrcBlt;
	union { DDPIXELFORMAT ddpfPixelFormat; DWORD dwFVF; };
	DDSCAPS2 ddsCaps;
	DWORD dwTextureStage;
} DDSURFACEDESC2, *LPDDSURFACEDESC2;

#ifdef __cplusplus
static_assert(sizeof(DDSURFACEDESC2) == 124, "DDSURFACEDESC2 124 bayt olmali");
static_assert(sizeof(DDPIXELFORMAT) == 32, "DDPIXELFORMAT 32 bayt olmali");
#endif

#define DDSD_CAPS 0x00000001l
#define DDSD_HEIGHT 0x00000002l
#define DDSD_WIDTH 0x00000004l
#define DDSD_PITCH 0x00000008l
#define DDSD_BACKBUFFERCOUNT 0x00000020l
#define DDSD_ZBUFFERBITDEPTH 0x00000040l
#define DDSD_ALPHABITDEPTH 0x00000080l
#define DDSD_LPSURFACE 0x00000800l
#define DDSD_PIXELFORMAT 0x00001000l
#define DDSD_CKDESTOVERLAY 0x00002000l
#define DDSD_CKDESTBLT 0x00004000l
#define DDSD_CKSRCOVERLAY 0x00008000l
#define DDSD_CKSRCBLT 0x00010000l
#define DDSD_MIPMAPCOUNT 0x00020000l
#define DDSD_REFRESHRATE 0x00040000l
#define DDSD_LINEARSIZE 0x00080000l
#define DDSD_TEXTURESTAGE 0x00100000l
#define DDSD_FVF 0x00200000l
#define DDSD_SRCVBHANDLE 0x00400000l
#define DDSD_DEPTH 0x00800000l
#define DDPF_ALPHAPIXELS 0x00000001l
#define DDPF_ALPHA 0x00000002l
#define DDPF_FOURCC 0x00000004l
#define DDPF_PALETTEINDEXED8 0x00000020l
#define DDPF_RGB 0x00000040l
#define DDPF_ALPHAPREMULT 0x00008000l
#define DDPF_LUMINANCE 0x00020000l
#define DDSCAPS_COMPLEX 0x00000008l
#define DDSCAPS_TEXTURE 0x00001000l
#define DDSCAPS_MIPMAP 0x00400000l
#define DDSCAPS2_CUBEMAP 0x00000200l
#define DDSCAPS2_VOLUME 0x00200000l
