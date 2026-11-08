// Cynex iOS: cd3d_egl.cpp karsiligi - EAGL (OpenGL ES 3) baglami + CAEAGLLayer cizim yuzeyi
// Egl_* arayuzu ayni; ekran cercevesi (framebuffer 0 degil) Egl_ScreenFbo() ile alinir.
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <QuartzCore/QuartzCore.h>

#include "cd3d_common.h"
#include <pthread.h>
#ifdef interface
#undef interface	// objbase.h: interface=struct; Objective-C @interface'i bozar
#endif

@interface CynexView : UIView
@property (nonatomic, readonly) int pixelWidth;
@property (nonatomic, readonly) int pixelHeight;
@end

namespace cd3d {

GLCaps g_glcaps;

namespace {

struct EaglState
{
	EAGLContext* ctx = nil;
	CynexView* view = nil;
	GLuint fbo = 0, colorRb = 0, depthRb = 0;
	int w = 0, h = 0;
	pthread_t renderThread;
	bool hasRenderThread = false;
	int interval = 1;
} s_e;

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

	// apple gpu: s3tc yok (dxt dokular cpu'da acilir)
	g_glcaps.s3tc = HasExt(ext, "GL_EXT_texture_compression_s3tc");
	g_glcaps.aniso = HasExt(ext, "GL_EXT_texture_filter_anisotropic");
	if (g_glcaps.aniso)
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &g_glcaps.maxAniso);

	GLint v = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &v);
	g_glcaps.maxTexSize = v > 0 ? v : 2048;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &v);
	g_glcaps.maxTexUnits = v < 8 ? v : 8;

	g_glcaps.bufStorage = false;	// apple gles'te GL_EXT_buffer_storage yok
	g_glcaps.useMapRange = false;	// glBufferSubData yolu (android'de de boyleydi)

	CD3D_LOGI("GL: %s / %s s3tc=%d map=%d aniso=%d(%.0f) maxtex=%d units=%d", g_glcaps.vendor, g_glcaps.renderer,
		g_glcaps.s3tc, g_glcaps.useMapRange, g_glcaps.aniso, g_glcaps.maxAniso, g_glcaps.maxTexSize, g_glcaps.maxTexUnits);
}

void DestroyTargets()
{
	if (s_e.fbo) { glDeleteFramebuffers(1, &s_e.fbo); s_e.fbo = 0; }
	if (s_e.colorRb) { glDeleteRenderbuffers(1, &s_e.colorRb); s_e.colorRb = 0; }
	if (s_e.depthRb) { glDeleteRenderbuffers(1, &s_e.depthRb); s_e.depthRb = 0; }
}

}	// namespace

GLuint Egl_ScreenFbo()
{
	return s_e.fbo;
}

bool Egl_Init()
{
	if (s_e.ctx)
		return true;
	s_e.ctx = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
	if (!s_e.ctx)
	{
		CD3D_LOGE("EAGLContext (gles3) olusturulamadi");
		return false;
	}
	if (![EAGLContext setCurrentContext:s_e.ctx])
	{
		CD3D_LOGE("EAGLContext baglanamadi");
		return false;
	}
	Egl_SetRenderThread();
	QueryGLCaps();
	return true;
}

bool Egl_IsReady()
{
	return s_e.ctx != nil;
}

bool Egl_AttachWindow(struct ANativeWindow* window)
{
	if (!Egl_IsReady() || !window)
		return false;
	CynexView* view = (__bridge CynexView*)(void*)window;
	if (s_e.view == view && s_e.fbo)
		return true;

	Egl_MakeCurrent();
	DestroyTargets();
	s_e.view = view;

	glGenFramebuffers(1, &s_e.fbo);
	glGenRenderbuffers(1, &s_e.colorRb);
	glBindFramebuffer(GL_FRAMEBUFFER, s_e.fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, s_e.colorRb);
	if (![s_e.ctx renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)view.layer])
	{
		CD3D_LOGE("renderbufferStorage basarisiz");
		DestroyTargets();
		s_e.view = nil;
		return false;
	}
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, s_e.colorRb);
	GLint rw = 0, rh = 0;
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &rw);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &rh);
	s_e.w = rw;
	s_e.h = rh;

	glGenRenderbuffers(1, &s_e.depthRb);
	glBindRenderbuffer(GL_RENDERBUFFER, s_e.depthRb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, rw, rh);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, s_e.depthRb);

	const GLenum st = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (st != GL_FRAMEBUFFER_COMPLETE)
	{
		CD3D_LOGE("ekran cerceve tamponu eksik 0x%x", st);
		DestroyTargets();
		s_e.view = nil;
		return false;
	}
	glBindRenderbuffer(GL_RENDERBUFFER, s_e.colorRb);
	CD3D_LOGI("pencere yuzeyi %dx%d", s_e.w, s_e.h);
	return true;
}

void Egl_DetachWindow()
{
	if (!s_e.view)
		return;
	Egl_MakeCurrent();
	DestroyTargets();
	s_e.view = nil;
	s_e.w = s_e.h = 0;
	CD3D_LOGI("pencere yuzeyi birakildi");
}

void Egl_SetSwapInterval(int interval)
{
	// ios: ekran yenileme 60 Hz'de sabit; 30 fps icin her ikinci kare atlanir
	s_e.interval = interval < 1 ? 1 : (interval > 4 ? 4 : interval);
}

bool Egl_HasWindow()
{
	return s_e.view != nil && s_e.fbo != 0;
}

int Egl_SurfaceWidth()
{
	return s_e.w;
}

int Egl_SurfaceHeight()
{
	return s_e.h;
}

bool Egl_Swap()
{
	if (!Egl_HasWindow())
		return false;
	// 30 fps: bazi karelerde sunulmaz, bekleme kare suresini tutar
	static int s_skip = 0;
	glBindRenderbuffer(GL_RENDERBUFFER, s_e.colorRb);
	if (![s_e.ctx presentRenderbuffer:GL_RENDERBUFFER])
	{
		CD3D_LOGW("presentRenderbuffer basarisiz");
		return false;
	}
	if (s_e.interval > 1)
	{
		++s_skip;
		usleep((s_e.interval - 1) * 16666);
	}
	return true;
}

void Egl_MakeCurrent()
{
	if (!Egl_IsReady())
		return;
	if ([EAGLContext currentContext] != s_e.ctx)
		[EAGLContext setCurrentContext:s_e.ctx];
}

bool Egl_IsRenderThread()
{
	return s_e.hasRenderThread && pthread_equal(s_e.renderThread, pthread_self());
}

void Egl_SetRenderThread()
{
	s_e.renderThread = pthread_self();
	s_e.hasRenderThread = true;
}

}	// namespace cd3d
