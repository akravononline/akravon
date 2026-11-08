// Cynex: sabit islev hatti -> glsl es 3.00 uretici
#pragma once
#include "cd3d_common.h"

namespace cd3d {

enum
{
	ATTR_POS = 0,
	ATTR_NORMAL = 1,
	ATTR_DIFFUSE = 2,
	ATTR_SPECULAR = 3,
	ATTR_TEX0 = 4,		// 4..11
	ATTR_COUNT = 12,
};

enum { FOGSRC_NONE = 0, FOGSRC_VERTEX, FOGSRC_SPECULAR, FOGSRC_TABLE };
enum { TCI_PASS = 0, TCI_CAMNORMAL, TCI_CAMPOS, TCI_REFLECT };

// Cynex: anahtar bayt bayt karsilastirilir - once sifirla
struct FFStage
{
	uint8_t colorOp, alphaOp;
	uint8_t colorArg[3];		// arg0, arg1, arg2
	uint8_t alphaArg[3];
	uint8_t resultTemp;
	uint8_t sample;				// doku orneklenir mi
	uint8_t texBound;
	uint8_t border;				// bit0 u, bit1 v
	uint8_t tciIndex, tciMode;
	uint8_t ttfCount, ttfProjected;
};

struct FFKey
{
	uint8_t posRHW, hasNormal, hasDiffuse, hasSpecular;
	uint8_t texSize[8];			// 0 yok, 1..4
	uint8_t isPoint;
	uint8_t lighting, specularEnable, localViewer, normalize;
	uint8_t diffSrc, ambSrc, specSrc, emisSrc;	// 0 malzeme, 1 color1, 2 color2
	uint8_t numLights;
	uint8_t lightType[8];		// D3DLIGHTTYPE
	uint8_t fogSource, fogMode, rangeFog;
	uint8_t alphaFunc;			// 0 kapali
	uint8_t specularAdd;
	uint8_t numStages;
	uint8_t pad[2];
	FFStage st[8];
};

struct FFKeyHash
{
	size_t operator()(const FFKey& k) const
	{
		const uint8_t* p = (const uint8_t*)&k;
		uint64_t h = 1469598103934665603ull;
		for (size_t i = 0; i < sizeof(FFKey); ++i)
			h = (h ^ p[i]) * 1099511628211ull;
		return (size_t)h;
	}
};
struct FFKeyEq
{
	bool operator()(const FFKey& a, const FFKey& b) const { return memcmp(&a, &b, sizeof(FFKey)) == 0; }
};

enum
{
	SG_MVP = 0,
	SG_WV,
	SG_TEXMAT,
	SG_MATERIAL,
	SG_LIGHTS,
	SG_FOG,
	SG_TFACTOR,
	SG_ALPHAREF,
	SG_BORDER,
	SG_RHW,
	SG_COUNT
};

struct FFProgram
{
	GLuint prog = 0;
	GLint uMVP = -1, uWV = -1, uNormalMat = -1, uRhw = -1;
	GLint uTexMat[8];
	GLint uMatDiff = -1, uMatAmb = -1, uMatSpec = -1, uMatEmis = -1, uMatPower = -1, uAmbient = -1;
	GLint uLDiff = -1, uLSpec = -1, uLAmb = -1, uLPos = -1, uLDir = -1, uLAtt = -1, uLSpot = -1;
	GLint uFog = -1, uFogColor = -1, uTF = -1, uAlphaRef = -1, uBorder = -1;
	uint32_t serial[SG_COUNT];
	FFKey key;
};

class FFCache
{
public:
	FFProgram* Get(const FFKey& key);
	void Clear();
	size_t Size() const { return m_map.size(); }

private:
	FFProgram* Build(const FFKey& key);
	std::unordered_map<FFKey, FFProgram*, FFKeyHash, FFKeyEq> m_map;
};

}	// namespace cd3d
