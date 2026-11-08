// Cynex: doku / yuzey / tampon uygulamalari
#include "cd3d_device.h"

namespace cd3d {

static UINT FullMipCount(UINT w, UINT h)
{
	UINT n = 1, m = w > h ? w : h;
	while (m > 1) { m >>= 1; ++n; }
	return n;
}

// ================================================================ Texture

Texture::Texture(Device* dev, UINT w, UINT h, UINT levels, DWORD usage, D3DFORMAT fmt, D3DPOOL pool)
	: m_dev(dev), m_w(w ? w : 1), m_h(h ? h : 1), m_usage(usage), m_fmt(fmt), m_pool(pool)
{
	UINT full = FullMipCount(m_w, m_h);
	m_levels = (levels == 0 || levels > full) ? full : levels;
	m_lv.resize(m_levels);
	for (UINT i = 0; i < m_levels; ++i)
	{
		Level& L = m_lv[i];
		L.w = m_w >> i; if (!L.w) L.w = 1;
		L.h = m_h >> i; if (!L.h) L.h = 1;
		L.pitch = SurfacePitch(fmt, L.w);
		L.size = SurfaceSize(fmt, L.w, L.h);
	}
	m_isRT = (usage & D3DUSAGE_RENDERTARGET) != 0;
	memset(&m_up, 0, sizeof(m_up));

	if (pool == D3DPOOL_SYSTEMMEM || pool == D3DPOOL_SCRATCH)
		m_keepShadow = true;
	else if (m_levels == 1 && !IsCompressed(fmt) && !m_isRT)
		m_keepShadow = true;

	if (m_isRT)
		m_dirtyAny = true;	// Cynex: ilk bagda gl nesnesi olussun
}

Texture::~Texture()
{
	if (m_dev && m_dev == g_device)
		m_dev->OnTextureDestroyed(this);
	if (m_surfaces)
	{
		for (UINT i = 0; i < m_levels; ++i)
			delete m_surfaces[i];
		delete[] m_surfaces;
	}
	for (Level& L : m_lv)
		free(L.data);
	if (m_gl)
		GLDeleteTexture(m_gl);
}

ULONG Texture::Release()
{
	ULONG r = --m_ref;
	if (r == 0)
		delete this;
	return r;
}

HRESULT Texture::GetDevice(IDirect3DDevice8** pp)
{
	if (!pp) return D3DERR_INVALIDCALL;
	*pp = m_dev;
	if (m_dev) m_dev->AddRef();
	return D3D_OK;
}

HRESULT Texture::GetLevelDesc(UINT level, D3DSURFACE_DESC* d)
{
	if (level >= m_levels || !d)
		return D3DERR_INVALIDCALL;
	d->Format = m_fmt;
	d->Type = D3DRTYPE_SURFACE;
	d->Usage = m_usage;
	d->Pool = m_pool;
	d->Size = m_lv[level].size;
	d->MultiSampleType = D3DMULTISAMPLE_NONE;
	d->Width = m_lv[level].w;
	d->Height = m_lv[level].h;
	return D3D_OK;
}

HRESULT Texture::GetSurfaceLevel(UINT level, IDirect3DSurface8** pp)
{
	if (!pp || level >= m_levels)
		return D3DERR_INVALIDCALL;
	if (!m_surfaces)
	{
		m_surfaces = new Surface*[m_levels];
		memset(m_surfaces, 0, sizeof(Surface*) * m_levels);
	}
	if (!m_surfaces[level])
		m_surfaces[level] = new Surface(this, level);
	*pp = m_surfaces[level];
	AddRef();
	return D3D_OK;
}

uint8_t* Texture::LevelData(UINT level, bool alloc)
{
	Level& L = m_lv[level];
	if (!L.data && alloc)
	{
		L.data = (uint8_t*)calloc(1, L.size ? L.size : 16);
		// Cynex: golge bir kez birakildiysa artik dinamik say, sakla
		if (L.lockTimes > 0)
			m_keepShadow = true;
	}
	return L.data;
}

void Texture::MarkLevelDirty(UINT level, const RECT* r)
{
	Level& L = m_lv[level];
	RECT full = { 0, 0, (LONG)L.w, (LONG)L.h };
	RECT rc = r ? *r : full;
	if (rc.left < 0) rc.left = 0;
	if (rc.top < 0) rc.top = 0;
	if (rc.right > (LONG)L.w) rc.right = L.w;
	if (rc.bottom > (LONG)L.h) rc.bottom = L.h;
	if (!L.dirty)
	{
		L.dirtyRect = rc;
		L.dirty = true;
	}
	else
	{
		if (rc.left < L.dirtyRect.left) L.dirtyRect.left = rc.left;
		if (rc.top < L.dirtyRect.top) L.dirtyRect.top = rc.top;
		if (rc.right > L.dirtyRect.right) L.dirtyRect.right = rc.right;
		if (rc.bottom > L.dirtyRect.bottom) L.dirtyRect.bottom = rc.bottom;
	}
	m_dirtyAny = true;
}

HRESULT Texture::LockRect(UINT level, D3DLOCKED_RECT* out, CONST RECT* rect, DWORD flags)
{
	if (level >= m_levels || !out)
		return D3DERR_INVALIDCALL;
	if (m_isRT)
		return D3DERR_INVALIDCALL;

	Level& L = m_lv[level];
	uint8_t* p = LevelData(level, true);
	if (!p)
		return D3DERR_OUTOFVIDEOMEMORY;

	UINT off = 0;
	if (rect)
	{
		const FormatInfo* fi = GetFormatInfo(m_fmt);
		if (fi && fi->blockBytes)
			off = (rect->top / 4) * L.pitch + (rect->left / 4) * fi->blockBytes;
		else
			off = rect->top * L.pitch + rect->left * (fi ? fi->bpp : 4);
	}
	out->Pitch = (INT)L.pitch;
	out->pBits = p + off;

	if (!(flags & D3DLOCK_READONLY) || !(m_validMask & (1u << level)))
		MarkLevelDirty(level, rect);

	++L.lockCount;
	if (++L.lockTimes > 1 && !m_fromFile)
		m_keepShadow = true;
	return D3D_OK;
}

HRESULT Texture::UnlockRect(UINT level)
{
	if (level >= m_levels)
		return D3DERR_INVALIDCALL;
	if (m_lv[level].lockCount > 0)
		--m_lv[level].lockCount;
	return D3D_OK;
}

HRESULT Texture::AddDirtyRect(CONST RECT* r)
{
	if (m_lv[0].data)
		MarkLevelDirty(0, r);
	return D3D_OK;
}

bool Texture::EnsureGL()
{
	if (m_gl)
		return true;
	if (!Egl_IsRenderThread() || !m_dev)
		return false;

	if (!GetGLUploadFmt(m_fmt, m_isRT, &m_up))
	{
		CD3D_LOGW("desteklenmeyen doku formati %d", (int)m_fmt);
		return false;
	}
	m_convert = m_up.convert;

	// Cynex: dxt1 cpu cozumu: seffaf blok yoksa 565, varsa 5551 (rgba8'in yarisi)
	if (m_convert == CONV_DXT_RGBA8 && m_fmt == D3DFMT_DXT1)
	{
		bool transparent = false;
		for (UINT i = 0; i < m_levels && !transparent; ++i)
			if (m_lv[i].data)
				transparent = DXT1HasTransparency(m_lv[i].data, m_lv[i].w, m_lv[i].h);
		if (transparent)
		{
			m_convert = CONV_DXT1_5551;
			m_up.internalFmt = GL_RGB5_A1; m_up.format = GL_RGBA; m_up.type = GL_UNSIGNED_SHORT_5_5_5_1;
		}
		else
		{
			m_convert = CONV_DXT1_565;
			m_up.internalFmt = GL_RGB565; m_up.format = GL_RGB; m_up.type = GL_UNSIGNED_SHORT_5_6_5;
		}
		m_up.dstBpp = 2;
	}

	if (m_convert == CONV_DXT_RGBA8 && g_glcaps.dxtLowMem)
	{
		m_convert = CONV_DXT_4444;
		m_up.internalFmt = GL_RGBA4; m_up.format = GL_RGBA; m_up.type = GL_UNSIGNED_SHORT_4_4_4_4;
		m_up.dstBpp = 2;
	}

	glGenTextures(1, &m_gl);
	m_dev->BindTextureUnit(0, m_gl);
	m_glLevels = m_isRT ? 1 : (int)m_levels;
	glTexStorage2D(GL_TEXTURE_2D, m_glLevels, m_up.internalFmt, m_w, m_h);
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
		CD3D_LOGW("glTexStorage2D %ux%u fmt=%d hata 0x%x", m_w, m_h, (int)m_fmt, err);

	if (m_up.swizzle[0] != GL_RED) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, m_up.swizzle[0]);
	if (m_up.swizzle[1] != GL_GREEN) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, m_up.swizzle[1]);
	if (m_up.swizzle[2] != GL_BLUE) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, m_up.swizzle[2]);
	if (m_up.swizzle[3] != GL_ALPHA) glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, m_up.swizzle[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	m_maxLevelSet = 0;
	if (m_isRT)
		m_validMask = 1;
	return true;
}

void Texture::UploadLevel(UINT i)
{
	Level& L = m_lv[i];
	PerfScope perf_(g_perf.upNs, g_perf.uploads);
	g_perf.upBytes += L.size;
	if (!L.data || (int)i >= m_glLevels)
	{
		L.dirty = false;
		return;
	}

	const bool first = !(m_validMask & (1u << i));
	RECT rc = L.dirtyRect;
	if (first || IsCompressed(m_fmt))
	{
		rc.left = rc.top = 0;
		rc.right = L.w;
		rc.bottom = L.h;
	}
	const UINT rw = rc.right - rc.left, rh = rc.bottom - rc.top;
	if (!rw || !rh)
	{
		L.dirty = false;
		return;
	}

	if (m_up.compressed)
	{
		glCompressedTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, L.w, L.h, m_up.internalFmt, L.size, L.data);
	}
	else if (m_convert == CONV_NONE)
	{
		const FormatInfo* fi = GetFormatInfo(m_fmt);
		const UINT bpp = fi ? fi->bpp : 4;
		const uint8_t* src = L.data + rc.top * L.pitch + rc.left * bpp;
		if (rw != L.w)
			glPixelStorei(GL_UNPACK_ROW_LENGTH, L.pitch / bpp);
		glTexSubImage2D(GL_TEXTURE_2D, i, rc.left, rc.top, rw, rh, m_up.format, m_up.type, src);
		if (rw != L.w)
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	}
	else if (m_convert == CONV_DXT_RGBA8 || m_convert == CONV_DXT1_565 || m_convert == CONV_DXT1_5551 || m_convert == CONV_DXT_4444)
	{
		static std::vector<uint8_t> tmp;
		tmp.resize((size_t)L.w * L.h * m_up.dstBpp);
		DecodeDXTLevel(m_fmt, L.data, L.w, L.h, tmp.data(), m_convert);
		glTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, L.w, L.h, m_up.format, m_up.type, tmp.data());
	}
	else
	{
		static std::vector<uint8_t> tmp;
		tmp.resize((size_t)rw * rh * m_up.dstBpp);
		const FormatInfo* fi = GetFormatInfo(m_fmt);
		for (UINT y = 0; y < rh; ++y)
		{
			const uint8_t* s = L.data + (rc.top + y) * L.pitch + rc.left * fi->bpp;
			uint8_t* d = tmp.data() + (size_t)y * rw * m_up.dstBpp;
			if (m_convert == CONV_RGB24)
			{
				for (UINT x = 0; x < rw; ++x)
				{
					d[x * 4 + 0] = s[x * 3 + 0];
					d[x * 4 + 1] = s[x * 3 + 1];
					d[x * 4 + 2] = s[x * 3 + 2];
					d[x * 4 + 3] = 255;
				}
			}
			else
			{
				const uint16_t* s16 = (const uint16_t*)s;
				uint16_t* d16 = (uint16_t*)d;
				if (m_convert == CONV_1555)
					for (UINT x = 0; x < rw; ++x) d16[x] = (uint16_t)((s16[x] << 1) | (s16[x] >> 15));
				else
					for (UINT x = 0; x < rw; ++x) d16[x] = (uint16_t)((s16[x] << 1) | 1);
			}
		}
		glTexSubImage2D(GL_TEXTURE_2D, i, rc.left, rc.top, rw, rh, m_up.format, m_up.type, tmp.data());
	}

	m_validMask |= 1u << i;
	L.dirty = false;
}

bool Texture::Sync()
{
	if (!EnsureGL())
		return false;
	if (!m_dirtyAny)
		return true;
	m_dirtyAny = false;
	if (m_isRT)
		return true;

	m_dev->BindTextureUnit(0, m_gl);
	bool level0 = false;
	for (UINT i = 0; i < m_levels; ++i)
	{
		if (m_lv[i].dirty)
		{
			if (i == 0)
				level0 = true;
			UploadLevel(i);
		}
	}

	// Cynex: d3dx dosya dokulari: alt seviyeler gpu'da uretilir
	if (m_autoMip && level0 && m_glLevels > 1)
	{
		// AKRAVON: doku olusturulurken MAX_LEVEL 0 yapiliyor; glGenerateMipmap yalniz base..MAX_LEVEL arasini uretir.
		// Once tum seviyeler acilmazsa alt seviyeler bos kalir (Adreno'da uzaktaki efektler / GM isareti / dusuk doku kalitesinde
		// mini harita kayboluyordu; masaustu surucusu bunu tolere ettigi icin emulatorde gorunmuyordu).
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_glLevels - 1);
		m_maxLevelSet = m_glLevels - 1;
		glGenerateMipmap(GL_TEXTURE_2D);
		m_validMask = (m_glLevels >= 32) ? 0xFFFFFFFFu : ((1u << m_glLevels) - 1);
	}

	int n = 0;
	while (n < m_glLevels && (m_validMask & (1u << n)))
		++n;
	int maxLevel = n > 0 ? n - 1 : 0;
	if (maxLevel != m_maxLevelSet)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, maxLevel);
		m_maxLevelSet = maxLevel;
	}

	if (!m_keepShadow)
		FreeShadows();
	return true;
}

void Texture::FreeShadows()
{
	for (Level& L : m_lv)
	{
		if (L.data && L.lockCount == 0 && !L.dirty)
		{
			free(L.data);
			L.data = nullptr;
		}
	}
}

// ================================================================ Surface

Surface::Surface(Device* dev, SurfKind kind, UINT w, UINT h, D3DFORMAT fmt, D3DPOOL pool, DWORD usage)
	: m_dev(dev), m_kind(kind), m_w(w), m_h(h), m_fmt(fmt), m_pool(pool), m_usage(usage)
{
	if (kind == SURF_IMAGE)
	{
		m_pitch = SurfacePitch(fmt, w);
		m_data = (uint8_t*)calloc(1, SurfaceSize(fmt, w, h) + 16);
	}
}

Surface::Surface(Texture* container, UINT level)
	: m_dev(container->m_dev), m_kind(SURF_TEXLEVEL), m_container(container), m_level(level)
{
	D3DSURFACE_DESC d;
	container->GetLevelDesc(level, &d);
	m_w = d.Width;
	m_h = d.Height;
	m_fmt = d.Format;
	m_pool = d.Pool;
	m_usage = d.Usage;
}

Surface::~Surface()
{
	if (m_dev && m_dev == g_device)
		m_dev->OnSurfaceDestroyed(this);
	if (m_rb)
		GLDeleteRenderbuffer(m_rb);
	free(m_data);
}

ULONG Surface::AddRef()
{
	if (m_container)
		return m_container->AddRef();
	return ++m_ref;
}

ULONG Surface::Release()
{
	if (m_container)
		return m_container->Release();
	if (m_kind == SURF_BACKBUFFER || m_kind == SURF_AUTODEPTH)
	{
		// Cynex: ortuk yuzeyleri cihaz siler
		ULONG r = m_ref.load();
		if (r > 0)
			r = --m_ref;
		return r;
	}
	ULONG r = --m_ref;
	if (r == 0)
		delete this;
	return r;
}

HRESULT Surface::GetDevice(IDirect3DDevice8** pp)
{
	if (!pp) return D3DERR_INVALIDCALL;
	*pp = m_dev;
	if (m_dev) m_dev->AddRef();
	return D3D_OK;
}

HRESULT Surface::GetContainer(REFIID riid, void** pp)
{
	if (!pp) return D3DERR_INVALIDCALL;
	if (m_container)
		return m_container->QueryInterface(riid, pp);
	if (m_dev)
		return m_dev->QueryInterface(riid, pp);
	*pp = NULL;
	return E_NOINTERFACE;
}

HRESULT Surface::GetDesc(D3DSURFACE_DESC* d)
{
	if (!d) return D3DERR_INVALIDCALL;
	d->Format = m_fmt;
	d->Type = D3DRTYPE_SURFACE;
	d->Usage = m_usage;
	d->Pool = m_pool;
	d->Size = SurfaceSize(m_fmt, m_w, m_h);
	d->MultiSampleType = D3DMULTISAMPLE_NONE;
	d->Width = m_w;
	d->Height = m_h;
	return D3D_OK;
}

HRESULT Surface::LockRect(D3DLOCKED_RECT* out, CONST RECT* rect, DWORD flags)
{
	if (!out)
		return D3DERR_INVALIDCALL;
	switch (m_kind)
	{
	case SURF_TEXLEVEL:
		return m_container->LockRect(m_level, out, rect, flags);
	case SURF_IMAGE:
	{
		const FormatInfo* fi = GetFormatInfo(m_fmt);
		UINT off = 0;
		if (rect)
			off = (fi && fi->blockBytes) ? (rect->top / 4) * m_pitch + (rect->left / 4) * fi->blockBytes : rect->top * m_pitch + rect->left * (fi ? fi->bpp : 4);
		out->Pitch = m_pitch;
		out->pBits = m_data + off;
		m_locked = true;
		return D3D_OK;
	}
	case SURF_BACKBUFFER:
	{
		// Cynex: ekran goruntusu - geri oku, salt okunur
		if (!m_dev || !Egl_IsRenderThread())
			return D3DERR_INVALIDCALL;
		m_pitch = m_w * 4;
		free(m_data);
		m_data = (uint8_t*)calloc(1, (size_t)m_pitch * m_h);
		if (!m_data || !m_dev->ReadBackbuffer(m_data, m_pitch))
			return D3DERR_INVALIDCALL;
		UINT off = rect ? rect->top * m_pitch + rect->left * 4 : 0;
		out->Pitch = m_pitch;
		out->pBits = m_data + off;
		m_locked = true;
		return D3D_OK;
	}
	default:
		return D3DERR_INVALIDCALL;
	}
}

HRESULT Surface::UnlockRect()
{
	if (m_kind == SURF_TEXLEVEL)
		return m_container->UnlockRect(m_level);
	if (m_kind == SURF_BACKBUFFER && m_data)
	{
		free(m_data);
		m_data = nullptr;
	}
	m_locked = false;
	return D3D_OK;
}

uint8_t* Surface::CpuData(UINT* pitch)
{
	if (m_kind == SURF_TEXLEVEL)
	{
		if (pitch) *pitch = m_container->Lv(m_level).pitch;
		return m_container->LevelData(m_level, true);
	}
	if (m_kind == SURF_IMAGE)
	{
		if (pitch) *pitch = m_pitch;
		return m_data;
	}
	return nullptr;
}

bool Surface::ColorHasAlpha() const
{
	if (m_kind == SURF_TEXLEVEL)
		return m_container->TargetHasAlpha();
	return m_fmt == D3DFMT_A8R8G8B8 && m_kind != SURF_BACKBUFFER;
}

GLuint Surface::Renderbuffer()
{
	if (m_rb || !Egl_IsRenderThread())
		return m_rb;
	GLenum ifmt;
	if (m_kind == SURF_DEPTH || m_kind == SURF_AUTODEPTH)
		ifmt = (m_fmt == D3DFMT_D16 || m_fmt == D3DFMT_D16_LOCKABLE) ? GL_DEPTH_COMPONENT16 : GL_DEPTH24_STENCIL8;
	else if (m_kind == SURF_RENDERTARGET)
		ifmt = m_fmt == D3DFMT_R5G6B5 ? GL_RGB565 : GL_RGBA8;
	else
		return 0;
	glGenRenderbuffers(1, &m_rb);
	glBindRenderbuffer(GL_RENDERBUFFER, m_rb);
	glRenderbufferStorage(GL_RENDERBUFFER, ifmt, m_w, m_h);
	return m_rb;
}

// ================================================================ tamponlar

bool BufferCore::Init(UINT sz, DWORD u, D3DPOOL p)
{
	size = sz;
	usage = u;
	pool = p;
	data = (uint8_t*)calloc(1, sz ? sz : 4);
	return data != nullptr;
}

void BufferCore::Free()
{
	free(data);
	data = nullptr;
	if (gl)
	{
		GLDeleteBuffer(gl);
		gl = 0;
	}
}

HRESULT BufferCore::Lock(UINT off, UINT len, BYTE** pp, DWORD flags)
{
	if (!pp || !data)
		return D3DERR_INVALIDCALL;
	if (off > size)
		off = size;
	if (len == 0 || off + len > size)
		len = size - off;
	*pp = data + off;
	lockOff = off;
	lockLen = len;
	lockFlags = flags;
	++lockCount;

	// Cynex: readonly ama hic yuklenmemisse d3d de ilk kullanimda yukler
	if (!(flags & D3DLOCK_READONLY) || !uploaded)
	{
		if ((flags & D3DLOCK_DISCARD))
		{
			orphan = true;
			dirtyMin = off;
			dirtyMax = off + len;
		}
		else if (dirtyMax <= dirtyMin)
		{
			dirtyMin = off;
			dirtyMax = off + len;
		}
		else
		{
			if (off < dirtyMin) dirtyMin = off;
			if (off + len > dirtyMax) dirtyMax = off + len;
		}
		if (lastMod != g_frame)
		{
			prevMod = lastMod;
			lastMod = g_frame;
		}
	}
	return D3D_OK;
}

HRESULT BufferCore::Unlock()
{
	if (lockCount > 0)
		--lockCount;
	return D3D_OK;
}

bool BufferCore::Streaming() const
{
	return (uint32_t)(g_frame - lastMod) <= 1u && (uint32_t)(g_frame - prevMod) <= 4u;
}

GLuint BufferCore::Sync(GLenum target)
{
	if (!Egl_IsRenderThread() || !g_device)
		return 0;
	GLCache& c = g_device->Cache();
	if (!gl)
		glGenBuffers(1, &gl);
	if (target == GL_ARRAY_BUFFER)
	{
		if (c.arrayBuf != gl) { glBindBuffer(GL_ARRAY_BUFFER, gl); c.arrayBuf = gl; }
	}
	else
	{
		if (c.elemBuf != gl) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl); c.elemBuf = gl; }
	}
	const GLenum hint = (usage & D3DUSAGE_DYNAMIC) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
	if (!glAlloc)
	{
		glBufferData(target, size, data, hint);
		glAlloc = true;
	}
	else if (dirtyMax > dirtyMin)
	{
		if (orphan)
			glBufferData(target, size, nullptr, hint);
		glBufferSubData(target, dirtyMin, dirtyMax - dirtyMin, data + dirtyMin);
	}
	uploaded = true;
	orphan = false;
	dirtyMin = dirtyMax = 0;
	return gl;
}

VertexBuffer::~VertexBuffer()
{
	if (m_dev && m_dev == g_device)
		m_dev->OnBufferDestroyed(this);
	m_core.Free();
}

ULONG VertexBuffer::Release()
{
	ULONG r = --m_ref;
	if (r == 0)
		delete this;
	return r;
}

HRESULT VertexBuffer::GetDevice(IDirect3DDevice8** pp)
{
	if (!pp) return D3DERR_INVALIDCALL;
	*pp = m_dev;
	if (m_dev) m_dev->AddRef();
	return D3D_OK;
}

HRESULT VertexBuffer::GetDesc(D3DVERTEXBUFFER_DESC* d)
{
	if (!d) return D3DERR_INVALIDCALL;
	d->Format = D3DFMT_VERTEXDATA;
	d->Type = D3DRTYPE_VERTEXBUFFER;
	d->Usage = m_core.usage;
	d->Pool = m_core.pool;
	d->Size = m_core.size;
	d->FVF = m_fvf;
	return D3D_OK;
}

IndexBuffer::~IndexBuffer()
{
	if (m_dev && m_dev == g_device)
		m_dev->OnBufferDestroyed(this);
	m_core.Free();
}

ULONG IndexBuffer::Release()
{
	ULONG r = --m_ref;
	if (r == 0)
		delete this;
	return r;
}

HRESULT IndexBuffer::GetDevice(IDirect3DDevice8** pp)
{
	if (!pp) return D3DERR_INVALIDCALL;
	*pp = m_dev;
	if (m_dev) m_dev->AddRef();
	return D3D_OK;
}

HRESULT IndexBuffer::GetDesc(D3DINDEXBUFFER_DESC* d)
{
	if (!d) return D3DERR_INVALIDCALL;
	d->Format = m_fmt;
	d->Type = D3DRTYPE_INDEXBUFFER;
	d->Usage = m_core.usage;
	d->Pool = m_core.pool;
	d->Size = m_core.size;
	return D3D_OK;
}

}	// namespace cd3d
