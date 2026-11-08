// Cynex: sabit islev hatti glsl uretici + program onbellegi
#include "cd3d_ffp.h"
#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include "CynexPlatform.h"

namespace cd3d {

namespace {

struct Src
{
	std::string s;
	void add(const char* t) { s += t; }
	void addf(const char* fmt, ...) __attribute__((format(printf, 2, 3)))
	{
		char buf[512];
		va_list ap;
		va_start(ap, fmt);
		vsnprintf(buf, sizeof(buf), fmt, ap);
		va_end(ap);
		s += buf;
	}
};

const char* RegName(uint8_t a, int stage)
{
	static char tbuf[8][8];
	switch (a & D3DTA_SELECTMASK)
	{
	case D3DTA_DIFFUSE: return "vD";
	case D3DTA_CURRENT: return "cur";
	case D3DTA_TEXTURE:
		snprintf(tbuf[stage], sizeof(tbuf[stage]), "t%d", stage);
		return tbuf[stage];
	case D3DTA_TFACTOR: return "uTF";
	case D3DTA_SPECULAR: return "vS";
	case D3DTA_TEMP: return "tmp";
	default: return "cur";
	}
}

// Cynex: arguman ifadesi; alpha=true -> skaler, degilse vec3
std::string ArgExpr(uint8_t a, int stage, bool alpha)
{
	std::string r = RegName(a, stage);
	if (alpha || (a & D3DTA_ALPHAREPLICATE))
		r = alpha ? r + ".a" : "vec3(" + r + ".a)";
	else
		r += ".rgb";
	if (a & D3DTA_COMPLEMENT)
		r = (alpha ? "(1.0-" : "(vec3(1.0)-") + r + ")";
	return r;
}

bool OpUsesArg(uint8_t op, int arg)
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

std::string OpExpr(uint8_t op, const std::string& a0, const std::string& a1, const std::string& a2,
	const std::string& a1alpha, int stage, bool alpha)
{
	char t[8];
	snprintf(t, sizeof(t), "t%d", stage);
	const std::string one = alpha ? "1.0" : "vec3(1.0)";
	const std::string half = alpha ? "0.5" : "vec3(0.5)";
	switch (op)
	{
	case D3DTOP_SELECTARG1: return a1;
	case D3DTOP_SELECTARG2: return a2;
	case D3DTOP_MODULATE: return a1 + "*" + a2;
	case D3DTOP_MODULATE2X: return "(" + a1 + "*" + a2 + ")*2.0";
	case D3DTOP_MODULATE4X: return "(" + a1 + "*" + a2 + ")*4.0";
	case D3DTOP_ADD: return a1 + "+" + a2;
	case D3DTOP_ADDSIGNED: return a1 + "+" + a2 + "-" + half;
	case D3DTOP_ADDSIGNED2X: return "(" + a1 + "+" + a2 + "-" + half + ")*2.0";
	case D3DTOP_SUBTRACT: return a1 + "-" + a2;
	case D3DTOP_ADDSMOOTH: return a1 + "+" + a2 + "*(" + one + "-" + a1 + ")";
	case D3DTOP_BLENDDIFFUSEALPHA: return "mix(" + a2 + "," + a1 + ",vD.a)";
	case D3DTOP_BLENDTEXTUREALPHA: return "mix(" + a2 + "," + a1 + "," + t + ".a)";
	case D3DTOP_BLENDFACTORALPHA: return "mix(" + a2 + "," + a1 + ",uTF.a)";
	case D3DTOP_BLENDTEXTUREALPHAPM: return a1 + "+" + a2 + "*(1.0-" + t + ".a)";
	case D3DTOP_BLENDCURRENTALPHA: return "mix(" + a2 + "," + a1 + ",cur.a)";
	case D3DTOP_PREMODULATE: return a1;
	case D3DTOP_MODULATEALPHA_ADDCOLOR: return alpha ? a1 : a1 + "+" + a1alpha + "*" + a2;
	case D3DTOP_MODULATECOLOR_ADDALPHA: return alpha ? a1 : a1 + "*" + a2 + "+vec3(" + a1alpha + ")";
	case D3DTOP_MODULATEINVALPHA_ADDCOLOR: return alpha ? a1 : "(1.0-" + a1alpha + ")*" + a2 + "+" + a1;
	case D3DTOP_MODULATEINVCOLOR_ADDALPHA: return alpha ? a1 : "(vec3(1.0)-" + a1 + ")*" + a2 + "+vec3(" + a1alpha + ")";
	case D3DTOP_DOTPRODUCT3:
		return alpha ? "4.0*((" + a1 + "-0.5)*(" + a2 + "-0.5))" : "vec3(4.0*dot(" + a1 + "-vec3(0.5)," + a2 + "-vec3(0.5)))";
	case D3DTOP_MULTIPLYADD: return a0 + "+" + a1 + "*" + a2;
	case D3DTOP_LERP: return "mix(" + a2 + "," + a1 + "," + a0 + ")";
	default: return alpha ? "cur.a" : "cur.rgb";
	}
}

const char* CmpOp(uint8_t f)
{
	switch (f)
	{
	case D3DCMP_LESS: return "<";
	case D3DCMP_EQUAL: return "==";
	case D3DCMP_LESSEQUAL: return "<=";
	case D3DCMP_GREATER: return ">";
	case D3DCMP_NOTEQUAL: return "!=";
	case D3DCMP_GREATEREQUAL: return ">=";
	default: return nullptr;
	}
}

std::string GenVS(const FFKey& k)
{
	Src o;
	o.add("#version 300 es\nprecision highp float;\n");
	o.add("layout(location=0) in vec4 aPos;\n");
	if (k.hasNormal) o.add("layout(location=1) in vec3 aNrm;\n");
	if (k.hasDiffuse) o.add("layout(location=2) in vec4 aCol0;\n");
	if (k.hasSpecular) o.add("layout(location=3) in vec4 aCol1;\n");
	bool texUsed[8] = { false };
	for (int s = 0; s < k.numStages; ++s)
		if (k.st[s].sample && k.st[s].tciMode == TCI_PASS && k.st[s].tciIndex < 8 && k.texSize[k.st[s].tciIndex])
			texUsed[k.st[s].tciIndex] = true;
	for (int i = 0; i < 8; ++i)
		if (texUsed[i])
			o.addf("layout(location=%d) in vec4 aT%d;\n", 4 + i, i);

	bool needEye = false, needNormal = false;
	if (!k.posRHW)
	{
		if (k.lighting || k.fogSource == FOGSRC_VERTEX || k.fogSource == FOGSRC_TABLE)
			needEye = true;
		if (k.lighting && k.hasNormal)
			needNormal = true;
		for (int s = 0; s < k.numStages; ++s)
		{
			if (!k.st[s].sample) continue;
			if (k.st[s].tciMode != TCI_PASS) needEye = true;
			if (k.st[s].tciMode == TCI_CAMNORMAL || k.st[s].tciMode == TCI_REFLECT) needNormal = true;
		}
		if (needNormal && !k.hasNormal) needNormal = false;
	}

	if (k.posRHW)
		o.add("uniform vec4 uRhw;\n");
	else
		o.add("uniform mat4 uMVP;\n");
	if (needEye) o.add("uniform mat4 uWV;\n");
	if (needNormal) o.add("uniform mat3 uNM;\n");
	for (int s = 0; s < k.numStages; ++s)
		if (k.st[s].sample && k.st[s].ttfCount)
			o.addf("uniform mat4 uTM%d;\n", s);
	const int nl = k.lighting ? k.numLights : 0;
	if (k.lighting)
	{
		o.add("uniform vec4 uMD, uMA, uMS, uME, uAmb;\nuniform float uMP;\n");
		if (nl)
			o.addf("uniform vec4 uLD[%d], uLS[%d], uLA[%d], uLP[%d], uLR[%d], uLT[%d], uLC[%d];\n", nl, nl, nl, nl, nl, nl, nl);
	}
	if (k.fogSource == FOGSRC_VERTEX) o.add("uniform vec4 uFog;\n");

	o.add("out vec4 vD;\nout vec4 vS;\n");
	for (int s = 0; s < k.numStages; ++s)
		if (k.st[s].sample)
			o.addf("out vec4 vT%d;\n", s);
	if (k.fogSource != FOGSRC_NONE) o.add("out float vF;\n");

	o.add("void main(){\n");
	o.add(k.hasDiffuse ? " vec4 c0=aCol0.bgra;\n" : " vec4 c0=vec4(1.0);\n");
	o.add(k.hasSpecular ? " vec4 c1=aCol1.bgra;\n" : " vec4 c1=vec4(0.0);\n");
	if (k.posRHW)
	{
		o.add(" float w=aPos.w!=0.0?1.0/aPos.w:1.0;\n");
		o.add(" gl_Position=vec4((aPos.x*uRhw.x+uRhw.y)*w,(aPos.y*uRhw.z+uRhw.w)*w,(aPos.z*2.0-1.0)*w,w);\n");
	}
	else
	{
		o.add(" vec4 p=vec4(aPos.xyz,1.0);\n gl_Position=uMVP*p;\n");
		if (needEye) o.add(" vec3 ep=(uWV*p).xyz;\n");
		if (needNormal)
		{
			o.add(" vec3 en=uNM*aNrm;\n");
			if (k.normalize) o.add(" en=normalize(en);\n");
		}
		else if (needEye)
			o.add(" vec3 en=vec3(0.0,0.0,1.0);\n");
	}

	if (k.lighting)
	{
		auto src = [](uint8_t s, const char* mat) -> const char* {
			return s == 1 ? "c0" : (s == 2 ? "c1" : mat);
		};
		o.addf(" vec4 md=%s, ma=%s, ms=%s, me=%s;\n", src(k.diffSrc, "uMD"), src(k.ambSrc, "uMA"), src(k.specSrc, "uMS"), src(k.emisSrc, "uME"));
		o.add(" vec3 la=vec3(0.0), ld=vec3(0.0), ls=vec3(0.0);\n");
		if (nl && k.specularEnable)
			o.add(k.localViewer ? " vec3 V=-normalize(ep);\n" : " vec3 V=vec3(0.0,0.0,1.0);\n");
		for (int i = 0; i < nl; ++i)
		{
			o.add(" {\n");
			if (k.lightType[i] == D3DLIGHT_DIRECTIONAL)
				o.addf("  vec3 L=-uLR[%d].xyz; float at=1.0;\n", i);
			else
			{
				o.addf("  vec3 Lv=uLP[%d].xyz-ep; float d=length(Lv); vec3 L=Lv/max(d,1e-6);\n", i);
				o.addf("  float at=d<=uLT[%d].w?1.0/max(uLT[%d].x+uLT[%d].y*d+uLT[%d].z*d*d,1e-6):0.0;\n", i, i, i, i);
				if (k.lightType[i] == D3DLIGHT_SPOT)
				{
					o.addf("  float rho=dot(-L,uLR[%d].xyz);\n", i);
					o.addf("  at*=rho>uLC[%d].x?1.0:(rho<=uLC[%d].y?0.0:pow(max((rho-uLC[%d].y)*uLC[%d].w,0.0),uLC[%d].z));\n", i, i, i, i, i);
				}
			}
			o.add("  float nd=max(dot(en,L),0.0);\n");
			o.addf("  la+=at*uLA[%d].rgb;\n  ld+=at*nd*uLD[%d].rgb;\n", i, i);
			if (k.specularEnable)
				o.addf("  if(nd>0.0){ vec3 H=normalize(L+V); ls+=at*pow(max(dot(en,H),0.0),uMP)*uLS[%d].rgb; }\n", i);
			o.add(" }\n");
		}
		o.add(" vD=clamp(vec4(me.rgb+ma.rgb*uAmb.rgb+ma.rgb*la+md.rgb*ld,md.a),0.0,1.0);\n");
		if (k.specularEnable)
			o.add(" vS=clamp(vec4(ms.rgb*ls,ms.a),0.0,1.0);\n");
		else
			o.add(" vS=vec4(0.0);\n");
	}
	else
		o.add(" vD=c0;\n vS=c1;\n");

	for (int s = 0; s < k.numStages; ++s)
	{
		const FFStage& st = k.st[s];
		if (!st.sample)
			continue;
		o.addf(" { vec4 tc;\n");
		if (k.posRHW && st.tciMode != TCI_PASS)
			o.add("  tc=vec4(0.0,0.0,0.0,1.0);\n");
		else switch (st.tciMode)
		{
		case TCI_CAMPOS: o.add("  tc=vec4(ep,1.0);\n"); break;
		case TCI_CAMNORMAL: o.add("  tc=vec4(en,1.0);\n"); break;
		case TCI_REFLECT: o.add("  tc=vec4(reflect(normalize(ep),en),1.0);\n"); break;
		default:
		{
			int idx = st.tciIndex;
			int sz = idx < 8 ? k.texSize[idx] : 0;
			// Cynex: d3d doku matrisi icin eksik bilesen: ilki 1, kalanlar 0
			if (sz == 1) o.addf("  tc=vec4(aT%d.x,1.0,0.0,0.0);\n", idx);
			else if (sz == 2) o.addf("  tc=vec4(aT%d.xy,1.0,0.0);\n", idx);
			else if (sz == 3) o.addf("  tc=vec4(aT%d.xyz,1.0);\n", idx);
			else if (sz == 4) o.addf("  tc=aT%d;\n", idx);
			else o.add("  tc=vec4(0.0,0.0,0.0,1.0);\n");
			break;
		}
		}
		if (st.ttfCount)
		{
			o.addf("  tc=uTM%d*tc;\n", s);
			if (st.ttfProjected)
			{
				static const char* comp[4] = { "x", "y", "z", "w" };
				o.addf("  vT%d=vec4(tc.xy,0.0,tc.%s);\n", s, comp[(st.ttfCount - 1) & 3]);
			}
			else
				o.addf("  vT%d=vec4(tc.xy,0.0,1.0);\n", s);
		}
		else
			o.addf("  vT%d=vec4(tc.xy,0.0,1.0);\n", s);
		o.add(" }\n");
	}

	if (k.fogSource == FOGSRC_VERTEX)
	{
		o.add(k.rangeFog ? " float fd=length(ep);\n" : " float fd=abs(ep.z);\n");
		if (k.fogMode == D3DFOG_EXP) o.add(" vF=clamp(exp(-uFog.z*fd),0.0,1.0);\n");
		else if (k.fogMode == D3DFOG_EXP2) o.add(" vF=clamp(exp(-(uFog.z*fd)*(uFog.z*fd)),0.0,1.0);\n");
		else o.add(" vF=clamp((uFog.y-fd)*uFog.w,0.0,1.0);\n");
	}
	else if (k.fogSource == FOGSRC_SPECULAR)
		o.add(k.hasSpecular ? " vF=c1.a;\n" : " vF=1.0;\n");
	else if (k.fogSource == FOGSRC_TABLE)
		o.add(k.posRHW ? " vF=w;\n" : (k.rangeFog ? " vF=length(ep);\n" : " vF=abs(ep.z);\n"));

	if (k.isPoint) o.add(" gl_PointSize=1.0;\n");
	o.add("}\n");
	return o.s;
}

std::string GenFS(const FFKey& k)
{
	Src o;
	o.add("#version 300 es\nprecision mediump float;\n");
	o.add("in vec4 vD;\nin vec4 vS;\n");
	bool anyBorder = false;
	for (int s = 0; s < k.numStages; ++s)
		if (k.st[s].sample)
		{
			o.addf("in highp vec4 vT%d;\n", s);
			if (k.st[s].texBound)
				o.addf("uniform sampler2D uS%d;\n", s);
			if (k.st[s].border)
				anyBorder = true;
		}
	if (k.fogSource != FOGSRC_NONE) o.add("in highp float vF;\nuniform vec4 uFogColor;\n");
	if (k.fogSource == FOGSRC_TABLE) o.add("uniform highp vec4 uFog;\n");
	o.add("uniform vec4 uTF;\n");
	if (k.alphaFunc) o.add("uniform float uAR;\n");
	if (anyBorder) o.add("uniform vec4 uBorder[8];\n");
	o.add("out vec4 oC;\n");

	o.add("void main(){\n vec4 cur=vD;\n vec4 tmp=vec4(0.0);\n");
	for (int s = 0; s < k.numStages; ++s)
	{
		const FFStage& st = k.st[s];
		o.addf(" {\n");
		if (st.sample)
		{
			if (st.texBound)
			{
				o.addf("  highp vec2 uv=vT%d.xy/vT%d.w;\n", s, s);
				o.addf("  vec4 t%d=texture(uS%d,uv,-0.5);\n", s, s);	// AKRAVON: mip gecisini biraz keskin tut
				if (st.border)
				{
					std::string cond;
					if (st.border & 1) cond += "uv.x<0.0||uv.x>1.0";
					if (st.border & 2) cond += std::string(cond.empty() ? "" : "||") + "uv.y<0.0||uv.y>1.0";
					o.addf("  if(%s) t%d=uBorder[%d];\n", cond.c_str(), s, s);
				}
			}
			else
				o.addf("  vec4 t%d=vec4(1.0);\n", s);
		}

		std::string ca[3], aa[3];
		for (int i = 0; i < 3; ++i)
		{
			ca[i] = ArgExpr(st.colorArg[i], s, false);
			aa[i] = ArgExpr(st.alphaArg[i], s, true);
		}
		std::string a1alpha = ArgExpr(st.colorArg[1], s, true);
		o.addf("  vec3 rc=clamp(%s,0.0,1.0);\n", OpExpr(st.colorOp, ca[0], ca[1], ca[2], a1alpha, s, false).c_str());
		if (st.colorOp == D3DTOP_DOTPRODUCT3)
			o.add("  float ra=rc.r;\n");
		else if (st.alphaOp == D3DTOP_DISABLE)
			o.add("  float ra=cur.a;\n");
		else
			o.addf("  float ra=clamp(%s,0.0,1.0);\n", OpExpr(st.alphaOp, aa[0], aa[1], aa[2], aa[1], s, true).c_str());
		o.add(st.resultTemp ? "  tmp=vec4(rc,ra);\n" : "  cur=vec4(rc,ra);\n");
		o.add(" }\n");
	}

	if (k.specularAdd)
		o.add(" cur.rgb=clamp(cur.rgb+vS.rgb,0.0,1.0);\n");

	if (k.fogSource == FOGSRC_TABLE)
	{
		if (k.fogMode == D3DFOG_EXP) o.add(" highp float f=clamp(exp(-uFog.z*vF),0.0,1.0);\n");
		else if (k.fogMode == D3DFOG_EXP2) o.add(" highp float f=clamp(exp(-(uFog.z*vF)*(uFog.z*vF)),0.0,1.0);\n");
		else o.add(" highp float f=clamp((uFog.y-vF)*uFog.w,0.0,1.0);\n");
		o.add(" cur.rgb=mix(uFogColor.rgb,cur.rgb,f);\n");
	}
	else if (k.fogSource != FOGSRC_NONE)
		o.add(" cur.rgb=mix(uFogColor.rgb,cur.rgb,clamp(vF,0.0,1.0));\n");

	if (k.alphaFunc == D3DCMP_NEVER)
		o.add(" discard;\n");
	else if (k.alphaFunc && k.alphaFunc != D3DCMP_ALWAYS)
		o.addf(" if(!(floor(cur.a*255.0+0.5)%suAR)) discard;\n", CmpOp(k.alphaFunc));

	o.add(" oC=cur;\n}\n");
	return o.s;
}

GLuint Compile(GLenum type, const std::string& src)
{
	GLuint sh = glCreateShader(type);
	const char* p = src.c_str();
	glShaderSource(sh, 1, &p, nullptr);
	glCompileShader(sh);
	GLint ok = 0;
	glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
	if (!ok)
	{
		char log[2048];
		glGetShaderInfoLog(sh, sizeof(log), nullptr, log);
		CD3D_LOGE("shader derleme hatasi:\n%s\n---\n%s", log, src.c_str());
		glDeleteShader(sh);
		return 0;
	}
	return sh;
}

}	// namespace

// ---------------------------------------------------------------- program ikili onbellegi

static uint64_t HashBytes(const void* data, size_t n, uint64_t h)
{
	const uint8_t* p = (const uint8_t*)data;
	for (size_t i = 0; i < n; ++i)
		h = (h ^ p[i]) * 1099511628211ull;
	return h;
}

static std::string s_cacheDir;
static int s_cacheState = 0;	// 0 bilinmiyor, 1 acik, -1 kapali

static bool BinaryCacheEnabled()
{
	if (s_cacheState == 0)
	{
		s_cacheState = -1;
		GLint n = 0;
		glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &n);
		const char* base = CynexPlatform_GetDataPath();
		if (n > 0 && base && *base)
		{
			s_cacheDir = std::string(base) + "/glcache";
			mkdir(s_cacheDir.c_str(), 0700);
			s_cacheState = 1;
		}
		CD3D_LOGI("program ikili onbellegi: %s (%d format)", s_cacheState > 0 ? s_cacheDir.c_str() : "kapali", n);
	}
	return s_cacheState > 0;
}

// Cynex: dosya: imza, kaynak ozeti, surucu ozeti, format, uzunluk, veri
struct BinHeader { uint32_t magic; uint32_t format; uint64_t srcHash; uint64_t drvHash; uint32_t len; uint32_t pad; };

static uint64_t DriverHash()
{
	const char* r = (const char*)glGetString(GL_RENDERER);
	const char* v = (const char*)glGetString(GL_VERSION);
	uint64_t h = HashBytes(r ? r : "", r ? strlen(r) : 0, 1469598103934665603ull);
	return HashBytes(v ? v : "", v ? strlen(v) : 0, h);
}

static std::string BinPath(uint64_t srcHash)
{
	char name[40];
	snprintf(name, sizeof(name), "/%016llx.bin", (unsigned long long)srcHash);
	return s_cacheDir + name;
}

static GLuint LoadBinary(uint64_t srcHash)
{
	if (!BinaryCacheEnabled())
		return 0;
	FILE* f = fopen(BinPath(srcHash).c_str(), "rb");
	if (!f)
		return 0;
	BinHeader h;
	GLuint prog = 0;
	if (fread(&h, sizeof(h), 1, f) == 1 && h.magic == 0x42505943 && h.srcHash == srcHash && h.drvHash == DriverHash() && h.len > 0 && h.len < (64u << 20))
	{
		std::vector<uint8_t> data(h.len);
		if (fread(data.data(), 1, h.len, f) == h.len)
		{
			prog = glCreateProgram();
			glProgramBinary(prog, h.format, data.data(), h.len);
			GLint ok = 0;
			glGetProgramiv(prog, GL_LINK_STATUS, &ok);
			if (!ok)
			{
				glDeleteProgram(prog);
				prog = 0;
			}
		}
	}
	fclose(f);
	while (glGetError() != GL_NO_ERROR) {}
	return prog;
}

static void SaveBinary(GLuint prog, uint64_t srcHash)
{
	if (!BinaryCacheEnabled())
		return;
	GLint len = 0;
	glGetProgramiv(prog, GL_PROGRAM_BINARY_LENGTH, &len);
	if (len <= 0)
		return;
	std::vector<uint8_t> data(len);
	BinHeader h = { 0x42505943, 0, srcHash, DriverHash(), 0, 0 };
	GLsizei got = 0;
	GLenum fmt = 0;
	glGetProgramBinary(prog, len, &got, &fmt, data.data());
	if (got <= 0)
		return;
	h.format = fmt;
	h.len = (uint32_t)got;
	std::string path = BinPath(srcHash), tmp = path + ".tmp";
	FILE* f = fopen(tmp.c_str(), "wb");
	if (!f)
		return;
	bool ok = fwrite(&h, sizeof(h), 1, f) == 1 && fwrite(data.data(), 1, got, f) == (size_t)got;
	fclose(f);
	if (ok)
		rename(tmp.c_str(), path.c_str());
	else
		remove(tmp.c_str());
}

FFProgram* FFCache::Get(const FFKey& key)
{
	auto it = m_map.find(key);
	if (it != m_map.end())
		return it->second;
	FFProgram* p = Build(key);
	m_map.emplace(key, p);
	return p;
}

FFProgram* FFCache::Build(const FFKey& k)
{
	FFProgram* p = new FFProgram;
	memset(p->serial, 0, sizeof(p->serial));
	for (int i = 0; i < 8; ++i)
		p->uTexMat[i] = -1;
	p->key = k;

	std::string vs = GenVS(k), fs = GenFS(k);
	uint64_t srcHash = HashBytes(vs.data(), vs.size(), HashBytes(fs.data(), fs.size(), 1469598103934665603ull));
	GLuint prog = LoadBinary(srcHash);
	if (!prog)
	{
		GLuint v = Compile(GL_VERTEX_SHADER, vs);
		GLuint f = Compile(GL_FRAGMENT_SHADER, fs);
		if (!v || !f)
		{
			if (v) glDeleteShader(v);
			if (f) glDeleteShader(f);
			return p;
		}
		prog = glCreateProgram();
		glAttachShader(prog, v);
		glAttachShader(prog, f);
		if (BinaryCacheEnabled())
			glProgramParameteri(prog, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
		glLinkProgram(prog);
		glDeleteShader(v);
		glDeleteShader(f);
		GLint ok = 0;
		glGetProgramiv(prog, GL_LINK_STATUS, &ok);
		if (!ok)
		{
			char log[2048];
			glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
			CD3D_LOGE("program baglama hatasi: %s", log);
			glDeleteProgram(prog);
			return p;
		}
		SaveBinary(prog, srcHash);
	}
	p->prog = prog;

	glUseProgram(prog);
	char name[32];
	for (int s = 0; s < 8; ++s)
	{
		snprintf(name, sizeof(name), "uS%d", s);
		GLint loc = glGetUniformLocation(prog, name);
		if (loc >= 0)
			glUniform1i(loc, s);
		snprintf(name, sizeof(name), "uTM%d", s);
		p->uTexMat[s] = glGetUniformLocation(prog, name);
	}
	p->uMVP = glGetUniformLocation(prog, "uMVP");
	p->uWV = glGetUniformLocation(prog, "uWV");
	p->uNormalMat = glGetUniformLocation(prog, "uNM");
	p->uRhw = glGetUniformLocation(prog, "uRhw");
	p->uMatDiff = glGetUniformLocation(prog, "uMD");
	p->uMatAmb = glGetUniformLocation(prog, "uMA");
	p->uMatSpec = glGetUniformLocation(prog, "uMS");
	p->uMatEmis = glGetUniformLocation(prog, "uME");
	p->uMatPower = glGetUniformLocation(prog, "uMP");
	p->uAmbient = glGetUniformLocation(prog, "uAmb");
	p->uLDiff = glGetUniformLocation(prog, "uLD");
	p->uLSpec = glGetUniformLocation(prog, "uLS");
	p->uLAmb = glGetUniformLocation(prog, "uLA");
	p->uLPos = glGetUniformLocation(prog, "uLP");
	p->uLDir = glGetUniformLocation(prog, "uLR");
	p->uLAtt = glGetUniformLocation(prog, "uLT");
	p->uLSpot = glGetUniformLocation(prog, "uLC");
	p->uFog = glGetUniformLocation(prog, "uFog");
	p->uFogColor = glGetUniformLocation(prog, "uFogColor");
	p->uTF = glGetUniformLocation(prog, "uTF");
	p->uAlphaRef = glGetUniformLocation(prog, "uAR");
	p->uBorder = glGetUniformLocation(prog, "uBorder");
	return p;
}

void FFCache::Clear()
{
	for (auto& it : m_map)
	{
		if (it.second->prog)
			glDeleteProgram(it.second->prog);
		delete it.second;
	}
	m_map.clear();
}

}	// namespace cd3d
