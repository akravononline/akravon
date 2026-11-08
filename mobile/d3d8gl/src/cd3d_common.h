// Cynex: d3d8 -> gles3 ortak tanimlar
#pragma once

#include <d3d8.h>
#include <d3dx8.h>

#include <GLES3/gl3.h>
#include <EGL/egl.h>

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <atomic>
#include <vector>
#include <unordered_map>

#include <android/log.h>
#include <android/native_window.h>	// ANativeWindow global olarak (cd3d icinde yeniden bildirilmesin)

#define CD3D_TAG "CynexD3D8"
#define CD3D_LOGI(...) __android_log_print(ANDROID_LOG_INFO, CD3D_TAG, __VA_ARGS__)
#define CD3D_LOGW(...) __android_log_print(ANDROID_LOG_WARN, CD3D_TAG, __VA_ARGS__)
#define CD3D_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, CD3D_TAG, __VA_ARGS__)

// Cynex: gl2ext'e bagli kalmayalim
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

namespace cd3d {

class Device;
class Texture;
class Surface;
class VertexBuffer;
class IndexBuffer;

struct GLCaps
{
	bool s3tc = false;
	bool aniso = false;
	float maxAniso = 1.0f;
	int maxTexSize = 4096;
	int maxTexUnits = 8;
	bool useMapRange = true;	// Cynex: halka tamponu map ile mi subdata ile mi
	bool bufStorage = false;	// Cynex: GL_EXT_buffer_storage (kalici eslenmis halka tamponu)
	bool forceCpuDxt = false;
	bool checkErrors = false;
	bool dxtLowMem = false;	// Cynex: setprop debug.cynex.dxtlow 1 -> dxt3/5 cpu cozumu rgba4444	// Cynex: setprop debug.cynex.glerror 1
	char renderer[128] = "OpenGL ES 3.0";
	char vendor[64] = "Cynex";
};
extern GLCaps g_glcaps;

// Cynex: egl katmani (cd3d_egl.cpp)
bool Egl_Init();
bool Egl_IsReady();
GLuint Egl_ScreenFbo();	// Cynex: ekranin cerceve tamponu (android: 0, ios: gorunum cercevesi)
bool Egl_AttachWindow(struct ANativeWindow* window);
void Egl_DetachWindow();
bool Egl_HasWindow();
int Egl_SurfaceWidth();
int Egl_SurfaceHeight();
void Egl_SetSwapInterval(int interval);	// AKRAVON: 1 = 60 fps, 2 = 30 fps
bool Egl_Swap();
void Egl_MakeCurrent();
bool Egl_IsRenderThread();
void Egl_SetRenderThread();

// Cynex: format bilgisi (cd3d_format.cpp)
struct FormatInfo
{
	D3DFORMAT fmt;
	uint8_t bpp;			// sikistirilmamis: piksel basina bayt
	uint8_t blockBytes;		// dxt: blok basina bayt (0 = sikistirilmamis)
	bool hasAlpha;
	bool depth;
};
const FormatInfo* GetFormatInfo(D3DFORMAT fmt);
bool IsCompressed(D3DFORMAT fmt);
UINT SurfacePitch(D3DFORMAT fmt, UINT w);
UINT SurfaceRows(D3DFORMAT fmt, UINT h);
UINT SurfaceSize(D3DFORMAT fmt, UINT w, UINT h);

// Cynex: gl yukleme formati
struct GLUploadFmt
{
	GLenum internalFmt;
	GLenum format;
	GLenum type;
	GLint swizzle[4];
	int convert;			// 0 yok, CONV_*
	uint8_t dstBpp;
	bool compressed;
};
enum
{
	CONV_NONE = 0,
	CONV_1555,			// argb1555 -> rgba5551
	CONV_X1555,
	CONV_RGB24,			// r8g8b8 -> rgba8 (bgr sirasi korunur, swizzle ile)
	CONV_DXT_RGBA8,		// dxt -> rgba8 (cpu)
	CONV_DXT1_565,		// dxt1 -> rgb565 (cpu)
	CONV_DXT1_5551,		// dxt1 -> rgba5551 (cpu)
	CONV_DXT_4444,		// dxt3/5 -> rgba4444 (cpu, dusuk bellek)
};
bool GetGLUploadFmt(D3DFORMAT fmt, bool renderTarget, GLUploadFmt* out);

// Cynex: piksel donusumleri - hepsi bgra8 (d3d A8R8G8B8 bellek duzeni) uzerinden
void DecodeToBGRA8(D3DFORMAT fmt, const uint8_t* src, UINT srcPitch, UINT w, UINT h, uint8_t* dst, UINT dstPitch);
bool EncodeFromBGRA8(D3DFORMAT fmt, const uint8_t* src, UINT srcPitch, UINT w, UINT h, uint8_t* dst, UINT dstPitch);
void DecodeDXTBlock(D3DFORMAT fmt, const uint8_t* block, uint8_t out[64]);	// rgba8 4x4
bool DXT1HasTransparency(const uint8_t* data, UINT w, UINT h);
void DecodeDXTLevel(D3DFORMAT fmt, const uint8_t* src, UINT w, UINT h, uint8_t* dstRGBA, int outKind);

// Cynex: matris yardimcilari
inline void MatMul(D3DMATRIX& out, const D3DMATRIX& a, const D3DMATRIX& b)
{
	D3DMATRIX r;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			r.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j] + a.m[i][2] * b.m[2][j] + a.m[i][3] * b.m[3][j];
	out = r;
}
inline void MatIdentity(D3DMATRIX& m)
{
	memset(&m, 0, sizeof(m));
	m._11 = m._22 = m._33 = m._44 = 1.0f;
}
inline float DwToF(DWORD d) { float f; memcpy(&f, &d, 4); return f; }
inline void ColorToVec4(D3DCOLOR c, float* v)
{
	v[0] = ((c >> 16) & 0xFF) / 255.0f;
	v[1] = ((c >> 8) & 0xFF) / 255.0f;
	v[2] = (c & 0xFF) / 255.0f;
	v[3] = ((c >> 24) & 0xFF) / 255.0f;
}

// Cynex: com sayaci - IUnknown'in sanal yikicisi var, delete this guvenli
#define CD3D_IUNKNOWN_IMPL(IFACE) \
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override \
	{ \
		if (!ppv) return E_POINTER; \
		if (riid == IID_IUnknown || riid == IID_##IFACE) { *ppv = this; AddRef(); return S_OK; } \
		*ppv = NULL; return E_NOINTERFACE; \
	}

}	// namespace cd3d

namespace cd3d {
void FillCaps(D3DCAPS8* c);
}
