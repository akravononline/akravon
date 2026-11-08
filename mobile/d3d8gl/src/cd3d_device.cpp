// Cynex: d3d8 -> gles3 cihaz (durum, kaynaklar, hedefler, sunum)
#include "cd3d_device.h"
#include "CynexPlatform.h"
#include <algorithm>
#include <mutex>
#include <math.h>

namespace cd3d {

Device* g_device = nullptr;
uint32_t g_frame = 1;
PerfCtr g_perf;
static std::mutex s_delMutex;

// ---------------------------------------------------------------- gl silme

void GLDeleteTexture(GLuint t)
{
	if (!t) return;
	if (Egl_IsReady() && Egl_IsRenderThread())
	{
		if (g_device)
		{
			GLCache& c = g_device->Cache();
			for (int i = 0; i < 8; ++i)
				if (c.tex[i] == t) c.tex[i] = 0;
		}
		glDeleteTextures(1, &t);
	}
	else if (g_device)
		g_device->QueueDelete(0, t);
}

void GLDeleteBuffer(GLuint b)
{
	if (!b) return;
	if (Egl_IsReady() && Egl_IsRenderThread())
	{
		if (g_device)
		{
			GLCache& c = g_device->Cache();
			if (c.arrayBuf == b) c.arrayBuf = 0;
			if (c.elemBuf == b) c.elemBuf = 0;
			for (int i = 0; i < ATTR_COUNT; ++i)
				if (c.attr[i].buf == b) c.attr[i].buf = 0xFFFFFFFF;
		}
		glDeleteBuffers(1, &b);
	}
	else if (g_device)
		g_device->QueueDelete(1, b);
}

void GLDeleteRenderbuffer(GLuint r)
{
	if (!r) return;
	if (Egl_IsReady() && Egl_IsRenderThread())
		glDeleteRenderbuffers(1, &r);
	else if (g_device)
		g_device->QueueDelete(2, r);
}

void Device::QueueDelete(int kind, GLuint name)
{
	std::lock_guard<std::mutex> lk(s_delMutex);
	m_pendingDeletes.push_back(std::make_pair(kind, name));
}

void Device::FlushDeletes()
{
	std::vector<std::pair<int, GLuint>> list;
	{
		std::lock_guard<std::mutex> lk(s_delMutex);
		if (m_pendingDeletes.empty())
			return;
		list.swap(m_pendingDeletes);
	}
	for (auto& d : list)
	{
		if (d.first == 0) GLDeleteTexture(d.second);
		else if (d.first == 1) GLDeleteBuffer(d.second);
		else if (d.first == 2) GLDeleteRenderbuffer(d.second);
		else if (d.first == 3) { glDeleteFramebuffers(1, &d.second); m_gl.fbo = 0xFFFFFFFF; }
	}
}

void GLCache::Invalidate()
{
	program = 0xFFFFFFFF;
	fbo = 0xFFFFFFFF;
	arrayBuf = elemBuf = 0xFFFFFFFF;
	activeUnit = -1;
	for (int i = 0; i < 8; ++i) { tex[i] = 0xFFFFFFFF; sampler[i] = 0xFFFFFFFF; }
	for (int i = 0; i < ATTR_COUNT; ++i) attr[i].buf = 0xFFFFFFFF;
	vp[0] = vp[1] = vp[2] = vp[3] = -1;
	depthRange[0] = depthRange[1] = -1;
}

// ---------------------------------------------------------------- pencere geri cagrisi

static void WindowCallback(ANativeWindow* window, int created)
{
	if (g_device)
		g_device->OnWindow(window, created != 0);
	else if (created)
		Egl_AttachWindow(window);
	else
		Egl_DetachWindow();
}

void Device::OnWindow(ANativeWindow* window, bool created)
{
	if (created)
		Egl_AttachWindow(window);
	else
		Egl_DetachWindow();
	m_gl.fbo = 0xFFFFFFFF;
	m_dirty |= DIRTY_FBO | DIRTY_VIEWPORT | DIRTY_RASTER;
}

// ---------------------------------------------------------------- yapim

Device::Device(IDirect3D8* d3d, HWND hwnd, DWORD behavior, D3DPRESENT_PARAMETERS* pp)
	: m_d3d(d3d), m_hwnd(hwnd), m_behavior(behavior)
{
	m_pp = *pp;
	memset(m_tex, 0, sizeof(m_tex));
	memset(m_streams, 0, sizeof(m_streams));
	memset(&m_key, 0, sizeof(m_key));
	memset(m_clipPlanes, 0, sizeof(m_clipPlanes));
	for (int i = 0; i < SG_COUNT; ++i)
		m_serial[i] = 1;
	for (int i = 0; i < 256; ++i)
	{
		m_gamma.red[i] = m_gamma.green[i] = m_gamma.blue[i] = (WORD)(i * 257);
	}
	m_vsTable.push_back(nullptr);	// Cynex: 0 bos tutamac
	m_vsConst.resize(96 * 4, 0.0f);
	if (m_d3d)
		m_d3d->AddRef();
}

static void FixPP(D3DPRESENT_PARAMETERS* pp)
{
	if (!pp->BackBufferWidth || !pp->BackBufferHeight)
	{
		int w = Egl_HasWindow() ? Egl_SurfaceWidth() : CynexPlatform_GetWidth();
		int h = Egl_HasWindow() ? Egl_SurfaceHeight() : CynexPlatform_GetHeight();
		pp->BackBufferWidth = w > 0 ? w : 1280;
		pp->BackBufferHeight = h > 0 ? h : 720;
	}
	if (pp->BackBufferFormat == D3DFMT_UNKNOWN)
		pp->BackBufferFormat = D3DFMT_X8R8G8B8;
	if (!pp->BackBufferCount)
		pp->BackBufferCount = 1;
	if (pp->EnableAutoDepthStencil && pp->AutoDepthStencilFormat == D3DFMT_UNKNOWN)
		pp->AutoDepthStencilFormat = D3DFMT_D24S8;
}

bool Device::Init()
{
	g_device = this;
	Egl_MakeCurrent();
	FixPP(&m_pp);

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	m_vring.Init(GL_ARRAY_BUFFER, g_glcaps.bufStorage ? 8 * 1024 * 1024 : 4 * 1024 * 1024);
	m_iring.Init(GL_ELEMENT_ARRAY_BUFFER, g_glcaps.bufStorage ? 2 * 1024 * 1024 : 1024 * 1024);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glDisable(GL_DITHER);

	// Cynex: gl'yi onbellek varsayilanlarina cek
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);
	glBlendEquation(GL_FUNC_ADD);
	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glDisable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glDisable(GL_STENCIL_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_POLYGON_OFFSET_FILL);
	m_gl.Invalidate();

	m_backbuffer = new Surface(this, SURF_BACKBUFFER, m_pp.BackBufferWidth, m_pp.BackBufferHeight, m_pp.BackBufferFormat, D3DPOOL_DEFAULT, D3DUSAGE_RENDERTARGET);
	m_autoDepth = new Surface(this, SURF_AUTODEPTH, m_pp.BackBufferWidth, m_pp.BackBufferHeight,
		m_pp.AutoDepthStencilFormat != D3DFMT_UNKNOWN ? m_pp.AutoDepthStencilFormat : D3DFMT_D24S8, D3DPOOL_DEFAULT, D3DUSAGE_DEPTHSTENCIL);
	m_rt = m_backbuffer;
	m_ds = m_pp.EnableAutoDepthStencil ? m_autoDepth : nullptr;

	SetDefaultStates();
	CynexPlatform_SetWindowCallback(WindowCallback);
	CD3D_LOGI("cihaz hazir: %ux%u fmt=%d ds=%d", m_pp.BackBufferWidth, m_pp.BackBufferHeight, (int)m_pp.BackBufferFormat, (int)m_pp.AutoDepthStencilFormat);
	return true;
}

Device::~Device()
{
	g_device = nullptr;
	CynexPlatform_SetWindowCallback(nullptr);
	if (Egl_IsRenderThread())
		Egl_MakeCurrent();

	for (int i = 0; i < 8; ++i)
		if (m_tex[i]) { m_tex[i]->Release(); m_tex[i] = nullptr; }
	for (int i = 0; i < 16; ++i)
		if (m_streams[i].vb) { m_streams[i].vb->Release(); m_streams[i].vb = nullptr; }
	if (m_ib) { m_ib->Release(); m_ib = nullptr; }
	if (m_rt && m_rt != m_backbuffer) m_rt->Release();
	if (m_ds && m_ds != m_autoDepth) m_ds->Release();
	m_rt = m_ds = nullptr;

	for (FboEntry& f : m_fbos)
	{
		glDeleteFramebuffers(1, &f.fbo);
		if (f.privDepth) glDeleteRenderbuffers(1, &f.privDepth);
	}
	m_fbos.clear();
	DestroyOffscreenBackbuffer();
	for (auto& s : m_samplers)
		glDeleteSamplers(1, &s.second);
	m_samplers.clear();
	m_ffp.Clear();
	m_vring.Destroy();
	m_iring.Destroy();
	if (m_vao) glDeleteVertexArrays(1, &m_vao);
	for (VShaderDecl* d : m_vsTable)
		delete d;

	delete m_backbuffer;
	delete m_autoDepth;
	{
		std::lock_guard<std::mutex> lk(s_delMutex);
		m_pendingDeletes.clear();
	}
	if (m_d3d)
		m_d3d->Release();
}

ULONG Device::Release()
{
	ULONG r = --m_ref;
	if (r == 0)
		delete this;
	return r;
}

// ---------------------------------------------------------------- varsayilan durum

void Device::SetDefaultStates()
{
	memset(m_rs, 0, sizeof(m_rs));
	m_rs[D3DRS_ZENABLE] = m_pp.EnableAutoDepthStencil ? D3DZB_TRUE : D3DZB_FALSE;
	m_rs[D3DRS_FILLMODE] = D3DFILL_SOLID;
	m_rs[D3DRS_SHADEMODE] = D3DSHADE_GOURAUD;
	m_rs[D3DRS_LINEPATTERN] = 0;
	m_rs[D3DRS_ZWRITEENABLE] = TRUE;
	m_rs[D3DRS_ALPHATESTENABLE] = FALSE;
	m_rs[D3DRS_LASTPIXEL] = TRUE;
	m_rs[D3DRS_SRCBLEND] = D3DBLEND_ONE;
	m_rs[D3DRS_DESTBLEND] = D3DBLEND_ZERO;
	m_rs[D3DRS_CULLMODE] = D3DCULL_CCW;
	m_rs[D3DRS_ZFUNC] = D3DCMP_LESSEQUAL;
	m_rs[D3DRS_ALPHAREF] = 0;
	m_rs[D3DRS_ALPHAFUNC] = D3DCMP_ALWAYS;
	m_rs[D3DRS_DITHERENABLE] = FALSE;
	m_rs[D3DRS_ALPHABLENDENABLE] = FALSE;
	m_rs[D3DRS_FOGENABLE] = FALSE;
	m_rs[D3DRS_SPECULARENABLE] = FALSE;
	m_rs[D3DRS_FOGCOLOR] = 0;
	m_rs[D3DRS_FOGTABLEMODE] = D3DFOG_NONE;
	float f0 = 0.0f, f1 = 1.0f;
	memcpy(&m_rs[D3DRS_FOGSTART], &f0, 4);
	memcpy(&m_rs[D3DRS_FOGEND], &f1, 4);
	memcpy(&m_rs[D3DRS_FOGDENSITY], &f1, 4);
	m_rs[D3DRS_RANGEFOGENABLE] = FALSE;
	m_rs[D3DRS_STENCILENABLE] = FALSE;
	m_rs[D3DRS_STENCILFAIL] = D3DSTENCILOP_KEEP;
	m_rs[D3DRS_STENCILZFAIL] = D3DSTENCILOP_KEEP;
	m_rs[D3DRS_STENCILPASS] = D3DSTENCILOP_KEEP;
	m_rs[D3DRS_STENCILFUNC] = D3DCMP_ALWAYS;
	m_rs[D3DRS_STENCILREF] = 0;
	m_rs[D3DRS_STENCILMASK] = 0xFFFFFFFF;
	m_rs[D3DRS_STENCILWRITEMASK] = 0xFFFFFFFF;
	m_rs[D3DRS_TEXTUREFACTOR] = 0xFFFFFFFF;
	m_rs[D3DRS_CLIPPING] = TRUE;
	m_rs[D3DRS_LIGHTING] = TRUE;
	m_rs[D3DRS_AMBIENT] = 0;
	m_rs[D3DRS_FOGVERTEXMODE] = D3DFOG_NONE;
	m_rs[D3DRS_COLORVERTEX] = TRUE;
	m_rs[D3DRS_LOCALVIEWER] = TRUE;
	m_rs[D3DRS_NORMALIZENORMALS] = FALSE;
	m_rs[D3DRS_DIFFUSEMATERIALSOURCE] = D3DMCS_COLOR1;
	m_rs[D3DRS_SPECULARMATERIALSOURCE] = D3DMCS_COLOR2;
	m_rs[D3DRS_AMBIENTMATERIALSOURCE] = D3DMCS_MATERIAL;
	m_rs[D3DRS_EMISSIVEMATERIALSOURCE] = D3DMCS_MATERIAL;
	m_rs[D3DRS_VERTEXBLEND] = D3DVBF_DISABLE;
	m_rs[D3DRS_CLIPPLANEENABLE] = 0;
	memcpy(&m_rs[D3DRS_POINTSIZE], &f1, 4);
	memcpy(&m_rs[D3DRS_POINTSIZE_MIN], &f1, 4);
	m_rs[D3DRS_MULTISAMPLEANTIALIAS] = TRUE;
	m_rs[D3DRS_MULTISAMPLEMASK] = 0xFFFFFFFF;
	m_rs[D3DRS_COLORWRITEENABLE] = 0x0000000F;
	m_rs[D3DRS_BLENDOP] = D3DBLENDOP_ADD;
	m_rs[D3DRS_ZBIAS] = 0;

	memset(m_tss, 0, sizeof(m_tss));
	for (int s = 0; s < 8; ++s)
	{
		DWORD* t = m_tss[s];
		t[D3DTSS_COLOROP] = s == 0 ? D3DTOP_MODULATE : D3DTOP_DISABLE;
		t[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
		t[D3DTSS_COLORARG2] = D3DTA_CURRENT;
		t[D3DTSS_ALPHAOP] = s == 0 ? D3DTOP_SELECTARG1 : D3DTOP_DISABLE;
		t[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
		t[D3DTSS_ALPHAARG2] = D3DTA_CURRENT;
		t[D3DTSS_TEXCOORDINDEX] = s;
		t[D3DTSS_ADDRESSU] = D3DTADDRESS_WRAP;
		t[D3DTSS_ADDRESSV] = D3DTADDRESS_WRAP;
		t[D3DTSS_ADDRESSW] = D3DTADDRESS_WRAP;
		t[D3DTSS_BORDERCOLOR] = 0;
		t[D3DTSS_MAGFILTER] = D3DTEXF_POINT;
		t[D3DTSS_MINFILTER] = D3DTEXF_POINT;
		t[D3DTSS_MIPFILTER] = D3DTEXF_NONE;
		t[D3DTSS_MAXANISOTROPY] = 1;
		t[D3DTSS_TEXTURETRANSFORMFLAGS] = D3DTTFF_DISABLE;
		t[D3DTSS_COLORARG0] = D3DTA_CURRENT;
		t[D3DTSS_ALPHAARG0] = D3DTA_CURRENT;
		t[D3DTSS_RESULTARG] = D3DTA_CURRENT;
	}

	MatIdentity(m_world);
	MatIdentity(m_view);
	MatIdentity(m_proj);
	for (int i = 0; i < 8; ++i)
		MatIdentity(m_texMat[i]);
	for (int i = 0; i < 3; ++i)
		MatIdentity(m_worldExtra[i]);
	memset(&m_mat, 0, sizeof(m_mat));
	m_lights.clear();
	m_numActiveLights = 0;

	m_vp.X = m_vp.Y = 0;
	m_vp.Width = m_pp.BackBufferWidth;
	m_vp.Height = m_pp.BackBufferHeight;
	m_vp.MinZ = 0.0f;
	m_vp.MaxZ = 1.0f;

	for (int i = 0; i < 8; ++i)
		if (m_tex[i]) { m_tex[i]->Release(); m_tex[i] = nullptr; }
	for (int i = 0; i < 16; ++i)
		if (m_streams[i].vb) { m_streams[i].vb->Release(); m_streams[i] = Stream{ nullptr, 0 }; }
	if (m_ib) { m_ib->Release(); m_ib = nullptr; }
	m_baseVertex = 0;
	m_vs = D3DFVF_XYZ;
	m_ps = 0;

	m_dirty = 0xFFFFFFFF;
	for (int i = 0; i < SG_COUNT; ++i)
		++m_serial[i];
	m_prog = nullptr;
}

// ---------------------------------------------------------------- genel

HRESULT Device::TestCooperativeLevel()
{
	if (!Egl_HasWindow())
		return D3DERR_DEVICELOST;
	return D3D_OK;
}

UINT Device::GetAvailableTextureMem()
{
	return 512u * 1024u * 1024u;
}

HRESULT Device::GetDirect3D(IDirect3D8** pp)
{
	if (!pp) return D3DERR_INVALIDCALL;
	*pp = m_d3d;
	if (m_d3d) m_d3d->AddRef();
	return D3D_OK;
}

HRESULT Device::GetDeviceCaps(D3DCAPS8* c)
{
	if (!c) return D3DERR_INVALIDCALL;
	FillCaps(c);
	return D3D_OK;
}

HRESULT Device::GetDisplayMode(D3DDISPLAYMODE* m)
{
	if (!m) return D3DERR_INVALIDCALL;
	int w = Egl_HasWindow() ? Egl_SurfaceWidth() : CynexPlatform_GetWidth();
	int h = Egl_HasWindow() ? Egl_SurfaceHeight() : CynexPlatform_GetHeight();
	m->Width = w > 0 ? w : m_pp.BackBufferWidth;
	m->Height = h > 0 ? h : m_pp.BackBufferHeight;
	m->RefreshRate = 60;
	m->Format = D3DFMT_X8R8G8B8;
	return D3D_OK;
}

HRESULT Device::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* p)
{
	if (!p) return D3DERR_INVALIDCALL;
	p->AdapterOrdinal = 0;
	p->DeviceType = D3DDEVTYPE_HAL;
	p->hFocusWindow = m_hwnd;
	p->BehaviorFlags = m_behavior;
	return D3D_OK;
}

HRESULT Device::GetRasterStatus(D3DRASTER_STATUS* p)
{
	if (!p) return D3DERR_INVALIDCALL;
	p->InVBlank = FALSE;
	p->ScanLine = 0;
	return D3D_OK;
}

void Device::SetGammaRamp(DWORD, CONST D3DGAMMARAMP* r)
{
	// Cynex: es3'te donanim gamma yok - sakla, uygulama yok
	if (r) m_gamma = *r;
}

void Device::GetGammaRamp(D3DGAMMARAMP* r)
{
	if (r) *r = m_gamma;
}

// ---------------------------------------------------------------- sunum

void Device::TargetSize(UINT* w, UINT* h) const
{
	if (!m_rt || m_rt == m_backbuffer)
	{
		*w = m_pp.BackBufferWidth;
		*h = m_pp.BackBufferHeight;
	}
	else
	{
		*w = m_rt->m_w;
		*h = m_rt->m_h;
	}
}

// AKRAVON: arka tampon ekranin gercek cozunurlugunde tutulur (en fazla 2.5 kat); oyun 1280x720 koordinatlarini gormeye devam eder
static const float MAX_UI_SCALE = 2.5f;

static void PhysicalBackbufferSize(UINT logicalW, UINT logicalH, UINT* w, UINT* h)
{
	UINT sw = (UINT)Egl_SurfaceWidth(), sh = (UINT)Egl_SurfaceHeight();
	const UINT maxW = (UINT)(logicalW * MAX_UI_SCALE), maxH = (UINT)(logicalH * MAX_UI_SCALE);
	if (sw > maxW || sh > maxH)
	{
		const float k = std::min((float)maxW / sw, (float)maxH / sh);
		sw = (UINT)(sw * k);
		sh = (UINT)(sh * k);
	}
	*w = std::max(sw, logicalW);
	*h = std::max(sh, logicalH);
}

float Device::UIScale() const
{
	if (!m_pp.BackBufferHeight || !Egl_HasWindow())
		return 1.0f;
	UINT w, h;
	PhysicalBackbufferSize(m_pp.BackBufferWidth, m_pp.BackBufferHeight, &w, &h);
	return (float)h / (float)m_pp.BackBufferHeight;
}

void Device::EnsureOffscreenBackbuffer()
{
	UINT physW, physH;
	PhysicalBackbufferSize(m_pp.BackBufferWidth, m_pp.BackBufferHeight, &physW, &physH);
	if (m_bbFbo && m_bbW == physW && m_bbH == physH)
		return;
	DestroyOffscreenBackbuffer();
	m_bbW = physW;
	m_bbH = physH;
	// AKRAVON: kenar yumusatma (msaa) - buyuk ekranlarda 2x, digerlerinde 4x
	GLint maxSamples = 0;
	glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
	m_bbSamples = (uint64_t)m_bbW * m_bbH > 2400000ull ? 2 : 4;
	// Cynex: Adreno (tile tabanli) ekran kartlarinda tam ekran MSAA ara hedef gecislerinde cok yavas - dosya ile acilabilir
	{ FILE* f = fopen("akravon_msaa.cfg", "r"); int v = 0; if (f) { if (fscanf(f, "%d", &v) != 1) v = 0; fclose(f); } m_bbSamples = v > 1 ? v : 0; }
	if (m_bbSamples > maxSamples)
		m_bbSamples = maxSamples > 1 ? maxSamples : 0;
	glGenRenderbuffers(1, &m_bbColor);
	glBindRenderbuffer(GL_RENDERBUFFER, m_bbColor);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_bbSamples, GL_RGBA8, m_bbW, m_bbH);
	glGenRenderbuffers(1, &m_bbDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, m_bbDepth);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_bbSamples, GL_DEPTH24_STENCIL8, m_bbW, m_bbH);
	if (m_bbSamples > 1)
	{
		glGenRenderbuffers(1, &m_bbResolveColor);
		glBindRenderbuffer(GL_RENDERBUFFER, m_bbResolveColor);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, m_bbW, m_bbH);
		glGenFramebuffers(1, &m_bbResolveFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_bbResolveFbo);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_bbResolveColor);
	}
	glGenFramebuffers(1, &m_bbFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_bbFbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_bbColor);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_bbDepth);
	GLenum st = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (st != GL_FRAMEBUFFER_COMPLETE)
		CD3D_LOGE("arka tampon fbo eksik 0x%x", st);
	m_gl.fbo = m_bbFbo;
	CD3D_LOGI("ekran disi arka tampon %ux%u msaa=%d", m_bbW, m_bbH, m_bbSamples);
}

void Device::DestroyOffscreenBackbuffer()
{
	if (m_bbFbo) { glDeleteFramebuffers(1, &m_bbFbo); m_bbFbo = 0; }
	if (m_bbColor) { glDeleteRenderbuffers(1, &m_bbColor); m_bbColor = 0; }
	if (m_bbDepth) { glDeleteRenderbuffers(1, &m_bbDepth); m_bbDepth = 0; }
	if (m_bbResolveFbo) { glDeleteFramebuffers(1, &m_bbResolveFbo); m_bbResolveFbo = 0; }
	if (m_bbResolveColor) { glDeleteRenderbuffers(1, &m_bbResolveColor); m_bbResolveColor = 0; }
	m_bbSamples = 0;
	m_gl.fbo = 0xFFFFFFFF;
}

GLuint Device::ResolveBackbuffer()
{
	// AKRAVON: coklu ornekli tampon ayni boyda tek ornekliye cozulur (cevirme/olcekleme sonraki kopyada)
	if (m_bbSamples <= 1 || !m_bbResolveFbo)
		return m_bbFbo;
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_bbFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_bbResolveFbo);
	if (m_gl.scissor) { glDisable(GL_SCISSOR_TEST); m_gl.scissor = false; }
	glBlitFramebuffer(0, 0, m_bbW, m_bbH, 0, 0, m_bbW, m_bbH, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	m_gl.fbo = 0xFFFFFFFF;
	return m_bbResolveFbo;
}

HRESULT Device::Present(CONST RECT*, CONST RECT*, HWND, CONST RGNDATA*)
{
	if (!Egl_IsRenderThread())
		return D3D_OK;
	FlushDeletes();
	if (!Egl_HasWindow())
	{
		++g_frame;
		return D3D_OK;
	}

	if (m_bbOffscreen && m_bbFbo)
	{
		const int sw = Egl_SurfaceWidth(), sh = Egl_SurfaceHeight();
		const GLuint src = ResolveBackbuffer();
		glBindFramebuffer(GL_READ_FRAMEBUFFER, src);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Egl_ScreenFbo());
		if (m_gl.scissor) { glDisable(GL_SCISSOR_TEST); m_gl.scissor = false; }
		const bool same = (int)m_bbW == sw && (int)m_bbH == sh;
		// Cynex: fbo ust satir once tutuluyor - kopyalarken y cevir
		glBlitFramebuffer(0, 0, m_bbW, m_bbH, 0, sh, sw, 0, GL_COLOR_BUFFER_BIT, same ? GL_NEAREST : GL_LINEAR);
		const GLenum att[2] = { GL_DEPTH_STENCIL_ATTACHMENT, GL_COLOR_ATTACHMENT0 };
		glBindFramebuffer(GL_FRAMEBUFFER, m_bbFbo);
		glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, att);
		m_gl.fbo = 0xFFFFFFFF;
	}
	else
	{
		if (m_gl.fbo != Egl_ScreenFbo())
		{
			glBindFramebuffer(GL_FRAMEBUFFER, Egl_ScreenFbo());
			m_gl.fbo = Egl_ScreenFbo();
		}
		const GLenum att[2] = { GL_DEPTH, GL_STENCIL };
		glInvalidateFramebuffer(GL_FRAMEBUFFER, 2, att);
	}

	{
		static uint64_t s_t0 = PerfNow();
		static uint32_t s_f = 0;
		uint64_t tn0 = PerfNow();
		Egl_Swap();
		static uint64_t s_swapNs = 0;
		s_swapNs += PerfNow() - tn0;
		m_vring.EndFrame();
		m_iring.EndFrame();
		++s_f;
		if (tn0 - s_t0 > 3000000000ull)
		{
			CD3D_LOGE("perf gl kare=%u cagri=%u toplam=%llu hazirlik=%llu baglama=%llu esitleme=%llu(%u) itme=%llu(%u) vsenk=%llu(%u) yukleme=%llu(%u,%lluKB) swap=%llu ms", s_f, g_perf.draws,
				(unsigned long long)(g_perf.drawNs / 1000000), (unsigned long long)(g_perf.prepNs / 1000000), (unsigned long long)(g_perf.bindNs / 1000000), (unsigned long long)(g_perf.syncNs / 1000000), g_perf.syncs, (unsigned long long)(g_perf.pushNs / 1000000), g_perf.pushes, (unsigned long long)(g_perf.vsyncNs / 1000000), g_perf.vsyncs, (unsigned long long)(g_perf.upNs / 1000000), g_perf.uploads, (unsigned long long)(g_perf.upBytes / 1024),
				(unsigned long long)(s_swapNs / 1000000));
			g_perf = PerfCtr();
			s_swapNs = 0; s_f = 0; s_t0 = tn0;
		}
	}
	++g_frame;
	return D3D_OK;
}

HRESULT Device::Reset(D3DPRESENT_PARAMETERS* pp)
{
	if (!pp)
		return D3DERR_INVALIDCALL;
	m_pp = *pp;
	FixPP(&m_pp);
	*pp = m_pp;

	if (m_rt && m_rt != m_backbuffer) m_rt->Release();
	if (m_ds && m_ds != m_autoDepth) m_ds->Release();
	m_backbuffer->m_w = m_autoDepth->m_w = m_pp.BackBufferWidth;
	m_backbuffer->m_h = m_autoDepth->m_h = m_pp.BackBufferHeight;
	m_backbuffer->m_fmt = m_pp.BackBufferFormat;
	m_rt = m_backbuffer;
	m_ds = m_pp.EnableAutoDepthStencil ? m_autoDepth : nullptr;

	// Cynex: auto derinlige bagli ozel fbo'lari at (boyut degisti)
	for (size_t i = 0; i < m_fbos.size();)
	{
		if (m_fbos[i].depth == m_autoDepth)
		{
			glDeleteFramebuffers(1, &m_fbos[i].fbo);
			if (m_fbos[i].privDepth) glDeleteRenderbuffers(1, &m_fbos[i].privDepth);
			m_fbos.erase(m_fbos.begin() + i);
		}
		else
			++i;
	}

	SetDefaultStates();
	m_gl.fbo = 0xFFFFFFFF;
	CD3D_LOGI("Reset %ux%u", m_pp.BackBufferWidth, m_pp.BackBufferHeight);
	return D3D_OK;
}

HRESULT Device::GetBackBuffer(UINT, D3DBACKBUFFER_TYPE, IDirect3DSurface8** pp)
{
	if (!pp) return D3DERR_INVALIDCALL;
	*pp = m_backbuffer;
	m_backbuffer->AddRef();
	return D3D_OK;
}

bool Device::ReadBackbuffer(uint8_t* dst, UINT pitch)
{
	const UINT w = m_pp.BackBufferWidth, h = m_pp.BackBufferHeight;
	GLuint fb;
	bool topDown;
	UINT rw = w, rh = h;
	if (m_bbOffscreen && m_bbFbo) { fb = ResolveBackbuffer(); topDown = true; rw = m_bbW; rh = m_bbH; }
	else if (Egl_HasWindow()) { fb = Egl_ScreenFbo(); topDown = false; }
	else return false;

	m_scratch.resize((size_t)rw * rh * 4);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
	glReadPixels(0, 0, rw, rh, GL_RGBA, GL_UNSIGNED_BYTE, m_scratch.data());
	m_gl.fbo = 0xFFFFFFFF;
	m_dirty |= DIRTY_FBO;
	// AKRAVON: fiziksel tampon mantiksal boyuta en yakin komsu ile indirilir
	for (UINT y = 0; y < h; ++y)
	{
		const UINT sy = (UINT)((uint64_t)y * rh / h);
		const uint8_t* s = m_scratch.data() + (size_t)(topDown ? sy : (rh - 1 - sy)) * rw * 4;
		uint8_t* d = dst + (size_t)y * pitch;
		for (UINT x = 0; x < w; ++x)
		{
			const UINT sx = (UINT)((uint64_t)x * rw / w) * 4;
			d[x * 4 + 0] = s[sx + 2];
			d[x * 4 + 1] = s[sx + 1];
			d[x * 4 + 2] = s[sx + 0];
			d[x * 4 + 3] = 255;
		}
	}
	return true;
}

HRESULT Device::GetFrontBuffer(IDirect3DSurface8* pDest)
{
	Surface* s = static_cast<Surface*>(pDest);
	if (!s || !Egl_IsRenderThread())
		return D3DERR_INVALIDCALL;
	UINT pitch;
	uint8_t* d = s->CpuData(&pitch);
	if (!d || s->m_fmt != D3DFMT_A8R8G8B8 || s->m_w < m_pp.BackBufferWidth || s->m_h < m_pp.BackBufferHeight)
		return D3DERR_INVALIDCALL;
	return ReadBackbuffer(d, pitch) ? D3D_OK : D3DERR_INVALIDCALL;
}

// ---------------------------------------------------------------- kaynak yaratma

HRESULT Device::CreateTexture(UINT w, UINT h, UINT levels, DWORD usage, D3DFORMAT fmt, D3DPOOL pool, IDirect3DTexture8** pp)
{
	if (!pp) return D3DERR_INVALIDCALL;
	*pp = NULL;
	GLUploadFmt up;
	const FormatInfo* fi = GetFormatInfo(fmt);
	if (!fi || fi->depth || !GetGLUploadFmt(fmt, (usage & D3DUSAGE_RENDERTARGET) != 0, &up))
	{
		CD3D_LOGW("CreateTexture: desteklenmeyen format %d", (int)fmt);
		return D3DERR_INVALIDCALL;
	}
	if (!w || !h)
		return D3DERR_INVALIDCALL;
	*pp = new Texture(this, w, h, levels, usage, fmt, pool);
	return D3D_OK;
}

HRESULT Device::CreateVertexBuffer(UINT len, DWORD usage, DWORD fvf, D3DPOOL pool, IDirect3DVertexBuffer8** pp)
{
	if (!pp || !len) return D3DERR_INVALIDCALL;
	VertexBuffer* vb = new VertexBuffer(this, fvf);
	if (!vb->m_core.Init(len, usage, pool))
	{
		delete vb;
		*pp = NULL;
		return D3DERR_OUTOFVIDEOMEMORY;
	}
	*pp = vb;
	return D3D_OK;
}

HRESULT Device::CreateIndexBuffer(UINT len, DWORD usage, D3DFORMAT fmt, D3DPOOL pool, IDirect3DIndexBuffer8** pp)
{
	if (!pp || !len || (fmt != D3DFMT_INDEX16 && fmt != D3DFMT_INDEX32)) return D3DERR_INVALIDCALL;
	IndexBuffer* ib = new IndexBuffer(this, fmt);
	if (!ib->m_core.Init(len, usage, pool))
	{
		delete ib;
		*pp = NULL;
		return D3DERR_OUTOFVIDEOMEMORY;
	}
	*pp = ib;
	return D3D_OK;
}

HRESULT Device::CreateRenderTarget(UINT w, UINT h, D3DFORMAT fmt, D3DMULTISAMPLE_TYPE, BOOL, IDirect3DSurface8** pp)
{
	if (!pp || !w || !h) return D3DERR_INVALIDCALL;
	*pp = new Surface(this, SURF_RENDERTARGET, w, h, fmt, D3DPOOL_DEFAULT, D3DUSAGE_RENDERTARGET);
	return D3D_OK;
}

HRESULT Device::CreateDepthStencilSurface(UINT w, UINT h, D3DFORMAT fmt, D3DMULTISAMPLE_TYPE, IDirect3DSurface8** pp)
{
	if (!pp || !w || !h) return D3DERR_INVALIDCALL;
	const FormatInfo* fi = GetFormatInfo(fmt);
	if (!fi || !fi->depth)
		return D3DERR_INVALIDCALL;
	*pp = new Surface(this, SURF_DEPTH, w, h, fmt, D3DPOOL_DEFAULT, D3DUSAGE_DEPTHSTENCIL);
	return D3D_OK;
}

HRESULT Device::CreateImageSurface(UINT w, UINT h, D3DFORMAT fmt, IDirect3DSurface8** pp)
{
	if (!pp || !w || !h || !GetFormatInfo(fmt)) return D3DERR_INVALIDCALL;
	*pp = new Surface(this, SURF_IMAGE, w, h, fmt, D3DPOOL_SYSTEMMEM, 0);
	return D3D_OK;
}

HRESULT Device::CopyRects(IDirect3DSurface8* pSrc, CONST RECT* srcRects, UINT n, IDirect3DSurface8* pDst, CONST POINT* pts)
{
	Surface* src = static_cast<Surface*>(pSrc);
	Surface* dst = static_cast<Surface*>(pDst);
	if (!src || !dst || src->m_fmt != dst->m_fmt)
		return D3DERR_INVALIDCALL;

	std::vector<uint8_t> bb;
	UINT spitch = 0;
	const uint8_t* sdata;
	if (src->m_kind == SURF_BACKBUFFER)
	{
		if (!Egl_IsRenderThread()) return D3DERR_INVALIDCALL;
		spitch = src->m_w * 4;
		bb.resize((size_t)spitch * src->m_h);
		if (!ReadBackbuffer(bb.data(), spitch)) return D3DERR_INVALIDCALL;
		sdata = bb.data();
	}
	else
		sdata = src->CpuData(&spitch);
	UINT dpitch = 0;
	uint8_t* ddata = dst->CpuData(&dpitch);
	if (!sdata || !ddata)
		return D3DERR_INVALIDCALL;

	const FormatInfo* fi = GetFormatInfo(src->m_fmt);
	if (!fi || fi->blockBytes)
		return D3DERR_INVALIDCALL;
	RECT full = { 0, 0, (LONG)src->m_w, (LONG)src->m_h };
	UINT count = (srcRects && n) ? n : 1;
	for (UINT i = 0; i < count; ++i)
	{
		RECT r = (srcRects && n) ? srcRects[i] : full;
		LONG dx = pts ? pts[i].x : r.left, dy = pts ? pts[i].y : r.top;
		LONG w = r.right - r.left, h = r.bottom - r.top;
		if (dx + w > (LONG)dst->m_w) w = dst->m_w - dx;
		if (dy + h > (LONG)dst->m_h) h = dst->m_h - dy;
		for (LONG y = 0; y < h; ++y)
			memcpy(ddata + (dy + y) * dpitch + dx * fi->bpp, sdata + (r.top + y) * spitch + r.left * fi->bpp, w * fi->bpp);
		if (dst->m_kind == SURF_TEXLEVEL)
		{
			RECT dr = { dx, dy, dx + w, dy + h };
			dst->m_container->MarkLevelDirty(dst->m_level, &dr);
		}
	}
	return D3D_OK;
}

HRESULT Device::UpdateTexture(IDirect3DBaseTexture8* pSrc, IDirect3DBaseTexture8* pDst)
{
	Texture* s = static_cast<Texture*>(pSrc);
	Texture* d = static_cast<Texture*>(pDst);
	if (!s || !d || s->Format() != d->Format())
		return D3DERR_INVALIDCALL;
	UINT skip = 0;
	while (skip < s->Levels() && s->Lv(skip).w > d->Width())
		++skip;
	for (UINT i = 0; i < d->Levels() && i + skip < s->Levels(); ++i)
	{
		uint8_t* sd = s->LevelData(i + skip, false);
		if (!sd || s->Lv(i + skip).size != d->Lv(i).size)
			continue;
		memcpy(d->LevelData(i, true), sd, d->Lv(i).size);
		d->MarkLevelDirty(i, nullptr);
	}
	return D3D_OK;
}

// ---------------------------------------------------------------- hedefler

HRESULT Device::SetRenderTarget(IDirect3DSurface8* pRT, IDirect3DSurface8* pDS)
{
	Surface* rt = static_cast<Surface*>(pRT);
	Surface* ds = static_cast<Surface*>(pDS);
	if (rt && rt != m_rt)
	{
		if (rt->m_kind != SURF_BACKBUFFER && rt->m_kind != SURF_RENDERTARGET && !(rt->m_kind == SURF_TEXLEVEL && rt->m_container->IsRenderTarget()))
			return D3DERR_INVALIDCALL;
		rt->AddRef();
		if (m_rt && m_rt != m_backbuffer) m_rt->Release();
		m_rt = rt;
	}
	if (ds != m_ds)
	{
		if (ds) ds->AddRef();
		if (m_ds && m_ds != m_autoDepth) m_ds->Release();
		m_ds = ds;
	}

	// Cynex: d3d8 hedef degisince viewport tum yuzeye doner
	UINT w, h;
	TargetSize(&w, &h);
	m_vp.X = m_vp.Y = 0;
	m_vp.Width = w;
	m_vp.Height = h;
	m_vp.MinZ = 0.0f;
	m_vp.MaxZ = 1.0f;
	m_dirty |= DIRTY_FBO | DIRTY_VIEWPORT | DIRTY_RASTER;
	Bump(SG_MVP);
	Bump(SG_RHW);
	return D3D_OK;
}

HRESULT Device::GetRenderTarget(IDirect3DSurface8** pp)
{
	if (!pp) return D3DERR_INVALIDCALL;
	*pp = m_rt;
	if (m_rt) m_rt->AddRef();
	return m_rt ? D3D_OK : D3DERR_NOTFOUND;
}

HRESULT Device::GetDepthStencilSurface(IDirect3DSurface8** pp)
{
	if (!pp) return D3DERR_INVALIDCALL;
	*pp = m_ds;
	if (!m_ds)
		return D3DERR_NOTFOUND;
	m_ds->AddRef();
	return D3D_OK;
}

void Device::OnTextureDestroyed(Texture* t)
{
	for (int i = 0; i < 8; ++i)
		if (m_tex[i] == t)
		{
			m_tex[i] = nullptr;
			m_dirty |= DIRTY_KEY;
		}
	if (m_rt && m_rt->m_container == t)
	{
		m_rt = m_backbuffer;
		m_dirty |= DIRTY_FBO | DIRTY_VIEWPORT | DIRTY_RASTER;
	}
}

void Device::OnSurfaceDestroyed(Surface* s)
{
	for (size_t i = 0; i < m_fbos.size();)
	{
		if (m_fbos[i].color == s || m_fbos[i].depth == s)
		{
			if (Egl_IsRenderThread())
			{
				glDeleteFramebuffers(1, &m_fbos[i].fbo);
				if (m_fbos[i].privDepth) glDeleteRenderbuffers(1, &m_fbos[i].privDepth);
			}
			else
			{
				QueueDelete(3, m_fbos[i].fbo);
				if (m_fbos[i].privDepth) QueueDelete(2, m_fbos[i].privDepth);
			}
			if (m_gl.fbo == m_fbos[i].fbo)
				m_gl.fbo = 0xFFFFFFFF;
			m_fbos.erase(m_fbos.begin() + i);
		}
		else
			++i;
	}
	if (m_rt == s)
	{
		m_rt = m_backbuffer;
		m_dirty |= DIRTY_FBO | DIRTY_VIEWPORT | DIRTY_RASTER;
	}
	if (m_ds == s)
	{
		m_ds = nullptr;
		m_dirty |= DIRTY_FBO | DIRTY_RASTER;
	}
}

void Device::OnBufferDestroyed(void* b)
{
	for (int i = 0; i < 16; ++i)
		if (m_streams[i].vb == b)
			m_streams[i].vb = nullptr;
	if (m_ib == b)
		m_ib = nullptr;
}

GLuint Device::GetFBO(Surface* color, Surface* depth, bool* hasDepth, bool* hasStencil)
{
	for (FboEntry& f : m_fbos)
		if (f.color == color && f.depth == depth)
		{
			*hasDepth = f.hasDepth;
			*hasStencil = f.hasStencil;
			return f.fbo;
		}

	FboEntry e = { color, depth, 0, 0, false, false };
	glGenFramebuffers(1, &e.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, e.fbo);
	m_gl.fbo = e.fbo;

	if (color->m_kind == SURF_TEXLEVEL)
	{
		Texture* t = color->m_container;
		if (!t->Sync() || !t->GL())
		{
			glDeleteFramebuffers(1, &e.fbo);
			m_gl.fbo = 0xFFFFFFFF;
			return 0;
		}
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, t->GL(), 0);
	}
	else if (color->m_kind == SURF_RENDERTARGET)
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color->Renderbuffer());

	if (depth)
	{
		if (depth->m_kind == SURF_DEPTH)
		{
			GLuint rb = depth->Renderbuffer();
			if (depth->HasStencil() || (depth->m_fmt != D3DFMT_D16 && depth->m_fmt != D3DFMT_D16_LOCKABLE))
			{
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rb);
				e.hasStencil = depth->HasStencil();
			}
			else
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb);
			e.hasDepth = true;
		}
		else
		{
			// Cynex: arka tamponun derinligi fbo'ya baglanamaz - hedef boyutunda ozel derinlik
			glGenRenderbuffers(1, &e.privDepth);
			glBindRenderbuffer(GL_RENDERBUFFER, e.privDepth);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, color->m_w, color->m_h);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, e.privDepth);
			e.hasDepth = e.hasStencil = true;
		}
	}

	GLenum st = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (st != GL_FRAMEBUFFER_COMPLETE)
		CD3D_LOGE("fbo eksik 0x%x (renk %ux%u fmt %d)", st, color->m_w, color->m_h, (int)color->m_fmt);
	m_fbos.push_back(e);
	*hasDepth = e.hasDepth;
	*hasStencil = e.hasStencil;
	return e.fbo;
}

void Device::ApplyFramebuffer()
{
	if (!(m_dirty & DIRTY_FBO) && m_gl.fbo != 0xFFFFFFFF)
		return;
	m_dirty &= ~DIRTY_FBO;
	m_dirty |= DIRTY_VIEWPORT | DIRTY_RASTER;

	GLuint fbo = 0;
	bool flip, hasDepth, hasStencil, alpha;
	UINT w, h;
	float scaleX = 1.0f, scaleY = 1.0f;
	if (!m_rt || m_rt == m_backbuffer)
	{
		if (!Egl_HasWindow())
		{
			m_targetValid = false;
			m_dirty |= DIRTY_FBO;
			return;
		}
		const int sw = Egl_SurfaceWidth(), sh = Egl_SurfaceHeight();
		m_bbOffscreen = (int)m_pp.BackBufferWidth != sw || (int)m_pp.BackBufferHeight != sh;
		if (m_bbOffscreen)
		{
			EnsureOffscreenBackbuffer();
			fbo = m_bbFbo;
			flip = true;
		}
		else
		{
			fbo = Egl_ScreenFbo();
			flip = false;
		}
		hasDepth = hasStencil = m_ds != nullptr;
		alpha = false;
		w = m_pp.BackBufferWidth;
		h = m_pp.BackBufferHeight;
		scaleX = m_bbOffscreen ? (float)m_bbW / (float)w : 1.0f;
		scaleY = m_bbOffscreen ? (float)m_bbH / (float)h : 1.0f;
	}
	else
	{
		fbo = GetFBO(m_rt, m_ds, &hasDepth, &hasStencil);
		if (!fbo)
		{
			m_targetValid = false;
			return;
		}
		flip = true;
		alpha = m_rt->ColorHasAlpha();
		w = m_rt->m_w;
		h = m_rt->m_h;
	}

	if (m_gl.fbo != fbo)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		m_gl.fbo = fbo;
	}
	if (flip != m_flip || w != m_targetW || h != m_targetH)
	{
		Bump(SG_MVP);
		Bump(SG_RHW);
	}
	m_flip = flip;
	m_targetDepth = hasDepth;
	m_targetStencil = hasStencil;
	m_targetAlpha = alpha;
	m_targetW = w;
	m_targetH = h;
	m_vpScaleX = scaleX;
	m_vpScaleY = scaleY;
	m_targetValid = true;
}

HRESULT Device::BeginScene()
{
	m_inScene = true;
	return D3D_OK;
}

HRESULT Device::EndScene()
{
	m_inScene = false;
	return D3D_OK;
}

HRESULT Device::Clear(DWORD count, CONST D3DRECT* rects, DWORD flags, D3DCOLOR color, float z, DWORD stencil)
{
	if (!Egl_IsRenderThread())
		return D3D_OK;
	ApplyFramebuffer();
	if (!m_targetValid)
		return D3D_OK;

	GLbitfield bits = 0;
	if (flags & D3DCLEAR_TARGET) bits |= GL_COLOR_BUFFER_BIT;
	if ((flags & D3DCLEAR_ZBUFFER) && m_targetDepth) bits |= GL_DEPTH_BUFFER_BIT;
	if ((flags & D3DCLEAR_STENCIL) && m_targetStencil) bits |= GL_STENCIL_BUFFER_BIT;
	if (!bits)
		return D3D_OK;

	if (bits & GL_COLOR_BUFFER_BIT)
	{
		float c[4];
		ColorToVec4(color, c);
		if (!m_targetAlpha)
			c[3] = 1.0f;
		glClearColor(c[0], c[1], c[2], c[3]);
		const bool a = true;	// Cynex: alfasiz hedefte de 1 yazilir
		if (!m_gl.colorMask[0] || !m_gl.colorMask[1] || !m_gl.colorMask[2] || m_gl.colorMask[3] != a)
		{
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, a);
			m_gl.colorMask[0] = m_gl.colorMask[1] = m_gl.colorMask[2] = true;
			m_gl.colorMask[3] = a;
		}
	}
	if (bits & GL_DEPTH_BUFFER_BIT)
	{
		glClearDepthf(z);
		if (!m_gl.depthMask) { glDepthMask(GL_TRUE); m_gl.depthMask = true; }
	}
	if (bits & GL_STENCIL_BUFFER_BIT)
	{
		glClearStencil(stencil);
		if (m_gl.swmask != 0xFFFFFFFF) { glStencilMask(0xFFFFFFFF); m_gl.swmask = 0xFFFFFFFF; }
	}

	const LONG tw = m_targetW, th = m_targetH;
	LONG vl = m_vp.X, vt = m_vp.Y, vr = m_vp.X + m_vp.Width, vb = m_vp.Y + m_vp.Height;
	if (vl < 0) vl = 0;
	if (vt < 0) vt = 0;
	if (vr > tw) vr = tw;
	if (vb > th) vb = th;

	const UINT n = (count && rects) ? count : 1;
	for (UINT i = 0; i < n; ++i)
	{
		LONG l = vl, t = vt, r = vr, b = vb;
		if (count && rects)
		{
			if (rects[i].x1 > l) l = rects[i].x1;
			if (rects[i].y1 > t) t = rects[i].y1;
			if (rects[i].x2 < r) r = rects[i].x2;
			if (rects[i].y2 < b) b = rects[i].y2;
		}
		if (r <= l || b <= t)
			continue;
		if (l == 0 && t == 0 && r == tw && b == th)
		{
			if (m_gl.scissor) { glDisable(GL_SCISSOR_TEST); m_gl.scissor = false; }
		}
		else
		{
			if (!m_gl.scissor) { glEnable(GL_SCISSOR_TEST); m_gl.scissor = true; }
			const GLint sl = (GLint)lroundf(l * m_vpScaleX), sr = (GLint)lroundf(r * m_vpScaleX);
			const GLint st = (GLint)lroundf((m_flip ? t : th - b) * m_vpScaleY);
			const GLint sb = (GLint)lroundf((m_flip ? b : th - t) * m_vpScaleY);
			glScissor(sl, st, sr - sl, sb - st);
		}
		glClear(bits);
	}
	if (m_gl.scissor) { glDisable(GL_SCISSOR_TEST); m_gl.scissor = false; }
	m_dirty |= DIRTY_RASTER;
	return D3D_OK;
}

// ---------------------------------------------------------------- donusumler / malzeme / isik

HRESULT Device::SetTransform(D3DTRANSFORMSTATETYPE st, CONST D3DMATRIX* m)
{
	if (!m) return D3DERR_INVALIDCALL;
	const DWORD s = (DWORD)st;
	if (s == D3DTS_VIEW)
	{
		m_view = *m;
		Bump(SG_MVP); Bump(SG_WV); Bump(SG_LIGHTS);
	}
	else if (s == D3DTS_PROJECTION)
	{
		m_proj = *m;
		Bump(SG_MVP);
	}
	else if (s == 256)
	{
		m_world = *m;
		Bump(SG_MVP); Bump(SG_WV);
	}
	else if (s >= D3DTS_TEXTURE0 && s <= D3DTS_TEXTURE7)
	{
		m_texMat[s - D3DTS_TEXTURE0] = *m;
		Bump(SG_TEXMAT);
	}
	else if (s >= 257 && s <= 259)
		m_worldExtra[s - 257] = *m;
	return D3D_OK;
}

HRESULT Device::GetTransform(D3DTRANSFORMSTATETYPE st, D3DMATRIX* m)
{
	if (!m) return D3DERR_INVALIDCALL;
	const DWORD s = (DWORD)st;
	if (s == D3DTS_VIEW) *m = m_view;
	else if (s == D3DTS_PROJECTION) *m = m_proj;
	else if (s == 256) *m = m_world;
	else if (s >= D3DTS_TEXTURE0 && s <= D3DTS_TEXTURE7) *m = m_texMat[s - D3DTS_TEXTURE0];
	else if (s >= 257 && s <= 259) *m = m_worldExtra[s - 257];
	else MatIdentity(*m);
	return D3D_OK;
}

HRESULT Device::MultiplyTransform(D3DTRANSFORMSTATETYPE st, CONST D3DMATRIX* m)
{
	if (!m) return D3DERR_INVALIDCALL;
	D3DMATRIX cur, r;
	GetTransform(st, &cur);
	MatMul(r, *m, cur);
	return SetTransform(st, &r);
}

HRESULT Device::SetViewport(CONST D3DVIEWPORT8* vp)
{
	if (!vp) return D3DERR_INVALIDCALL;
	m_vp = *vp;
	m_dirty |= DIRTY_VIEWPORT;
	Bump(SG_MVP);
	Bump(SG_RHW);
	return D3D_OK;
}

HRESULT Device::GetViewport(D3DVIEWPORT8* vp)
{
	if (!vp) return D3DERR_INVALIDCALL;
	*vp = m_vp;
	return D3D_OK;
}

HRESULT Device::SetMaterial(CONST D3DMATERIAL8* m)
{
	if (!m) return D3DERR_INVALIDCALL;
	m_mat = *m;
	Bump(SG_MATERIAL);
	return D3D_OK;
}

HRESULT Device::GetMaterial(D3DMATERIAL8* m)
{
	if (!m) return D3DERR_INVALIDCALL;
	*m = m_mat;
	return D3D_OK;
}

static void DefaultLight(D3DLIGHT8* l)
{
	memset(l, 0, sizeof(*l));
	l->Type = D3DLIGHT_DIRECTIONAL;
	l->Diffuse.r = l->Diffuse.g = l->Diffuse.b = 1.0f;
	l->Direction.z = 1.0f;
}

HRESULT Device::SetLight(DWORD idx, CONST D3DLIGHT8* l)
{
	if (!l || idx > 4096) return D3DERR_INVALIDCALL;
	if (idx >= m_lights.size())
		m_lights.resize(idx + 1, LightSlot{ D3DLIGHT8(), false, false });
	LightSlot& s = m_lights[idx];
	if (s.enabled && s.valid && s.l.Type != l->Type)
		m_dirty |= DIRTY_LIGHTLIST | DIRTY_KEY;
	s.l = *l;
	s.valid = true;
	Bump(SG_LIGHTS);
	return D3D_OK;
}

HRESULT Device::GetLight(DWORD idx, D3DLIGHT8* l)
{
	if (!l || idx >= m_lights.size() || !m_lights[idx].valid) return D3DERR_INVALIDCALL;
	*l = m_lights[idx].l;
	return D3D_OK;
}

HRESULT Device::LightEnable(DWORD idx, BOOL en)
{
	if (idx > 4096) return D3DERR_INVALIDCALL;
	if (idx >= m_lights.size())
		m_lights.resize(idx + 1, LightSlot{ D3DLIGHT8(), false, false });
	LightSlot& s = m_lights[idx];
	if (!s.valid)
	{
		DefaultLight(&s.l);
		s.valid = true;
	}
	if ((s.enabled != 0) != (en != 0))
	{
		s.enabled = en != 0;
		m_dirty |= DIRTY_LIGHTLIST | DIRTY_KEY;
		Bump(SG_LIGHTS);
	}
	return D3D_OK;
}

HRESULT Device::GetLightEnable(DWORD idx, BOOL* en)
{
	if (!en || idx >= m_lights.size() || !m_lights[idx].valid) return D3DERR_INVALIDCALL;
	*en = m_lights[idx].enabled ? 128 : 0;
	return D3D_OK;
}

void Device::UpdateLightList()
{
	m_numActiveLights = 0;
	for (size_t i = 0; i < m_lights.size() && m_numActiveLights < 8; ++i)
		if (m_lights[i].valid && m_lights[i].enabled)
			m_activeLights[m_numActiveLights++] = (int)i;
	m_dirty &= ~DIRTY_LIGHTLIST;
}

HRESULT Device::SetClipPlane(DWORD idx, CONST float* p)
{
	if (idx >= 6 || !p) return D3DERR_INVALIDCALL;
	memcpy(m_clipPlanes[idx], p, sizeof(float) * 4);
	return D3D_OK;
}

HRESULT Device::GetClipPlane(DWORD idx, float* p)
{
	if (idx >= 6 || !p) return D3DERR_INVALIDCALL;
	memcpy(p, m_clipPlanes[idx], sizeof(float) * 4);
	return D3D_OK;
}

// ---------------------------------------------------------------- durumlar

HRESULT Device::SetRenderState(D3DRENDERSTATETYPE st, DWORD v)
{
	const DWORD s = (DWORD)st;
	if (s >= 256)
		return D3D_OK;
	if (m_rs[s] == v)
		return D3D_OK;
	m_rs[s] = v;
	switch (s)
	{
	case D3DRS_LIGHTING: case D3DRS_SPECULARENABLE: case D3DRS_LOCALVIEWER: case D3DRS_NORMALIZENORMALS:
	case D3DRS_COLORVERTEX: case D3DRS_DIFFUSEMATERIALSOURCE: case D3DRS_SPECULARMATERIALSOURCE:
	case D3DRS_AMBIENTMATERIALSOURCE: case D3DRS_EMISSIVEMATERIALSOURCE: case D3DRS_FOGENABLE:
	case D3DRS_FOGTABLEMODE: case D3DRS_FOGVERTEXMODE: case D3DRS_RANGEFOGENABLE:
	case D3DRS_ALPHATESTENABLE: case D3DRS_ALPHAFUNC:
		m_dirty |= DIRTY_KEY;
		break;
	case D3DRS_ALPHAREF: Bump(SG_ALPHAREF); break;
	case D3DRS_TEXTUREFACTOR: Bump(SG_TFACTOR); break;
	case D3DRS_FOGCOLOR: case D3DRS_FOGSTART: case D3DRS_FOGEND: case D3DRS_FOGDENSITY: Bump(SG_FOG); break;
	case D3DRS_AMBIENT: Bump(SG_MATERIAL); break;
	case D3DRS_ZENABLE: case D3DRS_ZWRITEENABLE: case D3DRS_ZFUNC: case D3DRS_ALPHABLENDENABLE:
	case D3DRS_SRCBLEND: case D3DRS_DESTBLEND: case D3DRS_BLENDOP: case D3DRS_CULLMODE:
	case D3DRS_STENCILENABLE: case D3DRS_STENCILFAIL: case D3DRS_STENCILZFAIL: case D3DRS_STENCILPASS:
	case D3DRS_STENCILFUNC: case D3DRS_STENCILREF: case D3DRS_STENCILMASK: case D3DRS_STENCILWRITEMASK:
	case D3DRS_COLORWRITEENABLE: case D3DRS_ZBIAS:
		m_dirty |= DIRTY_RASTER;
		break;
	default:
		break;
	}
	return D3D_OK;
}

HRESULT Device::GetRenderState(D3DRENDERSTATETYPE st, DWORD* v)
{
	if (!v) return D3DERR_INVALIDCALL;
	*v = (DWORD)st < 256 ? m_rs[st] : 0;
	return D3D_OK;
}

HRESULT Device::GetTexture(DWORD stage, IDirect3DBaseTexture8** pp)
{
	if (!pp) return D3DERR_INVALIDCALL;
	*pp = stage < 8 ? m_tex[stage] : nullptr;
	if (*pp) (*pp)->AddRef();
	return D3D_OK;
}

HRESULT Device::SetTexture(DWORD stage, IDirect3DBaseTexture8* pTex)
{
	if (stage >= 8)
		return D3D_OK;
	Texture* t = static_cast<Texture*>(pTex);
	if (t == m_tex[stage])
		return D3D_OK;
	if ((t != nullptr) != (m_tex[stage] != nullptr))
		m_dirty |= DIRTY_KEY;
	if (t) t->AddRef();
	if (m_tex[stage]) m_tex[stage]->Release();
	m_tex[stage] = t;
	return D3D_OK;
}

HRESULT Device::GetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD* v)
{
	if (!v) return D3DERR_INVALIDCALL;
	*v = (stage < 8 && (DWORD)type < 32) ? m_tss[stage][type] : 0;
	return D3D_OK;
}

HRESULT Device::SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD v)
{
	if (stage >= 8 || (DWORD)type >= 32)
		return D3D_OK;
	if (m_tss[stage][type] == v)
		return D3D_OK;
	m_tss[stage][type] = v;
	switch (type)
	{
	case D3DTSS_COLOROP: case D3DTSS_COLORARG1: case D3DTSS_COLORARG2: case D3DTSS_ALPHAOP:
	case D3DTSS_ALPHAARG1: case D3DTSS_ALPHAARG2: case D3DTSS_COLORARG0: case D3DTSS_ALPHAARG0:
	case D3DTSS_RESULTARG: case D3DTSS_TEXCOORDINDEX: case D3DTSS_TEXTURETRANSFORMFLAGS:
		m_dirty |= DIRTY_KEY;
		break;
	case D3DTSS_ADDRESSU: case D3DTSS_ADDRESSV:
		m_dirty |= DIRTY_KEY | DIRTY_SAMPLERS;
		break;
	case D3DTSS_BORDERCOLOR:
		Bump(SG_BORDER);
		break;
	default:
		m_dirty |= DIRTY_SAMPLERS;
		break;
	}
	return D3D_OK;
}

// ---------------------------------------------------------------- akislar / golgelendiriciler

HRESULT Device::SetStreamSource(UINT n, IDirect3DVertexBuffer8* pVB, UINT stride)
{
	if (n >= 16) return D3DERR_INVALIDCALL;
	VertexBuffer* vb = static_cast<VertexBuffer*>(pVB);
	if (vb) vb->AddRef();
	if (m_streams[n].vb) m_streams[n].vb->Release();
	m_streams[n].vb = vb;
	m_streams[n].stride = stride;
	return D3D_OK;
}

HRESULT Device::GetStreamSource(UINT n, IDirect3DVertexBuffer8** pp, UINT* stride)
{
	if (n >= 16 || !pp || !stride) return D3DERR_INVALIDCALL;
	*pp = m_streams[n].vb;
	*stride = m_streams[n].stride;
	if (*pp) (*pp)->AddRef();
	return D3D_OK;
}

HRESULT Device::SetIndices(IDirect3DIndexBuffer8* pIB, UINT base)
{
	IndexBuffer* ib = static_cast<IndexBuffer*>(pIB);
	if (ib) ib->AddRef();
	if (m_ib) m_ib->Release();
	m_ib = ib;
	m_baseVertex = base;
	return D3D_OK;
}

HRESULT Device::GetIndices(IDirect3DIndexBuffer8** pp, UINT* base)
{
	if (!pp || !base) return D3DERR_INVALIDCALL;
	*pp = m_ib;
	*base = m_baseVertex;
	if (m_ib) m_ib->AddRef();
	return D3D_OK;
}

static bool ParseDecl(const DWORD* tok, VShaderDecl* d)
{
	VLayout& L = d->layout;
	UINT offs[16] = { 0 };
	UINT stream = 0;
	for (int guard = 0; guard < 256; ++guard)
	{
		DWORD t = *tok++;
		d->tokens.push_back(t);
		if (t == D3DVSD_END())
			break;
		DWORD type = (t & D3DVSD_TOKENTYPEMASK) >> D3DVSD_TOKENTYPESHIFT;
		switch (type)
		{
		case D3DVSD_TOKEN_NOP:
			break;
		case D3DVSD_TOKEN_STREAM:
			stream = t & D3DVSD_STREAMNUMBERMASK;
			break;
		case D3DVSD_TOKEN_STREAMDATA:
			if (t & D3DVSD_DATALOADTYPEMASK)
				offs[stream] += ((t & D3DVSD_SKIPCOUNTMASK) >> D3DVSD_SKIPCOUNTSHIFT) * 4;
			else
			{
				DWORD reg = t & D3DVSD_VERTEXREGMASK;
				DWORD dt = (t & D3DVSD_DATATYPEMASK) >> D3DVSD_DATATYPESHIFT;
				static const uint8_t sizes[8] = { 4, 8, 12, 16, 4, 4, 4, 8 };
				static const uint8_t comps[8] = { 1, 2, 3, 4, 4, 4, 2, 4 };
				if (dt > 7)
					return false;
				int attrib = -1;
				if (reg == D3DVSDE_POSITION) attrib = ATTR_POS;
				else if (reg == D3DVSDE_NORMAL) attrib = ATTR_NORMAL;
				else if (reg == D3DVSDE_DIFFUSE) attrib = ATTR_DIFFUSE;
				else if (reg == D3DVSDE_SPECULAR) attrib = ATTR_SPECULAR;
				else if (reg >= D3DVSDE_TEXCOORD0 && reg <= D3DVSDE_TEXCOORD7) attrib = ATTR_TEX0 + (reg - D3DVSDE_TEXCOORD0);
				if (attrib >= 0 && L.count < 16)
				{
					VElem& e = L.el[L.count++];
					e.attrib = (uint8_t)attrib;
					e.stream = (uint8_t)stream;
					e.offset = (uint16_t)offs[stream];
					e.comps = comps[dt];
					e.normalized = dt == D3DVSDT_D3DCOLOR;
					e.type = dt <= D3DVSDT_FLOAT4 ? GL_FLOAT : (dt == D3DVSDT_D3DCOLOR || dt == D3DVSDT_UBYTE4) ? GL_UNSIGNED_BYTE : GL_SHORT;
					L.streamMask |= 1 << stream;
					if (attrib == ATTR_NORMAL) L.hasNormal = 1;
					else if (attrib == ATTR_DIFFUSE) L.hasDiffuse = 1;
					else if (attrib == ATTR_SPECULAR) L.hasSpecular = 1;
					else if (attrib >= ATTR_TEX0) L.texSize[attrib - ATTR_TEX0] = comps[dt];
				}
				offs[stream] += sizes[dt];
			}
			break;
		case D3DVSD_TOKEN_TESSELLATOR:
			break;
		case D3DVSD_TOKEN_CONSTMEM:
		{
			DWORD n = (t & D3DVSD_CONSTCOUNTMASK) >> D3DVSD_CONSTCOUNTSHIFT;
			for (DWORD i = 0; i < n * 4; ++i)
				d->tokens.push_back(*tok++);
			break;
		}
		case D3DVSD_TOKEN_EXT:
		{
			DWORD n = (t & D3DVSD_EXTCOUNTMASK) >> D3DVSD_EXTCOUNTSHIFT;
			for (DWORD i = 0; i < n; ++i)
				d->tokens.push_back(*tok++);
			break;
		}
		default:
			break;
		}
	}
	L.fvfStride = offs[0];
	return true;
}

HRESULT Device::CreateVertexShader(CONST DWORD* decl, CONST DWORD* func, DWORD* pHandle, DWORD)
{
	if (!pHandle || !decl)
		return D3DERR_INVALIDCALL;
	if (func)
	{
		// Cynex: programlanabilir vs yok - istemci yalniz bildirim kullaniyor
		CD3D_LOGW("CreateVertexShader: fonksiyonlu golgelendirici desteklenmiyor");
		*pHandle = 0;
		return D3DERR_INVALIDCALL;
	}
	VShaderDecl* d = new VShaderDecl;
	if (!ParseDecl(decl, d))
	{
		delete d;
		return D3DERR_INVALIDCALL;
	}
	size_t idx = 0;
	for (size_t i = 1; i < m_vsTable.size(); ++i)
		if (!m_vsTable[i]) { idx = i; break; }
	if (!idx)
	{
		idx = m_vsTable.size();
		m_vsTable.push_back(nullptr);
	}
	m_vsTable[idx] = d;
	// Cynex: tek sayi = tutamac, fvf'lerde bit0 hep 0
	*pHandle = (DWORD)((idx << 1) | 1);
	return D3D_OK;
}

HRESULT Device::SetVertexShader(DWORD h)
{
	if (h != m_vs)
	{
		m_vs = h;
		m_dirty |= DIRTY_KEY;
	}
	return D3D_OK;
}

HRESULT Device::GetVertexShader(DWORD* h)
{
	if (!h) return D3DERR_INVALIDCALL;
	*h = m_vs;
	return D3D_OK;
}

HRESULT Device::DeleteVertexShader(DWORD h)
{
	if (!(h & 1))
		return D3DERR_INVALIDCALL;
	size_t idx = h >> 1;
	if (idx == 0 || idx >= m_vsTable.size() || !m_vsTable[idx])
		return D3DERR_INVALIDCALL;
	if (m_lastLayout == &m_vsTable[idx]->layout)
	{
		m_lastLayout = nullptr;
		m_lastLayoutVS = 0xFFFFFFFF;
	}
	delete m_vsTable[idx];
	m_vsTable[idx] = nullptr;
	m_dirty |= DIRTY_KEY;
	return D3D_OK;
}

HRESULT Device::SetVertexShaderConstant(DWORD reg, CONST void* data, DWORD count)
{
	if (!data) return D3DERR_INVALIDCALL;
	if ((reg + count) * 4 > m_vsConst.size())
		m_vsConst.resize((reg + count) * 4, 0.0f);
	memcpy(&m_vsConst[reg * 4], data, count * 16);
	return D3D_OK;
}

HRESULT Device::GetVertexShaderConstant(DWORD reg, void* data, DWORD count)
{
	if (!data || (reg + count) * 4 > m_vsConst.size()) return D3DERR_INVALIDCALL;
	memcpy(data, &m_vsConst[reg * 4], count * 16);
	return D3D_OK;
}

HRESULT Device::GetVertexShaderDeclaration(DWORD h, void* data, DWORD* size)
{
	if (!size || !(h & 1)) return D3DERR_INVALIDCALL;
	size_t idx = h >> 1;
	if (idx >= m_vsTable.size() || !m_vsTable[idx]) return D3DERR_INVALIDCALL;
	DWORD need = (DWORD)(m_vsTable[idx]->tokens.size() * 4);
	if (!data) { *size = need; return D3D_OK; }
	if (*size < need) return D3DERR_MOREDATA;
	memcpy(data, m_vsTable[idx]->tokens.data(), need);
	*size = need;
	return D3D_OK;
}

// ---------------------------------------------------------------- AKRAVON: goruntu filtresi

static GLuint PfCompile(GLenum type, const char* src)
{
	GLuint sh = glCreateShader(type);
	glShaderSource(sh, 1, &src, nullptr);
	glCompileShader(sh);
	GLint ok = 0;
	glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
	if (!ok)
	{
		char log[1024];
		glGetShaderInfoLog(sh, sizeof(log), nullptr, log);
		CD3D_LOGE("goruntu filtresi derlenemedi: %s", log);
		glDeleteShader(sh);
		return 0;
	}
	return sh;
}

static const char* s_pfVS =
	"#version 300 es\n"
	"void main(){vec2 p=vec2(float((gl_VertexID<<1)&2),float(gl_VertexID&2));gl_Position=vec4(p*2.0-1.0,0.0,1.0);}\n";

// u_p: doygunluk, s egrisi, keskinlik, kenar karartma
static const char* s_pfFS =
	"#version 300 es\n"
	"precision mediump float;\n"
	"uniform sampler2D u_t;uniform vec4 u_p;uniform vec3 u_w;uniform vec2 u_s;\n"
	"out vec4 o;\n"
	"vec3 F(ivec2 p,ivec2 m){return texelFetch(u_t,clamp(p,ivec2(0),m),0).rgb;}\n"
	"void main(){\n"
	" ivec2 p=ivec2(gl_FragCoord.xy);ivec2 m=ivec2(u_s)-1;\n"
	" vec3 c=F(p,m);\n"
	" vec3 n=(F(p+ivec2(1,0),m)+F(p-ivec2(1,0),m)+F(p+ivec2(0,1),m)+F(p-ivec2(0,1),m))*0.25;\n"
	" c=clamp(c+(c-n)*u_p.z,0.0,1.0);\n"
	" float l=dot(c,vec3(0.299,0.587,0.114));\n"
	" c=clamp(mix(vec3(l),c,u_p.x),0.0,1.0);\n"
	" c=mix(c,c*c*(3.0-2.0*c),u_p.y);\n"
	" c=clamp(c*u_w,0.0,1.0);\n"
	" vec2 uv=gl_FragCoord.xy/u_s-0.5;uv.x*=u_s.x/u_s.y;\n"
	" c*=1.0-u_p.w*smoothstep(0.45,1.05,length(uv));\n"
	" o=vec4(c,1.0);\n"
	"}\n";

void Device::ApplyPostFilter(int mode)
{
	if (mode <= 0 || m_pfFailed || !Egl_IsRenderThread() || !Egl_HasWindow())
		return;
	if (m_rt && m_rt != m_backbuffer)
		return;
	ApplyFramebuffer();
	if (!m_targetValid)
		return;

	GLuint dstFbo;
	UINT w, h;
	if (m_bbOffscreen && m_bbFbo)
	{
		dstFbo = m_bbFbo;
		w = m_bbW;
		h = m_bbH;
	}
	else
	{
		dstFbo = 0;
		w = (UINT)Egl_SurfaceWidth();
		h = (UINT)Egl_SurfaceHeight();
	}
	if (!w || !h)
		return;

	if (!m_pfProg)
	{
		GLuint vs = PfCompile(GL_VERTEX_SHADER, s_pfVS);
		GLuint fs = PfCompile(GL_FRAGMENT_SHADER, s_pfFS);
		if (!vs || !fs)
		{
			if (vs) glDeleteShader(vs);
			if (fs) glDeleteShader(fs);
			m_pfFailed = true;
			return;
		}
		m_pfProg = glCreateProgram();
		glAttachShader(m_pfProg, vs);
		glAttachShader(m_pfProg, fs);
		glLinkProgram(m_pfProg);
		glDeleteShader(vs);
		glDeleteShader(fs);
		GLint ok = 0;
		glGetProgramiv(m_pfProg, GL_LINK_STATUS, &ok);
		if (!ok)
		{
			CD3D_LOGE("goruntu filtresi baglanamadi");
			glDeleteProgram(m_pfProg);
			m_pfProg = 0;
			m_pfFailed = true;
			return;
		}
		m_pfLocParam = glGetUniformLocation(m_pfProg, "u_p");
		m_pfLocWarm = glGetUniformLocation(m_pfProg, "u_w");
		m_pfLocSize = glGetUniformLocation(m_pfProg, "u_s");
		glUseProgram(m_pfProg);
		glUniform1i(glGetUniformLocation(m_pfProg, "u_t"), 0);
		m_gl.program = m_pfProg;
		glGenVertexArrays(1, &m_pfVao);
	}

	if (!m_pfTex || m_pfW != w || m_pfH != h)
	{
		if (m_pfFbo) { glDeleteFramebuffers(1, &m_pfFbo); m_pfFbo = 0; }
		if (m_pfTex) { glDeleteTextures(1, &m_pfTex); m_pfTex = 0; }
		glGenTextures(1, &m_pfTex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_pfTex);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glGenFramebuffers(1, &m_pfFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_pfFbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pfTex, 0);
		m_pfW = w;
		m_pfH = h;
	}

	// sahne -> doku (coklu ornek burada cozulur)
	if (m_gl.scissor) { glDisable(GL_SCISSOR_TEST); }
	glBindFramebuffer(GL_READ_FRAMEBUFFER, dstFbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_pfFbo);
	glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, dstFbo);
	glViewport(0, 0, w, h);
	if (m_gl.blend) glDisable(GL_BLEND);
	if (m_gl.depthTest) glDisable(GL_DEPTH_TEST);
	if (m_gl.cull) glDisable(GL_CULL_FACE);
	if (m_gl.stencil) glDisable(GL_STENCIL_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glUseProgram(m_pfProg);
	if (mode >= 2)
	{
		glUniform4f(m_pfLocParam, 1.12f, 0.12f, 0.30f, 0.04f);	// AKRAVON: yumusatildi (eskisi 1.32/0.42/0.55/0.22 asiri kontrast/HDR)
		glUniform3f(m_pfLocWarm, 1.02f, 1.00f, 0.97f);
	}
	else
	{
		glUniform4f(m_pfLocParam, 1.04f, 0.06f, 0.18f, 0.00f);	// AKRAVON: dogal = orijinale cok yakin, hafif keskinlik
		glUniform3f(m_pfLocWarm, 1.01f, 1.00f, 0.99f);
	}
	glUniform2f(m_pfLocSize, (float)w, (float)h);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_pfTex);
	glBindSampler(0, 0);
	glBindVertexArray(m_pfVao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(m_vao);

	// d3d tarafinin onbellegiyle gl'yi yeniden esle
	if (m_gl.blend) glEnable(GL_BLEND);
	if (m_gl.depthTest) glEnable(GL_DEPTH_TEST);
	if (m_gl.cull) glEnable(GL_CULL_FACE);
	if (m_gl.stencil) glEnable(GL_STENCIL_TEST);
	if (m_gl.scissor) glEnable(GL_SCISSOR_TEST);
	glColorMask(m_gl.colorMask[0], m_gl.colorMask[1], m_gl.colorMask[2], m_gl.colorMask[3]);
	m_gl.Invalidate();
	m_dirty |= DIRTY_FBO | DIRTY_VIEWPORT | DIRTY_RASTER | DIRTY_SAMPLERS;
}

}	// namespace cd3d

namespace cd3d { extern int g_cynexTextureQuality; }

// AKRAVON: oyun secenekleri - doku kalitesi (0..2) ve fps (30/60); cizim is parcaciginda cagrilir
void CynexGfx_SetTextureQuality(int quality)
{
	quality = quality < 0 ? 0 : (quality > 2 ? 2 : quality);
	cd3d::g_cynexTextureQuality = quality;	// ornekleyici anahtari kaliteyi icerir -> yenisi olusur
}

void CynexGfx_SetFpsLimit(int fps)
{
	cd3d::Egl_SetSwapInterval(fps <= 30 ? 2 : 1);
}

const char* CynexGfx_GpuName() { return cd3d::g_glcaps.renderer; }
int CynexGfx_DisplayWidth() { return cd3d::Egl_HasWindow() ? cd3d::Egl_SurfaceWidth() : 0; }
int CynexGfx_DisplayHeight() { return cd3d::Egl_HasWindow() ? cd3d::Egl_SurfaceHeight() : 0; }

// AKRAVON: istemci tarafindan cagrilir (RenderGame sonu, arayuzden once)
void CynexGfx_PostFilter(int mode)
{
	if (cd3d::g_device)
		cd3d::g_device->ApplyPostFilter(mode);
}

// AKRAVON: yazi tipi dokulari bu oranla buyuk cizilir (mantiksal 1280x720 -> ekran)
float CynexD3D_UIScale()
{
	return cd3d::g_device ? cd3d::g_device->UIScale() : 1.0f;
}

