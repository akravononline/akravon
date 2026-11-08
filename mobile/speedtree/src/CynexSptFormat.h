///////////////////////////////////////////////////////////////////////
//	CynexSptFormat.h
//
//	Cynex: sptconv ciktisi (.spt yerine gecen dosya) icin ortak tanimlar.
//	Cynex: little-endian, 4 bayt hizali; [magic][version][chunk...] duzeni.
//	Cynex: her chunk = fourcc + bayt sayisi + veri; bilinmeyen chunk atlanir.

#pragma once

#include <stdint.h>

namespace CynexSpt
{
	// Cynex: orijinal .spt "begin_file" token'i ile cakismasin diye ascii imza
	static const char		c_szMagic[8] = { 'C', 'Y', 'N', 'X', 'S', 'P', 'T', 0x1A };
	static const uint32_t	c_uiVersion = 1;

	enum ETag
	{
		TAG_HEAD = 0x44414548,	// 'HEAD'
		TAG_TEXS = 0x53584554,	// 'TEXS'
		TAG_COLL = 0x4C4C4F43,	// 'COLL'
		TAG_BRCH = 0x48435242,	// 'BRCH'
		TAG_FRND = 0x444E5246,	// 'FRND'
		TAG_LEAF = 0x4641454C,	// 'LEAF'
		TAG_BILL = 0x4C4C4942,	// 'BILL'
	};

	// Cynex: HEAD.uiFlags
	enum EHeadFlags
	{
		HEAD_HAS_BILLBOARD		= 1 << 0,	// basit billboard var (drop-to-billboard sadece bunda etkili)
		HEAD_HAS_360			= 1 << 1,	// 360 derece billboard dizisi
		HEAD_HAS_HORIZONTAL		= 1 << 2,	// yatay billboard
	};

	// Cynex: indexed geometri dizi maskesi (BRCH/FRND)
	enum EArrayBits
	{
		ARRAY_COORDS			= 1 << 0,
		ARRAY_NORMALS			= 1 << 1,
		ARRAY_BINORMALS			= 1 << 2,
		ARRAY_TANGENTS			= 1 << 3,
		ARRAY_TEXCOORDS0		= 1 << 4,
		ARRAY_TEXCOORDS1		= 1 << 5,
		ARRAY_COLORS			= 1 << 6,
		ARRAY_WIND_WEIGHTS		= 1 << 7,
		ARRAY_WIND_INDICES		= 1 << 8,
		ARRAY_FROND_LOCAL_UV	= 1 << 9,	// SetFrondTextureCoords icin harita ici (u,v) + vertex basina harita
		// Cynex: statik isikta binormal/tangent de veriliyorsa (dallar) bu bit acik
		ARRAY_BINTAN_STATIC		= 1 << 10,
	};

	// Cynex: yaprak LOD dizi maskesi
	enum ELeafArrayBits
	{
		LEAF_COLORS_STATIC		= 1 << 0,
		LEAF_COLORS_DYNAMIC		= 1 << 1,
		LEAF_NORMALS			= 1 << 2,
		LEAF_BINORMALS			= 1 << 3,
		LEAF_TANGENTS			= 1 << 4,
		LEAF_WIND_WEIGHTS		= 1 << 5,
		LEAF_WIND_INDICES		= 1 << 6,
	};

	///////////////////////////////////////////////////////////////////////
	//	sabit boyutlu bloklar (dosyada aynen bu sirayla)

	struct SHead
	{
		uint16_t	usBranchLods;
		uint16_t	usFrondLods;
		uint16_t	usLeafLods;
		uint16_t	usLeafLodMethod;	// Cynex: CSpeedTreeRT::ELodMethod (POP / SMOOTH / NONE)
		uint32_t	uiFlags;
		float		afBoundingBox[6];
		float		fTreeSize;
		float		fTreeSizeVariance;
		uint32_t	uiSeed;
		// Cynex: dosyadaki varsayilanlar (CSpeedTreeRT getter'lari bunlari doner)
		int32_t		iBranchLighting;
		int32_t		iLeafLighting;
		int32_t		iFrondLighting;
		int32_t		iStaticLightingStyle;
		int32_t		iBranchWind;
		int32_t		iLeafWind;
		int32_t		iFrondWind;
		int32_t		iLeafRocking;
		float		fLeafLightingAdjustment;
		float		fLodNear;
		float		fLodFar;
		// Cynex: yaprak sallanma: aci = derece(fRockAngle * s) * sin(fRockFrequency * s * t + faz)
		float		fWindStrengthInitial;
		float		fRockAngle;
		float		fRockFrequency;
		float		afBranchMaterial[13];
		float		afLeafMaterial[13];
		float		afFrondMaterial[13];
		// Cynex: LOD gecis egrisi (yaricap, gecis carpani, us)
		float		fLodRadius;
		float		fLodTransition;
		float		fLodExponent;
		// Cynex: statik renkler hangi isikla hesaplandi (bilgi amacli)
		float		afConvertLight[16];
	};

	struct SCollision
	{
		int32_t		iType;
		float		afPosition[3];
		float		afDimensions[3];
	};

	struct SLeafCluster
	{
		float		fYaw;		// radyan, kamera azimutuna eklenir
		float		fPitch;		// radyan, kamera egiminden cikarilir
		float		fPhase;		// sallanma fazi
		float		afCorners[12];	// kamera duzleminde 4 kose (x,y,z), LOD0 olcegi
	};

	struct SBillboard
	{
		float		fHalfWidth;
		float		fTop;
		float		fBottom;
		float		afTexCoords[8];	// basit billboard (cevrilmemis)
		// Cynex: 360 derece dizisi
		uint32_t	uiNum360;
		float		f360Offset;		// ilk sektor baslangici (radyan); gecis egrisi LOD ile ayni
		// Cynex: yatay billboard
		float		afHorizontalCoords[12];
		float		afHorizontalTexCoords[8];
		// Cynex: ardindan uiNum360 * 8 float (resim texcoord'lari)
	};
}
