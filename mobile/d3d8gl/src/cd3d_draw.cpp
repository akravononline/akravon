// Cynex: cizim yolu - yerlesim, anahtar, uniform, gl durumu, akislar
#include "cd3d_device.h"
#include <math.h>
#include <algorithm>

namespace cd3d {

// ---------------------------------------------------------------- fvf

bool BuildFVFLayout(DWORD fvf, VLayout* L)
{
	memset(L, 0, sizeof(*L));
	UINT off = 0;
	auto add = [&](int attrib, int comps, GLenum type, bool norm, UINT bytes) {
		VElem& e = L->el[L->count++];
		e.attrib = (uint8_t)attrib;
		e.stream = 0;
		e.comps = (uint8_t)comps;
		e.normalized = norm;
		e.offset = (uint16_t)off;
		e.type = type;
		off += bytes;
	};
	DWORD pos = fvf & D3DFVF_POSITION_MASK;
	if (pos == D3DFVF_XYZRHW)
	{
		add(ATTR_POS, 4, GL_FLOAT, false, 16);
		L->posRHW = 1;
	}
	else if (pos == D3DFVF_XYZ)
		add(ATTR_POS, 3, GL_FLOAT, false, 12);
	else if (pos >= D3DFVF_XYZB1)
	{
		add(ATTR_POS, 3, GL_FLOAT, false, 12);
		off += ((pos - 4) / 2) * 4;	// Cynex: harman agirliklari atlanir
	}
	if (fvf & D3DFVF_NORMAL)
	{
		add(ATTR_NORMAL, 3, GL_FLOAT, false, 12);
		L->hasNormal = 1;
	}
	if (fvf & D3DFVF_PSIZE)
		off += 4;
	if (fvf & D3DFVF_DIFFUSE)
	{
		add(ATTR_DIFFUSE, 4, GL_UNSIGNED_BYTE, true, 4);
		L->hasDiffuse = 1;
	}
	if (fvf & D3DFVF_SPECULAR)
	{
		add(ATTR_SPECULAR, 4, GL_UNSIGNED_BYTE, true, 4);
		L->hasSpecular = 1;
	}
	UINT ntex = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
	if (ntex > 8) ntex = 8;
	for (UINT i = 0; i < ntex; ++i)
	{
		DWORD f = (fvf >> (16 + i * 2)) & 3;
		int comps = f == D3DFVF_TEXTUREFORMAT1 ? 1 : f == D3DFVF_TEXTUREFORMAT3 ? 3 : f == D3DFVF_TEXTUREFORMAT4 ? 4 : 2;
		add(ATTR_TEX0 + i, comps, GL_FLOAT, false, comps * 4);
		L->texSize[i] = (uint8_t)comps;
	}
	L->fvfStride = off;
	L->streamMask = 1;
	return true;
}

UINT FVFStride(DWORD fvf)
{
	VLayout L;
	BuildFVFLayout(fvf, &L);
	return L.fvfStride;
}

const VLayout* Device::CurrentLayout()
{
	if (m_vs == m_lastLayoutVS && m_lastLayout)
		return m_lastLayout;
	const VLayout* L = nullptr;
	if (m_vs & 1)
	{
		size_t idx = m_vs >> 1;
		if (idx < m_vsTable.size() && m_vsTable[idx])
			L = &m_vsTable[idx]->layout;
	}
	else
	{
		auto it = m_fvfLayouts.find(m_vs);
		if (it == m_fvfLayouts.end())
		{
			VLayout nl;
			BuildFVFLayout(m_vs, &nl);
			it = m_fvfLayouts.emplace(m_vs, nl).first;
		}
		L = &it->second;
	}
	m_lastLayoutVS = m_vs;
	m_lastLayout = L;
	return L;
}

// ---------------------------------------------------------------- anahtar

static bool OpReads(uint8_t op, int arg)
{
	switch (op)
	{
	case D3DTOP_DISABLE: return false;
	case D3DTOP_SELECTARG1: case D3DTOP_PREMODULATE: return arg == 1;
	case D3DTOP_SELECTARG2: return arg == 2;
	case D3DTOP_MULTIPLYADD: case D3DTOP_LERP: return true;
	case D3DTOP_BUMPENVMAP: case D3DTOP_BUMPENVMAPLUMINANCE: return false;
	default: return arg != 0;
	}
}

static uint8_t MatSrc(DWORD v, bool colorVertex, bool hasD, bool hasS)
{
	if (!colorVertex) return 0;
	if (v == D3DMCS_COLOR1) return hasD ? 1 : 0;
	if (v == D3DMCS_COLOR2) return hasS ? 2 : 0;
	return 0;
}

void Device::BuildKey(const VLayout* lay, bool isPoint, FFKey* k)
{
	memset(k, 0, sizeof(*k));
	k->posRHW = lay->posRHW;
	k->hasNormal = lay->hasNormal;
	k->hasDiffuse = lay->hasDiffuse;
	k->hasSpecular = lay->hasSpecular;
	memcpy(k->texSize, lay->texSize, 8);
	k->isPoint = isPoint;

	k->lighting = m_rs[D3DRS_LIGHTING] && !lay->posRHW;
	if (k->lighting)
	{
		k->specularEnable = m_rs[D3DRS_SPECULARENABLE] ? 1 : 0;
		k->localViewer = m_rs[D3DRS_LOCALVIEWER] ? 1 : 0;
		k->normalize = m_rs[D3DRS_NORMALIZENORMALS] ? 1 : 0;
		const bool cv = m_rs[D3DRS_COLORVERTEX] != 0;
		k->diffSrc = MatSrc(m_rs[D3DRS_DIFFUSEMATERIALSOURCE], cv, lay->hasDiffuse, lay->hasSpecular);
		k->ambSrc = MatSrc(m_rs[D3DRS_AMBIENTMATERIALSOURCE], cv, lay->hasDiffuse, lay->hasSpecular);
		k->specSrc = k->specularEnable ? MatSrc(m_rs[D3DRS_SPECULARMATERIALSOURCE], cv, lay->hasDiffuse, lay->hasSpecular) : 0;
		k->emisSrc = MatSrc(m_rs[D3DRS_EMISSIVEMATERIALSOURCE], cv, lay->hasDiffuse, lay->hasSpecular);
		k->numLights = (uint8_t)m_numActiveLights;
		for (int i = 0; i < m_numActiveLights; ++i)
		{
			DWORD t = m_lights[m_activeLights[i]].l.Type;
			k->lightType[i] = (uint8_t)((t >= D3DLIGHT_POINT && t <= D3DLIGHT_DIRECTIONAL) ? t : D3DLIGHT_DIRECTIONAL);
		}
		if (!lay->hasNormal)
			k->localViewer = 0;
	}

	if (m_rs[D3DRS_FOGENABLE])
	{
		DWORD tm = m_rs[D3DRS_FOGTABLEMODE], vm = m_rs[D3DRS_FOGVERTEXMODE];
		if (tm != D3DFOG_NONE && tm <= D3DFOG_LINEAR)
		{
			k->fogSource = FOGSRC_TABLE;
			k->fogMode = (uint8_t)tm;
		}
		else if (!lay->posRHW && vm != D3DFOG_NONE && vm <= D3DFOG_LINEAR)
		{
			k->fogSource = FOGSRC_VERTEX;
			k->fogMode = (uint8_t)vm;
		}
		else
			k->fogSource = FOGSRC_SPECULAR;
		k->rangeFog = (!lay->posRHW && m_rs[D3DRS_RANGEFOGENABLE]) ? 1 : 0;
	}

	if (m_rs[D3DRS_ALPHATESTENABLE])
	{
		DWORD f = m_rs[D3DRS_ALPHAFUNC];
		k->alphaFunc = (uint8_t)((f >= D3DCMP_NEVER && f <= D3DCMP_ALWAYS) ? f : D3DCMP_ALWAYS);
		if (k->alphaFunc == D3DCMP_ALWAYS)
			k->alphaFunc = 0;
	}

	if (m_rs[D3DRS_SPECULARENABLE] && (k->lighting || lay->hasSpecular))
		k->specularAdd = 1;

	int n = 0;
	for (; n < 8; ++n)
	{
		const DWORD* t = m_tss[n];
		DWORD cop = t[D3DTSS_COLOROP];
		if (cop == D3DTOP_DISABLE || cop < D3DTOP_DISABLE || cop > D3DTOP_LERP)
			break;
		FFStage& st = k->st[n];
		st.colorOp = (uint8_t)cop;
		DWORD aop = t[D3DTSS_ALPHAOP];
		st.alphaOp = (uint8_t)((aop >= D3DTOP_DISABLE && aop <= D3DTOP_LERP) ? aop : D3DTOP_DISABLE);
		static const D3DTEXTURESTAGESTATETYPE cargs[3] = { D3DTSS_COLORARG0, D3DTSS_COLORARG1, D3DTSS_COLORARG2 };
		static const D3DTEXTURESTAGESTATETYPE aargs[3] = { D3DTSS_ALPHAARG0, D3DTSS_ALPHAARG1, D3DTSS_ALPHAARG2 };
		bool sample = cop == D3DTOP_BLENDTEXTUREALPHA || cop == D3DTOP_BLENDTEXTUREALPHAPM ||
			st.alphaOp == D3DTOP_BLENDTEXTUREALPHA || st.alphaOp == D3DTOP_BLENDTEXTUREALPHAPM;
		const bool modAlpha = cop >= D3DTOP_MODULATEALPHA_ADDCOLOR && cop <= D3DTOP_MODULATEINVCOLOR_ADDALPHA;
		for (int a = 0; a < 3; ++a)
		{
			if (OpReads(st.colorOp, a))
			{
				st.colorArg[a] = (uint8_t)(t[cargs[a]] & 0x3F);
				if ((st.colorArg[a] & D3DTA_SELECTMASK) == D3DTA_TEXTURE) sample = true;
			}
			if (st.alphaOp != D3DTOP_DISABLE && OpReads(st.alphaOp, a))
			{
				st.alphaArg[a] = (uint8_t)(t[aargs[a]] & 0x3F);
				if ((st.alphaArg[a] & D3DTA_SELECTMASK) == D3DTA_TEXTURE) sample = true;
			}
		}
		(void)modAlpha;
		st.resultTemp = (t[D3DTSS_RESULTARG] & D3DTA_SELECTMASK) == D3DTA_TEMP ? 1 : 0;
		if (sample)
		{
			st.sample = 1;
			st.texBound = m_tex[n] != nullptr;
			if (st.texBound)
			{
				if (t[D3DTSS_ADDRESSU] == D3DTADDRESS_BORDER) st.border |= 1;
				if (t[D3DTSS_ADDRESSV] == D3DTADDRESS_BORDER) st.border |= 2;
				DWORD tci = t[D3DTSS_TEXCOORDINDEX];
				st.tciIndex = (uint8_t)((tci & 0xFFFF) < 8 ? (tci & 0xFFFF) : 7);
				st.tciMode = (uint8_t)((tci >> 16) & 3);
				DWORD ttf = t[D3DTSS_TEXTURETRANSFORMFLAGS];
				st.ttfCount = (uint8_t)((ttf & 0xFF) <= 4 ? (ttf & 0xFF) : 0);
				st.ttfProjected = (st.ttfCount && (ttf & D3DTTFF_PROJECTED)) ? 1 : 0;
			}
		}
	}
	k->numStages = (uint8_t)n;
}

// ---------------------------------------------------------------- gl durum yardimcilari

static GLenum MapBlend(DWORD b)
{
	switch (b)
	{
	case D3DBLEND_ZERO: return GL_ZERO;
	case D3DBLEND_ONE: return GL_ONE;
	case D3DBLEND_SRCCOLOR: return GL_SRC_COLOR;
	case D3DBLEND_INVSRCCOLOR: return GL_ONE_MINUS_SRC_COLOR;
	case D3DBLEND_SRCALPHA: return GL_SRC_ALPHA;
	case D3DBLEND_INVSRCALPHA: return GL_ONE_MINUS_SRC_ALPHA;
	case D3DBLEND_DESTALPHA: return GL_DST_ALPHA;
	case D3DBLEND_INVDESTALPHA: return GL_ONE_MINUS_DST_ALPHA;
	case D3DBLEND_DESTCOLOR: return GL_DST_COLOR;
	case D3DBLEND_INVDESTCOLOR: return GL_ONE_MINUS_DST_COLOR;
	case D3DBLEND_SRCALPHASAT: return GL_SRC_ALPHA_SATURATE;
	default: return GL_ONE;
	}
}

static GLenum MapCmp(DWORD c)
{
	switch (c)
	{
	case D3DCMP_NEVER: return GL_NEVER;
	case D3DCMP_LESS: return GL_LESS;
	case D3DCMP_EQUAL: return GL_EQUAL;
	case D3DCMP_LESSEQUAL: return GL_LEQUAL;
	case D3DCMP_GREATER: return GL_GREATER;
	case D3DCMP_NOTEQUAL: return GL_NOTEQUAL;
	case D3DCMP_GREATEREQUAL: return GL_GEQUAL;
	default: return GL_ALWAYS;
	}
}

static GLenum MapStencilOp(DWORD o)
{
	switch (o)
	{
	case D3DSTENCILOP_ZERO: return GL_ZERO;
	case D3DSTENCILOP_REPLACE: return GL_REPLACE;
	case D3DSTENCILOP_INCRSAT: return GL_INCR;
	case D3DSTENCILOP_DECRSAT: return GL_DECR;
	case D3DSTENCILOP_INVERT: return GL_INVERT;
	case D3DSTENCILOP_INCR: return GL_INCR_WRAP;
	case D3DSTENCILOP_DECR: return GL_DECR_WRAP;
	default: return GL_KEEP;
	}
}

static inline void GLToggle(GLenum cap, bool& cur, bool want)
{
	if (cur != want)
	{
		if (want) glEnable(cap); else glDisable(cap);
		cur = want;
	}
}

void Device::ApplyRaster()
{
	GLCache& c = m_gl;

	// Cynex: harman
	bool blend = m_rs[D3DRS_ALPHABLENDENABLE] != 0;
	GLToggle(GL_BLEND, c.blend, blend);
	if (blend)
	{
		DWORD s = m_rs[D3DRS_SRCBLEND], d = m_rs[D3DRS_DESTBLEND];
		GLenum gs, gd;
		if (s == D3DBLEND_BOTHSRCALPHA) { gs = GL_SRC_ALPHA; gd = GL_ONE_MINUS_SRC_ALPHA; }
		else if (s == D3DBLEND_BOTHINVSRCALPHA) { gs = GL_ONE_MINUS_SRC_ALPHA; gd = GL_SRC_ALPHA; }
		else { gs = MapBlend(s); gd = MapBlend(d); }
		if (gs != c.bsrc || gd != c.bdst)
		{
			glBlendFunc(gs, gd);
			c.bsrc = gs; c.bdst = gd;
		}
		GLenum op;
		switch (m_rs[D3DRS_BLENDOP])
		{
		case D3DBLENDOP_SUBTRACT: op = GL_FUNC_SUBTRACT; break;
		case D3DBLENDOP_REVSUBTRACT: op = GL_FUNC_REVERSE_SUBTRACT; break;
		case D3DBLENDOP_MIN: op = GL_MIN; break;
		case D3DBLENDOP_MAX: op = GL_MAX; break;
		default: op = GL_FUNC_ADD; break;
		}
		if (op != c.bop) { glBlendEquation(op); c.bop = op; }
	}

	// Cynex: derinlik
	bool zt = m_rs[D3DRS_ZENABLE] != D3DZB_FALSE && m_targetDepth;
	GLToggle(GL_DEPTH_TEST, c.depthTest, zt);
	if (zt)
	{
		GLenum f = MapCmp(m_rs[D3DRS_ZFUNC]);
		if (f != c.depthFunc) { glDepthFunc(f); c.depthFunc = f; }
		bool zw = m_rs[D3DRS_ZWRITEENABLE] != 0;
		if (zw != c.depthMask) { glDepthMask(zw ? GL_TRUE : GL_FALSE); c.depthMask = zw; }
	}

	// Cynex: kirpma - d3d gorsel yon, fbo'da y ters oldugundan yon de ters
	DWORD cull = m_rs[D3DRS_CULLMODE];
	bool cullOn = cull == D3DCULL_CW || cull == D3DCULL_CCW;
	GLToggle(GL_CULL_FACE, c.cull, cullOn);
	if (cullOn)
	{
		bool ffCW = (cull == D3DCULL_CCW);
		if (m_flip) ffCW = !ffCW;
		GLenum ff = ffCW ? GL_CW : GL_CCW;
		if (ff != c.frontFace) { glFrontFace(ff); c.frontFace = ff; }
	}

	// Cynex: sablon
	bool st = m_rs[D3DRS_STENCILENABLE] != 0 && m_targetStencil;
	GLToggle(GL_STENCIL_TEST, c.stencil, st);
	if (st)
	{
		GLenum f = MapCmp(m_rs[D3DRS_STENCILFUNC]);
		GLint ref = (GLint)m_rs[D3DRS_STENCILREF];
		GLuint mask = m_rs[D3DRS_STENCILMASK];
		if (f != c.sfunc || ref != c.sref || mask != c.smask)
		{
			glStencilFunc(f, ref, mask);
			c.sfunc = f; c.sref = ref; c.smask = mask;
		}
		GLenum sf = MapStencilOp(m_rs[D3DRS_STENCILFAIL]), szf = MapStencilOp(m_rs[D3DRS_STENCILZFAIL]), sp = MapStencilOp(m_rs[D3DRS_STENCILPASS]);
		if (sf != c.sfail || szf != c.szfail || sp != c.spass)
		{
			glStencilOp(sf, szf, sp);
			c.sfail = sf; c.szfail = szf; c.spass = sp;
		}
		GLuint wm = m_rs[D3DRS_STENCILWRITEMASK];
		if (wm != c.swmask) { glStencilMask(wm); c.swmask = wm; }
	}

	// Cynex: renk maskesi - alfasiz hedefte alfa yazilmaz (pencere opak kalsin)
	DWORD cw = m_rs[D3DRS_COLORWRITEENABLE];
	bool m[4] = { (cw & 1) != 0, (cw & 2) != 0, (cw & 4) != 0, (cw & 8) != 0 && m_targetAlpha };
	if (m[0] != c.colorMask[0] || m[1] != c.colorMask[1] || m[2] != c.colorMask[2] || m[3] != c.colorMask[3])
	{
		glColorMask(m[0], m[1], m[2], m[3]);
		memcpy(c.colorMask, m, sizeof(m));
	}

	DWORD zb = m_rs[D3DRS_ZBIAS];
	bool po = zb != 0;
	GLToggle(GL_POLYGON_OFFSET_FILL, c.polyOffset, po);
	if (po)
	{
		float u = -(float)zb;
		if (u != c.polyUnits) { glPolygonOffset(0.0f, u); c.polyUnits = u; }
	}
	GLToggle(GL_SCISSOR_TEST, c.scissor, false);
}

void Device::ApplyViewport()
{
	// AKRAVON: d3d viewport mantiksal pikselde; gl hedefi fiziksel cozunurlukte olabilir
	const float sx = m_vpScaleX, sy = m_vpScaleY;
	const GLint x = (GLint)lroundf(m_vp.X * sx);
	const GLint w = (GLint)lroundf((m_vp.X + m_vp.Width) * sx) - x;
	const GLint top = m_flip ? (GLint)m_vp.Y : (GLint)m_targetH - (GLint)(m_vp.Y + m_vp.Height);
	const GLint y = (GLint)lroundf(top * sy);
	const GLint h = (GLint)lroundf((top + (GLint)m_vp.Height) * sy) - y;
	if (x != m_gl.vp[0] || y != m_gl.vp[1] || w != m_gl.vp[2] || h != m_gl.vp[3])
	{
		glViewport(x, y, w, h);
		m_gl.vp[0] = x; m_gl.vp[1] = y; m_gl.vp[2] = w; m_gl.vp[3] = h;
	}
	if (m_vp.MinZ != m_gl.depthRange[0] || m_vp.MaxZ != m_gl.depthRange[1])
	{
		glDepthRangef(m_vp.MinZ, m_vp.MaxZ);
		m_gl.depthRange[0] = m_vp.MinZ;
		m_gl.depthRange[1] = m_vp.MaxZ;
	}
}

void Device::BindTextureUnit(int unit, GLuint tex)
{
	if (m_gl.tex[unit] == tex)
		return;
	if (m_gl.activeUnit != unit)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		m_gl.activeUnit = unit;
	}
	glBindTexture(GL_TEXTURE_2D, tex);
	m_gl.tex[unit] = tex;
}

int g_cynexTextureQuality = 2;	// AKRAVON: CynexGfx_SetTextureQuality

GLuint Device::GetSampler(DWORD s)
{
	const DWORD* t = m_tss[s];
	DWORD au = t[D3DTSS_ADDRESSU] & 7, av = t[D3DTSS_ADDRESSV] & 7;
	DWORD mn = t[D3DTSS_MINFILTER] & 7, mg = t[D3DTSS_MAGFILTER] & 7, mp = t[D3DTSS_MIPFILTER] & 7;
	DWORD an = 1;
	if (g_glcaps.aniso && (mn == D3DTEXF_ANISOTROPIC || mg == D3DTEXF_ANISOTROPIC))
	{
		an = t[D3DTSS_MAXANISOTROPY];
		if (an < 1) an = 1;
		if (an > (DWORD)g_glcaps.maxAniso) an = (DWORD)g_glcaps.maxAniso;
		if (an > 16) an = 16;
	}
	// AKRAVON: mipmapli dokularda en az 16x anizotropik suzme - karakter/yer dokulari egik acida bulaniklasmasin
	// AKRAVON: doku kalitesi (oyun secenekleri) 0 dusuk, 1 orta, 2 yuksek
	const int q = g_cynexTextureQuality;
	const DWORD anMax = q >= 2 ? 16 : (q == 1 ? 4 : 1);
	if (g_glcaps.aniso && mp != D3DTEXF_NONE && mn != D3DTEXF_POINT && mn != D3DTEXF_NONE && an < anMax)
		an = (DWORD)std::min((float)anMax, g_glcaps.maxAniso);
	if (an > anMax)
		an = anMax < 1 ? 1 : anMax;
	const float minLod = (mp != D3DTEXF_NONE && q == 0) ? 1.0f : 0.0f;	// dusuk: en buyuk mip atlanir (dunya dokulari yari cozunurluk)
	uint64_t key = au | (av << 3) | (mn << 6) | (mg << 9) | (mp << 12) | ((uint64_t)an << 15) | ((uint64_t)(minLod > 0.0f) << 24);
	auto it = m_samplers.find(key);
	if (it != m_samplers.end())
		return it->second;

	auto wrap = [](DWORD a) -> GLenum {
		switch (a)
		{
		case D3DTADDRESS_MIRROR: case D3DTADDRESS_MIRRORONCE: return GL_MIRRORED_REPEAT;
		case D3DTADDRESS_CLAMP: case D3DTADDRESS_BORDER: return GL_CLAMP_TO_EDGE;
		default: return GL_REPEAT;
		}
	};
	const bool minLin = mn != D3DTEXF_POINT && mn != D3DTEXF_NONE;
	const bool magLin = mg != D3DTEXF_POINT && mg != D3DTEXF_NONE;
	GLenum minf;
	if (mp == D3DTEXF_NONE)
		minf = minLin ? GL_LINEAR : GL_NEAREST;
	else if (mp == D3DTEXF_POINT)
		minf = minLin ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST;
	else
		minf = minLin ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR;

	GLuint smp;
	glGenSamplers(1, &smp);
	glSamplerParameteri(smp, GL_TEXTURE_WRAP_S, wrap(au));
	glSamplerParameteri(smp, GL_TEXTURE_WRAP_T, wrap(av));
	glSamplerParameteri(smp, GL_TEXTURE_MIN_FILTER, minf);
	glSamplerParameteri(smp, GL_TEXTURE_MAG_FILTER, magLin ? GL_LINEAR : GL_NEAREST);
	if (an > 1)
		glSamplerParameterf(smp, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)an);
	if (minLod > 0.0f)
		glSamplerParameterf(smp, GL_TEXTURE_MIN_LOD, minLod);
	m_samplers.emplace(key, smp);
	return smp;
}

void Device::ApplyTextures(const FFKey& k)
{
	for (int s = 0; s < k.numStages; ++s)
	{
		const FFStage& st = k.st[s];
		if (!st.sample || !st.texBound)
			continue;
		Texture* t = m_tex[s];
		GLuint name = 0;
		if (t && t->Sync())
			name = t->GL();
		BindTextureUnit(s, name);
		GLuint smp = GetSampler(s);
		if (m_gl.sampler[s] != smp)
		{
			glBindSampler(s, smp);
			m_gl.sampler[s] = smp;
		}
	}
}

// ---------------------------------------------------------------- uniformlar

static void Inverse3x3T(const D3DMATRIX& m, float out[9])
{
	// Cynex: ust 3x3'un ters-devrigi (normaller icin)
	const float a = m._11, b = m._12, c = m._13;
	const float d = m._21, e = m._22, f = m._23;
	const float g = m._31, h = m._32, i = m._33;
	const float A = e * i - f * h, B = -(d * i - f * g), C = d * h - e * g;
	const float D = -(b * i - c * h), E = a * i - c * g, F = -(a * h - b * g);
	const float G = b * f - c * e, H = -(a * f - c * d), I = a * e - b * d;
	float det = a * A + b * B + c * C;
	float inv = fabsf(det) > 1e-20f ? 1.0f / det : 0.0f;
	// Cynex: ters = kofaktor^T / det, ters-devrik = kofaktor / det (satir duzeni)
	out[0] = A * inv; out[1] = B * inv; out[2] = C * inv;
	out[3] = D * inv; out[4] = E * inv; out[5] = F * inv;
	out[6] = G * inv; out[7] = H * inv; out[8] = I * inv;
}

void Device::UploadUniforms(FFProgram* p, const FFKey& k)
{
	uint32_t* ps = p->serial;

	if (ps[SG_MVP] != m_serial[SG_MVP])
	{
		ps[SG_MVP] = m_serial[SG_MVP];
		if (p->uMVP >= 0)
		{
			if (m_mvpSerial != m_serial[SG_MVP])
			{
				m_mvpSerial = m_serial[SG_MVP];
				D3DMATRIX wvp, post;
				MatMul(m_wv, m_world, m_view);
				MatMul(wvp, m_wv, m_proj);
				// Cynex: d3d z 0..1 -> gl -1..1, yarim piksel kaydirma, fbo'da y ters
				const float s = m_flip ? -1.0f : 1.0f;
				const float iw = m_vp.Width ? 1.0f / m_vp.Width : 0.0f;
				const float ih = m_vp.Height ? 1.0f / m_vp.Height : 0.0f;
				memset(&post, 0, sizeof(post));
				post._11 = 1.0f;
				post._22 = s;
				post._33 = 2.0f;
				post._41 = iw;
				post._42 = -s * ih;
				post._43 = -1.0f;
				post._44 = 1.0f;
				MatMul(m_mvp, wvp, post);
			}
			glUniformMatrix4fv(p->uMVP, 1, GL_FALSE, &m_mvp.m[0][0]);
		}
	}
	if (ps[SG_WV] != m_serial[SG_WV])
	{
		ps[SG_WV] = m_serial[SG_WV];
		if (p->uWV >= 0 || p->uNormalMat >= 0)
		{
			MatMul(m_wv, m_world, m_view);
			if (p->uWV >= 0)
				glUniformMatrix4fv(p->uWV, 1, GL_FALSE, &m_wv.m[0][0]);
			if (p->uNormalMat >= 0)
			{
				Inverse3x3T(m_wv, m_normalMat);
				glUniformMatrix3fv(p->uNormalMat, 1, GL_FALSE, m_normalMat);
			}
		}
	}
	if (ps[SG_RHW] != m_serial[SG_RHW])
	{
		ps[SG_RHW] = m_serial[SG_RHW];
		if (p->uRhw >= 0)
		{
			const float W = m_vp.Width ? (float)m_vp.Width : 1.0f, H = m_vp.Height ? (float)m_vp.Height : 1.0f;
			const float s = m_flip ? -1.0f : 1.0f;
			float v[4] = {
				2.0f / W, -2.0f * m_vp.X / W - 1.0f + 1.0f / W,
				s * (-2.0f / H), s * (1.0f + 2.0f * m_vp.Y / H - 1.0f / H) };
			glUniform4fv(p->uRhw, 1, v);
		}
	}
	if (ps[SG_TEXMAT] != m_serial[SG_TEXMAT])
	{
		ps[SG_TEXMAT] = m_serial[SG_TEXMAT];
		for (int s = 0; s < k.numStages; ++s)
			if (p->uTexMat[s] >= 0)
				glUniformMatrix4fv(p->uTexMat[s], 1, GL_FALSE, &m_texMat[s].m[0][0]);
	}
	if (ps[SG_MATERIAL] != m_serial[SG_MATERIAL])
	{
		ps[SG_MATERIAL] = m_serial[SG_MATERIAL];
		if (p->uMatDiff >= 0) glUniform4fv(p->uMatDiff, 1, &m_mat.Diffuse.r);
		if (p->uMatAmb >= 0) glUniform4fv(p->uMatAmb, 1, &m_mat.Ambient.r);
		if (p->uMatSpec >= 0) glUniform4fv(p->uMatSpec, 1, &m_mat.Specular.r);
		if (p->uMatEmis >= 0) glUniform4fv(p->uMatEmis, 1, &m_mat.Emissive.r);
		if (p->uMatPower >= 0) glUniform1f(p->uMatPower, m_mat.Power);
		if (p->uAmbient >= 0)
		{
			float a[4];
			ColorToVec4(m_rs[D3DRS_AMBIENT], a);
			glUniform4fv(p->uAmbient, 1, a);
		}
	}
	if (ps[SG_LIGHTS] != m_serial[SG_LIGHTS])
	{
		ps[SG_LIGHTS] = m_serial[SG_LIGHTS];
		const int n = k.lighting ? k.numLights : 0;
		if (n > 0 && p->uLDiff >= 0)
		{
			float ld[32], ls[32], la[32], lp[32], lr[32], lt[32], lc[32];
			memset(lc, 0, sizeof(lc));
			for (int i = 0; i < n; ++i)
			{
				const D3DLIGHT8& L = m_lights[m_activeLights[i]].l;
				memcpy(ld + i * 4, &L.Diffuse, 16);
				memcpy(ls + i * 4, &L.Specular, 16);
				memcpy(la + i * 4, &L.Ambient, 16);
				const D3DVECTOR& P = L.Position;
				lp[i * 4 + 0] = P.x * m_view._11 + P.y * m_view._21 + P.z * m_view._31 + m_view._41;
				lp[i * 4 + 1] = P.x * m_view._12 + P.y * m_view._22 + P.z * m_view._32 + m_view._42;
				lp[i * 4 + 2] = P.x * m_view._13 + P.y * m_view._23 + P.z * m_view._33 + m_view._43;
				lp[i * 4 + 3] = 1.0f;
				const D3DVECTOR& D = L.Direction;
				float dx = D.x * m_view._11 + D.y * m_view._21 + D.z * m_view._31;
				float dy = D.x * m_view._12 + D.y * m_view._22 + D.z * m_view._32;
				float dz = D.x * m_view._13 + D.y * m_view._23 + D.z * m_view._33;
				float len = sqrtf(dx * dx + dy * dy + dz * dz);
				if (len > 0.0f) { dx /= len; dy /= len; dz /= len; }
				lr[i * 4 + 0] = dx; lr[i * 4 + 1] = dy; lr[i * 4 + 2] = dz; lr[i * 4 + 3] = 0.0f;
				lt[i * 4 + 0] = L.Attenuation0;
				lt[i * 4 + 1] = L.Attenuation1;
				lt[i * 4 + 2] = L.Attenuation2;
				lt[i * 4 + 3] = L.Range > 0.0f ? L.Range : 3.4e38f;
				if (L.Type == D3DLIGHT_SPOT)
				{
					float ct = cosf(L.Theta * 0.5f), cp = cosf(L.Phi * 0.5f);
					lc[i * 4 + 0] = ct;
					lc[i * 4 + 1] = cp;
					lc[i * 4 + 2] = L.Falloff;
					lc[i * 4 + 3] = (ct - cp) > 1e-6f ? 1.0f / (ct - cp) : 1e6f;
				}
			}
			glUniform4fv(p->uLDiff, n, ld);
			if (p->uLSpec >= 0) glUniform4fv(p->uLSpec, n, ls);
			if (p->uLAmb >= 0) glUniform4fv(p->uLAmb, n, la);
			if (p->uLPos >= 0) glUniform4fv(p->uLPos, n, lp);
			if (p->uLDir >= 0) glUniform4fv(p->uLDir, n, lr);
			if (p->uLAtt >= 0) glUniform4fv(p->uLAtt, n, lt);
			if (p->uLSpot >= 0) glUniform4fv(p->uLSpot, n, lc);
		}
	}
	if (ps[SG_FOG] != m_serial[SG_FOG])
	{
		ps[SG_FOG] = m_serial[SG_FOG];
		if (p->uFog >= 0)
		{
			float st = DwToF(m_rs[D3DRS_FOGSTART]), en = DwToF(m_rs[D3DRS_FOGEND]);
			float range = en - st;
			float v[4] = { st, en, DwToF(m_rs[D3DRS_FOGDENSITY]), fabsf(range) > 1e-20f ? 1.0f / range : 1e20f };
			glUniform4fv(p->uFog, 1, v);
		}
		if (p->uFogColor >= 0)
		{
			float c[4];
			ColorToVec4(m_rs[D3DRS_FOGCOLOR], c);
			glUniform4fv(p->uFogColor, 1, c);
		}
	}
	if (ps[SG_TFACTOR] != m_serial[SG_TFACTOR])
	{
		ps[SG_TFACTOR] = m_serial[SG_TFACTOR];
		if (p->uTF >= 0)
		{
			float c[4];
			ColorToVec4(m_rs[D3DRS_TEXTUREFACTOR], c);
			glUniform4fv(p->uTF, 1, c);
		}
	}
	if (ps[SG_ALPHAREF] != m_serial[SG_ALPHAREF])
	{
		ps[SG_ALPHAREF] = m_serial[SG_ALPHAREF];
		if (p->uAlphaRef >= 0)
			glUniform1f(p->uAlphaRef, (float)(m_rs[D3DRS_ALPHAREF] & 0xFF));
	}
	if (ps[SG_BORDER] != m_serial[SG_BORDER])
	{
		ps[SG_BORDER] = m_serial[SG_BORDER];
		if (p->uBorder >= 0)
		{
			float b[32];
			for (int s = 0; s < 8; ++s)
				ColorToVec4(m_tss[s][D3DTSS_BORDERCOLOR], b + s * 4);
			glUniform4fv(p->uBorder, 8, b);
		}
	}
}

// ---------------------------------------------------------------- hazirlik

bool Device::PrepareDraw(D3DPRIMITIVETYPE type)
{
	PerfScope perfp_(g_perf.prepNs, g_perf.preps);
	if (!Egl_IsRenderThread())
	{
		if (!m_lostLogged)
		{
			CD3D_LOGW("cizim cagrisi cizim ipligi disindan - atlandi");
			m_lostLogged = true;
		}
		return false;
	}
	ApplyFramebuffer();
	if (!m_targetValid)
		return false;

	const VLayout* lay = CurrentLayout();
	if (!lay || !lay->count)
		return false;

	if (m_dirty & DIRTY_LIGHTLIST)
		UpdateLightList();

	const bool isPoint = type == D3DPT_POINTLIST;
	if ((m_dirty & DIRTY_KEY) || !m_prog || isPoint != (m_key.isPoint != 0))
	{
		FFKey nk;
		BuildKey(lay, isPoint, &nk);
		if (!m_prog || memcmp(&nk, &m_key, sizeof(FFKey)) != 0)
		{
			m_key = nk;
			m_prog = m_ffp.Get(nk);
		}
		m_dirty &= ~DIRTY_KEY;
	}
	if (!m_prog || !m_prog->prog)
		return false;
	if (m_gl.program != m_prog->prog)
	{
		glUseProgram(m_prog->prog);
		m_gl.program = m_prog->prog;
	}

	if (m_dirty & DIRTY_VIEWPORT)
	{
		ApplyViewport();
		m_dirty &= ~DIRTY_VIEWPORT;
	}
	if (m_dirty & DIRTY_RASTER)
	{
		ApplyRaster();
		m_dirty &= ~DIRTY_RASTER;
	}
	UploadUniforms(m_prog, m_key);
	ApplyTextures(m_key);
	return true;
}

// ---------------------------------------------------------------- akislar

typedef void (GL_APIENTRY *PFN_BUFFERSTORAGE)(GLenum target, GLsizeiptr size, const void* data, GLbitfield flags);
#define CYNEX_MAP_PERSISTENT 0x0040
#define CYNEX_MAP_COHERENT 0x0080
#define CYNEX_DYNAMIC_STORAGE 0x0100

bool StreamRing::Init(GLenum t, size_t sz)
{
	target = t;
	size = sz;
	pos = 0;
	glGenBuffers(1, &buf);
	glBindBuffer(t, buf);
	// Cynex: Adreno'da glMapBufferRange/BufferSubData cizim basina 0.04-1.5 ms; kalici eslenmis tampon + kare citlari
	if (g_glcaps.bufStorage)
	{
#ifdef __APPLE__
		PFN_BUFFERSTORAGE fn = nullptr;	// Cynex iOS: GL_EXT_buffer_storage yok
#else
		PFN_BUFFERSTORAGE fn = (PFN_BUFFERSTORAGE)eglGetProcAddress("glBufferStorageEXT");
#endif
		if (fn)
		{
			m_segSize = sz;
			const size_t total = sz * kSeg;
			fn(t, (GLsizeiptr)total, nullptr, CYNEX_MAP_PERSISTENT | CYNEX_MAP_COHERENT | GL_MAP_WRITE_BIT | CYNEX_DYNAMIC_STORAGE);
			m_base = (uint8_t*)glMapBufferRange(t, 0, (GLsizeiptr)total, GL_MAP_WRITE_BIT | CYNEX_MAP_PERSISTENT | CYNEX_MAP_COHERENT);
			if (m_base)
			{
				m_persist = true;
				size = total;
				m_seg = 0;
				m_segStart = 0;
				CD3D_LOGI("kalici eslenmis halka %s: %u KB x %d", t == GL_ARRAY_BUFFER ? "tepe" : "indeks", (unsigned)(sz / 1024), kSeg);
				return true;
			}
			glDeleteBuffers(1, &buf);
			glGenBuffers(1, &buf);
			glBindBuffer(t, buf);
		}
	}
	glBufferData(t, size, nullptr, GL_STREAM_DRAW);
	return buf != 0;
}

void StreamRing::EndFrame()
{
	if (!m_persist)
		return;
	m_fence[m_seg] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	m_seg = (m_seg + 1) % kSeg;
	m_segStart = (size_t)m_seg * m_segSize;
	pos = m_segStart;
	if (m_fence[m_seg])
	{
		glClientWaitSync((GLsync)m_fence[m_seg], GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000ull);
		glDeleteSync((GLsync)m_fence[m_seg]);
		m_fence[m_seg] = nullptr;
	}
}

void StreamRing::Destroy()
{
	if (buf) glDeleteBuffers(1, &buf);
	buf = 0;
	free(m_tmp);
	m_tmp = nullptr;
	m_tmpSize = 0;
}

void StreamRing::Bind(GLCache& c)
{
	if (target == GL_ARRAY_BUFFER)
	{
		if (c.arrayBuf != buf) { glBindBuffer(GL_ARRAY_BUFFER, buf); c.arrayBuf = buf; }
	}
	else
	{
		if (c.elemBuf != buf) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf); c.elemBuf = buf; }
	}
}

void* StreamRing::Map(GLCache& c, size_t bytes, size_t align, size_t* outOff)
{
	Bind(c);
	if (m_persist)
	{
		size_t off = (pos + align - 1) & ~(align - 1);
		if (bytes <= m_segSize)
		{
			if (off + bytes > m_segStart + m_segSize)
			{
				// Cynex: bu karenin dilimi doldu - gpu bitene kadar bekle, basa don (nadir)
				glFinish();
				off = m_segStart;
			}
			pos = off + bytes;
			*outOff = off;
			m_mapped = false;
			m_direct = true;
			return m_base + off;
		}
		// Cynex: dilimden buyuk tek parca (cok nadir): gecici bellek + subdata, dilim basina
		glFinish();
		off = m_segStart;
		pos = off;
		*outOff = off;
		m_tmpOff = off;
		m_tmpBytes = bytes < m_segSize ? bytes : m_segSize;
		m_mapped = false;
		m_direct = false;
		if (m_tmpSize < m_tmpBytes)
		{
			m_tmpSize = m_tmpBytes < 65536 ? 65536 : m_tmpBytes;
			m_tmp = (uint8_t*)realloc(m_tmp, m_tmpSize);
		}
		return m_tmp;
	}
	size_t off = (pos + align - 1) & ~(align - 1);
	if (bytes > size)
	{
		// Cynex: buyut
		while (size < bytes) size *= 2;
		glBufferData(target, size, nullptr, GL_STREAM_DRAW);
		off = 0;
	}
	else if (off + bytes > size)
	{
		// Cynex: sarmal - yetim birak, surucu eskiyi cizimler bitene kadar tutar
		glBufferData(target, size, nullptr, GL_STREAM_DRAW);
		off = 0;
	}
	pos = off + bytes;
	*outOff = off;
	m_tmpOff = off;
	m_tmpBytes = bytes;
	if (g_glcaps.useMapRange)
	{
		void* p = glMapBufferRange(target, off, bytes, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
		if (p)
		{
			m_mapped = true;
			return p;
		}
		g_glcaps.useMapRange = false;
		CD3D_LOGW("glMapBufferRange basarisiz, subdata yoluna geciliyor");
	}
	m_mapped = false;
	if (m_tmpSize < bytes)
	{
		m_tmpSize = bytes < 65536 ? 65536 : bytes;
		m_tmp = (uint8_t*)realloc(m_tmp, m_tmpSize);
	}
	return m_tmp;
}

void StreamRing::Unmap()
{
	if (m_persist && m_direct)
		return;
	if (m_mapped)
	{
		glUnmapBuffer(target);
		m_mapped = false;
	}
	else
		glBufferSubData(target, m_tmpOff, m_tmpBytes, m_tmp);
}

size_t StreamRing::Push(GLCache& c, const void* data, size_t bytes, size_t align)
{
	PerfScope perfpush_(g_perf.pushNs, g_perf.pushes);
	size_t off;
	void* p = Map(c, bytes, align, &off);
	memcpy(p, data, bytes);
	Unmap();
	return off;
}

void Device::SetAttrib(int a, GLuint buf, GLintptr off, GLsizei stride, GLenum type, GLint comps, GLboolean norm)
{
	GLCache::Attr& c = m_gl.attr[a];
	if (!(m_gl.attribEnabled & (1u << a)))
	{
		glEnableVertexAttribArray(a);
		m_gl.attribEnabled |= 1u << a;
	}
	if (c.buf == buf && c.off == off && c.stride == stride && c.type == type && c.comps == comps && c.norm == norm)
		return;
	if (m_gl.arrayBuf != buf)
	{
		glBindBuffer(GL_ARRAY_BUFFER, buf);
		m_gl.arrayBuf = buf;
	}
	glVertexAttribPointer(a, comps, type, norm, stride, (const void*)off);
	c.buf = buf; c.off = off; c.stride = stride; c.type = type; c.comps = comps; c.norm = norm;
}

void Device::DisableUnusedAttribs(uint32_t used)
{
	uint32_t off = m_gl.attribEnabled & ~used;
	if (!off)
		return;
	for (int a = 0; a < ATTR_COUNT; ++a)
		if (off & (1u << a))
			glDisableVertexAttribArray(a);
	m_gl.attribEnabled &= used;
}

// Cynex: vbase = gl'deki 0. tepeye karsilik gelen d3d tepe indeksi
bool Device::BindVertexStreams(const VLayout* lay, UINT vbase, UINT vcount, int upStream, const void* upData, UINT upStride)
{
	PerfScope perfb_(g_perf.bindNs, g_perf.binds);
	GLuint sbuf[16];
	GLintptr soff[16];
	GLsizei sstride[16];
	uint32_t done = 0;
	uint32_t used = 0;
	for (int i = 0; i < lay->count; ++i)
	{
		const VElem& e = lay->el[i];
		const int sn = e.stream;
		if (!(done & (1u << sn)))
		{
			if (sn == upStream)
			{
				sstride[sn] = upStride;
				soff[sn] = (GLintptr)m_vring.Push(m_gl, (const uint8_t*)upData + (size_t)vbase * upStride, (size_t)vcount * upStride, 4);
				sbuf[sn] = m_vring.buf;
			}
			else
			{
				Stream& st = m_streams[sn];
				if (!st.vb)
					return false;
				BufferCore& core = st.vb->m_core;
				UINT stride = st.stride ? st.stride : (sn == 0 ? lay->fvfStride : 0);
				if (!stride)
					return false;
				sstride[sn] = stride;
				if (core.Streaming())
				{
					size_t begin = (size_t)vbase * stride;
					size_t bytes = (size_t)vcount * stride;
					if (begin >= core.size)
						return false;
					if (begin + bytes > core.size)
						bytes = core.size - begin;
					soff[sn] = (GLintptr)m_vring.Push(m_gl, core.data + begin, bytes, 4);
					sbuf[sn] = m_vring.buf;
				}
				else
				{
					{ PerfScope perfvs_(g_perf.vsyncNs, g_perf.vsyncs); sbuf[sn] = core.Sync(GL_ARRAY_BUFFER); }
					soff[sn] = (GLintptr)vbase * stride;
				}
			}
			done |= 1u << sn;
		}
		SetAttrib(e.attrib, sbuf[sn], soff[sn] + e.offset, sstride[sn], e.type, e.comps, e.normalized ? GL_TRUE : GL_FALSE);
		used |= 1u << e.attrib;
	}
	DisableUnusedAttribs(used);
	return true;
}

// Cynex: hata denetimi (yalniz debug.cynex.glerror acikken)
static void CheckGL(const char* where, const FFKey& k)
{
	if (!g_glcaps.checkErrors)
		return;
	GLenum e = glGetError();
	static int s_count = 0;
	if (e != GL_NO_ERROR && s_count < 50)
	{
		++s_count;
		CD3D_LOGE("GL hata 0x%x %s (rhw=%d stages=%d light=%d fog=%d)", e, where, k.posRHW, k.numStages, k.lighting, k.fogSource);
	}
}

static GLenum PrimMode(D3DPRIMITIVETYPE t)
{
	switch (t)
	{
	case D3DPT_POINTLIST: return GL_POINTS;
	case D3DPT_LINELIST: return GL_LINES;
	case D3DPT_LINESTRIP: return GL_LINE_STRIP;
	case D3DPT_TRIANGLESTRIP: return GL_TRIANGLE_STRIP;
	case D3DPT_TRIANGLEFAN: return GL_TRIANGLE_FAN;
	default: return GL_TRIANGLES;
	}
}

static UINT VertexCount(D3DPRIMITIVETYPE t, UINT prims)
{
	switch (t)
	{
	case D3DPT_POINTLIST: return prims;
	case D3DPT_LINELIST: return prims * 2;
	case D3DPT_LINESTRIP: return prims + 1;
	case D3DPT_TRIANGLELIST: return prims * 3;
	case D3DPT_TRIANGLESTRIP: case D3DPT_TRIANGLEFAN: return prims + 2;
	default: return 0;
	}
}

// ---------------------------------------------------------------- cizimler

HRESULT Device::DrawPrimitive(D3DPRIMITIVETYPE type, UINT start, UINT prims)
{
	PerfScope perf_(g_perf.drawNs, g_perf.draws);
	UINT n = VertexCount(type, prims);
	if (!n)
		return D3D_OK;
	if (!PrepareDraw(type))
		return D3D_OK;
	if (!BindVertexStreams(m_lastLayout, start, n, -1, nullptr, 0))
		return D3D_OK;
	glDrawArrays(PrimMode(type), 0, n);
	CheckGL("DrawPrimitive", m_key);
	return D3D_OK;
}

HRESULT Device::DrawPrimitiveUP(D3DPRIMITIVETYPE type, UINT prims, CONST void* data, UINT stride)
{
	PerfScope perf_(g_perf.drawNs, g_perf.draws);
	UINT n = VertexCount(type, prims);
	if (n && data && stride && PrepareDraw(type))
	{
		if (BindVertexStreams(m_lastLayout, 0, n, 0, data, stride))
		{
			glDrawArrays(PrimMode(type), 0, n);
			CheckGL("DrawPrimitiveUP", m_key);
		}
	}
	// Cynex: d3d8: UP cizimden sonra akis 0 bosaltilir
	if (m_streams[0].vb) { m_streams[0].vb->Release(); m_streams[0].vb = nullptr; }
	m_streams[0].stride = 0;
	return D3D_OK;
}

template<class T>
static void MinMaxIdx(const T* p, UINT n, UINT* mn, UINT* mx)
{
	UINT a = 0xFFFFFFFFu, b = 0;
	for (UINT i = 0; i < n; ++i)
	{
		UINT v = p[i];
		if (v < a) a = v;
		if (v > b) b = v;
	}
	*mn = a;
	*mx = b;
}

HRESULT Device::DrawIndexedCommon(D3DPRIMITIVETYPE type, UINT minIndex, UINT numVertices, UINT prims,
	const void* cpuIdx, GLuint glIb, size_t glIbOffset, bool idx32, int upStream, const void* upData, UINT upStride)
{
	PerfScope perf_(g_perf.drawNs, g_perf.draws);
	(void)minIndex;
	(void)numVertices;
	const UINT nidx = VertexCount(type, prims);
	if (!nidx || !cpuIdx)
		return D3D_OK;
	if (!PrepareDraw(type))
		return D3D_OK;
	const VLayout* lay = m_lastLayout;
	const GLenum mode = PrimMode(type);

	bool streamed = upStream >= 0 || !glIb;
	if (!streamed)
	{
		for (int i = 0; i < 16; ++i)
			if ((lay->streamMask & (1u << i)) && m_streams[i].vb && m_streams[i].vb->m_core.Streaming())
			{
				streamed = true;
				break;
			}
	}

	if (!streamed)
	{
		// Cynex: her sey kalici - indeksler dogrudan gl tamponundan
		if (!BindVertexStreams(lay, upStream >= 0 ? 0 : m_baseVertex, 0, -1, nullptr, 0))
			return D3D_OK;
		if (m_gl.elemBuf != glIb)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glIb);
			m_gl.elemBuf = glIb;
		}
		glDrawElements(mode, nidx, idx32 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, (const void*)glIbOffset);
		CheckGL("DrawIndexed", m_key);
		return D3D_OK;
	}

	// Cynex: akitilan yol - gercek tepe araligini indekslerden bul
	UINT vmin, vmax;
	if (idx32) MinMaxIdx((const uint32_t*)cpuIdx, nidx, &vmin, &vmax);
	else MinMaxIdx((const uint16_t*)cpuIdx, nidx, &vmin, &vmax);
	if (vmin > vmax)
		return D3D_OK;
	const UINT vcount = vmax - vmin + 1;
	const UINT base = upStream >= 0 ? 0 : m_baseVertex;
	if (!BindVertexStreams(lay, base + vmin, vcount, upStream, upData, upStride))
		return D3D_OK;

	if (vmin == 0 && glIb)
	{
		if (m_gl.elemBuf != glIb)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glIb);
			m_gl.elemBuf = glIb;
		}
		glDrawElements(mode, nidx, idx32 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, (const void*)glIbOffset);
		CheckGL("DrawIndexed", m_key);
		return D3D_OK;
	}

	const bool out32 = vcount > 65536;
	size_t off;
	void* dst = m_iring.Map(m_gl, (size_t)nidx * (out32 ? 4 : 2), 4, &off);
	if (out32)
	{
		uint32_t* d = (uint32_t*)dst;
		if (idx32) { const uint32_t* s = (const uint32_t*)cpuIdx; for (UINT i = 0; i < nidx; ++i) d[i] = s[i] - vmin; }
		else { const uint16_t* s = (const uint16_t*)cpuIdx; for (UINT i = 0; i < nidx; ++i) d[i] = s[i] - vmin; }
	}
	else
	{
		uint16_t* d = (uint16_t*)dst;
		if (idx32) { const uint32_t* s = (const uint32_t*)cpuIdx; for (UINT i = 0; i < nidx; ++i) d[i] = (uint16_t)(s[i] - vmin); }
		else { const uint16_t* s = (const uint16_t*)cpuIdx; for (UINT i = 0; i < nidx; ++i) d[i] = (uint16_t)(s[i] - vmin); }
	}
	m_iring.Unmap();
	glDrawElements(mode, nidx, out32 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, (const void*)off);
	CheckGL("DrawIndexed(akis)", m_key);
	return D3D_OK;
}

HRESULT Device::DrawIndexedPrimitive(D3DPRIMITIVETYPE type, UINT minIndex, UINT numVertices, UINT startIndex, UINT prims)
{
	if (!m_ib)
		return D3DERR_INVALIDCALL;
	BufferCore& ic = m_ib->m_core;
	const bool idx32 = m_ib->m_fmt == D3DFMT_INDEX32;
	const UINT isz = idx32 ? 4 : 2;
	const UINT nidx = VertexCount(type, prims);
	if ((size_t)(startIndex + nidx) * isz > ic.size)
		return D3DERR_INVALIDCALL;
	GLuint glIb = 0;
	if (!ic.Streaming() && Egl_IsRenderThread())
	{
		PerfScope perfs_(g_perf.syncNs, g_perf.syncs);
		glIb = ic.Sync(GL_ELEMENT_ARRAY_BUFFER);
	}
	return DrawIndexedCommon(type, minIndex, numVertices, prims, ic.data + (size_t)startIndex * isz, glIb, (size_t)startIndex * isz, idx32, -1, nullptr, 0);
}

HRESULT Device::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE type, UINT minIndex, UINT numVertices, UINT prims,
	CONST void* idx, D3DFORMAT fmt, CONST void* data, UINT stride)
{
	if (idx && data && stride)
		DrawIndexedCommon(type, minIndex, numVertices, prims, idx, 0, 0, fmt == D3DFMT_INDEX32, 0, data, stride);
	// Cynex: d3d8: UP sonrasi akis 0 ve indeksler bosaltilir
	if (m_streams[0].vb) { m_streams[0].vb->Release(); m_streams[0].vb = nullptr; }
	m_streams[0].stride = 0;
	if (m_ib) { m_ib->Release(); m_ib = nullptr; }
	m_baseVertex = 0;
	return D3D_OK;
}

}	// namespace cd3d
