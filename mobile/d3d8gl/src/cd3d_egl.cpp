// Cynex: egl baglami - pencere gidince yalniz yuzey yok edilir, baglam kalir
#include "cd3d_common.h"
#include <android/native_window.h>
#include <pthread.h>
#include <sys/system_properties.h>

namespace cd3d {

GLCaps g_glcaps;

namespace {

struct EglState
{
	EGLDisplay dpy = EGL_NO_DISPLAY;
	EGLConfig cfg = nullptr;
	EGLContext ctx = EGL_NO_CONTEXT;
	EGLSurface win = EGL_NO_SURFACE;
	EGLSurface pbuf = EGL_NO_SURFACE;
	ANativeWindow* window = nullptr;
	int w = 0, h = 0;
	bool surfaceless = false;
	pthread_t renderThread;
	bool hasRenderThread = false;
} s_egl;

#ifndef EGL_OPENGL_ES3_BIT_KHR
#define EGL_OPENGL_ES3_BIT_KHR 0x00000040
#endif

bool ChooseConfig(EGLint surfaceType)
{
	const EGLint attribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
		EGL_SURFACE_TYPE, surfaceType,
		EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 24, EGL_STENCIL_SIZE, 8,
		EGL_NONE
	};
	EGLConfig cfgs[64];
	EGLint n = 0;
	if (!eglChooseConfig(s_egl.dpy, attribs, cfgs, 64, &n) || n <= 0)
		return false;

	// Cynex: tam rgba8/d24s8 olani sec, msaa'siz
	int best = -1;
	for (int i = 0; i < n; ++i)
	{
		EGLint r, g, b, a, d, s, samples;
		eglGetConfigAttrib(s_egl.dpy, cfgs[i], EGL_RED_SIZE, &r);
		eglGetConfigAttrib(s_egl.dpy, cfgs[i], EGL_GREEN_SIZE, &g);
		eglGetConfigAttrib(s_egl.dpy, cfgs[i], EGL_BLUE_SIZE, &b);
		eglGetConfigAttrib(s_egl.dpy, cfgs[i], EGL_ALPHA_SIZE, &a);
		eglGetConfigAttrib(s_egl.dpy, cfgs[i], EGL_DEPTH_SIZE, &d);
		eglGetConfigAttrib(s_egl.dpy, cfgs[i], EGL_STENCIL_SIZE, &s);
		eglGetConfigAttrib(s_egl.dpy, cfgs[i], EGL_SAMPLES, &samples);
		if (r == 8 && g == 8 && b == 8 && a == 8 && d == 24 && s == 8 && samples == 0)
		{
			best = i;
			break;
		}
		if (best < 0)
			best = i;
	}
	s_egl.cfg = cfgs[best];
	return true;
}

bool HasExt(const char* list, const char* name)
{
	if (!list)
		return false;
	size_t n = strlen(name);
	const char* p = list;
	while ((p = strstr(p, name)) != nullptr)
	{
		if ((p == list || p[-1] == ' ') && (p[n] == ' ' || p[n] == 0))
			return true;
		p += n;
	}
	return false;
}

void QueryGLCaps()
{
	const char* ext = (const char*)glGetString(GL_EXTENSIONS);
	const char* ren = (const char*)glGetString(GL_RENDERER);
	const char* ven = (const char*)glGetString(GL_VENDOR);
	if (ren)
		strncpy(g_glcaps.renderer, ren, sizeof(g_glcaps.renderer) - 1);
	if (ven)
		strncpy(g_glcaps.vendor, ven, sizeof(g_glcaps.vendor) - 1);

	g_glcaps.s3tc = HasExt(ext, "GL_EXT_texture_compression_s3tc") ||
		(HasExt(ext, "GL_EXT_texture_compression_dxt1") && HasExt(ext, "GL_ANGLE_texture_compression_dxt3") && HasExt(ext, "GL_ANGLE_texture_compression_dxt5"));
	g_glcaps.aniso = HasExt(ext, "GL_EXT_texture_filter_anisotropic");
	if (g_glcaps.aniso)
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &g_glcaps.maxAniso);

	GLint v = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &v);
	g_glcaps.maxTexSize = v > 0 ? v : 2048;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &v);
	g_glcaps.maxTexUnits = v < 8 ? v : 8;

	// Cynex: halka tamponu glMapBufferRange ile yazmak Adreno 660'ta cizim basina ~1.5 ms tutuyordu (oyun 1-2 FPS);
	// glBufferSubData yolu ~30 FPS. Emulator gles kodlayicisi de map'te geri okuma yapiyor. Her ikisinde subdata kullan.
	// (debug.cynex.map 1 ile eski yol denenebilir)
	g_glcaps.bufStorage = HasExt(ext, "GL_EXT_buffer_storage");
	{ char pb[PROP_VALUE_MAX] = {0}; if (__system_property_get("debug.cynex.nopersist", pb) > 0 && pb[0] == '1') g_glcaps.bufStorage = false; }
	g_glcaps.useMapRange = false;
	{ char pm[PROP_VALUE_MAX] = {0}; if (__system_property_get("debug.cynex.map", pm) > 0 && pm[0] == '1') g_glcaps.useMapRange = true; }

	// Cynex: debug icin dxt cpu yolu zorlanabilir (setprop debug.cynex.cpudxt 1)
	char prop[PROP_VALUE_MAX] = {0};
	if (__system_property_get("debug.cynex.cpudxt", prop) > 0 && prop[0] == '1')
		g_glcaps.forceCpuDxt = true;
	if (__system_property_get("debug.cynex.dxtlow", prop) > 0 && prop[0] == '1')
		g_glcaps.dxtLowMem = true;
	if (__system_property_get("debug.cynex.glerror", prop) > 0 && prop[0] == '1')
		g_glcaps.checkErrors = true;
	if (__system_property_get("debug.cynex.subdata", prop) > 0 && prop[0] == '1')
		g_glcaps.useMapRange = false;

	CD3D_LOGI("GL: %s / %s s3tc=%d map=%d aniso=%d(%.0f) maxtex=%d units=%d", g_glcaps.vendor, g_glcaps.renderer,
		g_glcaps.s3tc, g_glcaps.useMapRange, g_glcaps.aniso, g_glcaps.maxAniso, g_glcaps.maxTexSize, g_glcaps.maxTexUnits);
}

}	// namespace

bool Egl_Init()
{
	if (s_egl.ctx != EGL_NO_CONTEXT)
		return true;

	s_egl.dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (s_egl.dpy == EGL_NO_DISPLAY || !eglInitialize(s_egl.dpy, nullptr, nullptr))
	{
		CD3D_LOGE("eglInitialize basarisiz");
		return false;
	}

	if (!ChooseConfig(EGL_WINDOW_BIT | EGL_PBUFFER_BIT))
	{
		if (!ChooseConfig(EGL_WINDOW_BIT))
		{
			CD3D_LOGE("uygun egl config yok");
			return false;
		}
		s_egl.surfaceless = true;
	}

	const EGLint ctxAttr[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
	s_egl.ctx = eglCreateContext(s_egl.dpy, s_egl.cfg, EGL_NO_CONTEXT, ctxAttr);
	if (s_egl.ctx == EGL_NO_CONTEXT)
	{
		CD3D_LOGE("eglCreateContext basarisiz 0x%x", eglGetError());
		return false;
	}

	if (!s_egl.surfaceless)
	{
		const EGLint pbAttr[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
		s_egl.pbuf = eglCreatePbufferSurface(s_egl.dpy, s_egl.cfg, pbAttr);
		if (s_egl.pbuf == EGL_NO_SURFACE)
			s_egl.surfaceless = true;
	}

	EGLSurface s = s_egl.surfaceless ? EGL_NO_SURFACE : s_egl.pbuf;
	if (!eglMakeCurrent(s_egl.dpy, s, s, s_egl.ctx))
	{
		CD3D_LOGE("eglMakeCurrent (pbuffer) basarisiz 0x%x", eglGetError());
		return false;
	}

	Egl_SetRenderThread();
	QueryGLCaps();
	return true;
}

GLuint Egl_ScreenFbo()
{
	return 0;
}

bool Egl_IsReady()
{
	return s_egl.ctx != EGL_NO_CONTEXT;
}

static int s_swapInterval = 1;	// AKRAVON: oyun secenekleri FPS

bool Egl_AttachWindow(ANativeWindow* window)
{
	if (!Egl_IsReady() || !window)
		return false;

	if (s_egl.win != EGL_NO_SURFACE && s_egl.window == window)
		return true;

	Egl_DetachWindow();

	s_egl.win = eglCreateWindowSurface(s_egl.dpy, s_egl.cfg, window, nullptr);
	if (s_egl.win == EGL_NO_SURFACE)
	{
		CD3D_LOGE("eglCreateWindowSurface basarisiz 0x%x", eglGetError());
		return false;
	}
	s_egl.window = window;
	if (!eglMakeCurrent(s_egl.dpy, s_egl.win, s_egl.win, s_egl.ctx))
	{
		CD3D_LOGE("eglMakeCurrent (pencere) basarisiz 0x%x", eglGetError());
		eglDestroySurface(s_egl.dpy, s_egl.win);
		s_egl.win = EGL_NO_SURFACE;
		s_egl.window = nullptr;
		return false;
	}
	eglQuerySurface(s_egl.dpy, s_egl.win, EGL_WIDTH, &s_egl.w);
	eglQuerySurface(s_egl.dpy, s_egl.win, EGL_HEIGHT, &s_egl.h);
	eglSwapInterval(s_egl.dpy, s_swapInterval);
	CD3D_LOGI("pencere yuzeyi %dx%d", s_egl.w, s_egl.h);
	return true;
}

void Egl_DetachWindow()
{
	if (s_egl.win == EGL_NO_SURFACE)
		return;

	EGLSurface s = s_egl.surfaceless ? EGL_NO_SURFACE : s_egl.pbuf;
	eglMakeCurrent(s_egl.dpy, s, s, s_egl.ctx);
	eglDestroySurface(s_egl.dpy, s_egl.win);
	s_egl.win = EGL_NO_SURFACE;
	s_egl.window = nullptr;
	s_egl.w = s_egl.h = 0;
	CD3D_LOGI("pencere yuzeyi birakildi");
}

void Egl_SetSwapInterval(int interval)
{
	interval = interval < 1 ? 1 : (interval > 4 ? 4 : interval);
	if (interval == s_swapInterval)
		return;
	s_swapInterval = interval;
	if (s_egl.win != EGL_NO_SURFACE)
		eglSwapInterval(s_egl.dpy, interval);
}

bool Egl_HasWindow()
{
	return s_egl.win != EGL_NO_SURFACE;
}

int Egl_SurfaceWidth()
{
	if (s_egl.win != EGL_NO_SURFACE)
		eglQuerySurface(s_egl.dpy, s_egl.win, EGL_WIDTH, &s_egl.w);
	return s_egl.w;
}

int Egl_SurfaceHeight()
{
	if (s_egl.win != EGL_NO_SURFACE)
		eglQuerySurface(s_egl.dpy, s_egl.win, EGL_HEIGHT, &s_egl.h);
	return s_egl.h;
}

bool Egl_Swap()
{
	if (s_egl.win == EGL_NO_SURFACE)
		return false;
	if (!eglSwapBuffers(s_egl.dpy, s_egl.win))
	{
		EGLint e = eglGetError();
		CD3D_LOGW("eglSwapBuffers 0x%x", e);
		return false;
	}
	return true;
}

void Egl_MakeCurrent()
{
	if (!Egl_IsReady())
		return;
	EGLSurface s = s_egl.win != EGL_NO_SURFACE ? s_egl.win : (s_egl.surfaceless ? EGL_NO_SURFACE : s_egl.pbuf);
	if (eglGetCurrentContext() != s_egl.ctx)
		eglMakeCurrent(s_egl.dpy, s, s, s_egl.ctx);
}

bool Egl_IsRenderThread()
{
	return s_egl.hasRenderThread && pthread_equal(s_egl.renderThread, pthread_self());
}

void Egl_SetRenderThread()
{
	s_egl.renderThread = pthread_self();
	s_egl.hasRenderThread = true;
}

}	// namespace cd3d
