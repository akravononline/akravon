// Cynex: data_type_definition tabanli boyut, uye arama ve donusum
#include "granny_internal.h"

#include <cmath>

using namespace CynexGranny;

// Cynex: dosya duzeni = header'daki pack(4) yapilar; uye boyutlari toplami ile ayni olmali
static_assert(sizeof(granny_data_type_definition) == 4 + 3 * sizeof(void *) + 16, "data_type_definition duzeni");
static_assert(sizeof(granny_variant) == 2 * sizeof(void *), "variant duzeni");
static_assert(sizeof(granny_transform) == 68, "transform duzeni");

namespace
{
	bool IsScalarType(granny_member_type eType)
	{
		switch (eType)
		{
			case GrannyReal32Member:
			case GrannyInt8Member:
			case GrannyUInt8Member:
			case GrannyBinormalInt8Member:
			case GrannyNormalUInt8Member:
			case GrannyInt16Member:
			case GrannyUInt16Member:
			case GrannyBinormalInt16Member:
			case GrannyNormalUInt16Member:
			case GrannyInt32Member:
			case GrannyUInt32Member:
			case GrannyReal16Member:
				return true;
			default:
				return false;
		}
	}

	int ScalarSize(granny_member_type eType)
	{
		switch (eType)
		{
			case GrannyInt8Member:
			case GrannyUInt8Member:
			case GrannyBinormalInt8Member:
			case GrannyNormalUInt8Member:
				return 1;
			case GrannyInt16Member:
			case GrannyUInt16Member:
			case GrannyBinormalInt16Member:
			case GrannyNormalUInt16Member:
			case GrannyReal16Member:
				return 2;
			default:
				return 4;
		}
	}

	float HalfToFloat(granny_uint16 h)
	{
		const uint32_t sign = (uint32_t) (h & 0x8000u) << 16;
		const uint32_t exponent = (h >> 10) & 0x1Fu;
		uint32_t mantissa = h & 0x3FFu;
		uint32_t bits;
		if (exponent == 0)
		{
			if (mantissa == 0)
				bits = sign;
			else
			{
				int e = -1;
				do
				{
					++e;
					mantissa <<= 1;
				} while (!(mantissa & 0x400u));
				bits = sign | ((uint32_t) (127 - 15 - e) << 23) | ((mantissa & 0x3FFu) << 13);
			}
		}
		else if (exponent == 31)
			bits = sign | 0x7F800000u | (mantissa << 13);
		else
			bits = sign | ((exponent + 127 - 15) << 23) | (mantissa << 13);
		float f;
		memcpy(&f, &bits, sizeof(f));
		return f;
	}

	granny_uint16 FloatToHalf(float f)
	{
		uint32_t bits;
		memcpy(&bits, &f, sizeof(bits));
		const uint32_t sign = (bits >> 16) & 0x8000u;
		const int exponent = (int) ((bits >> 23) & 0xFFu) - 127 + 15;
		const uint32_t mantissa = bits & 0x7FFFFFu;
		if (((bits >> 23) & 0xFFu) == 0xFFu)
			return (granny_uint16) (sign | 0x7C00u | (mantissa ? 0x200u : 0u));
		if (exponent >= 31)
			return (granny_uint16) (sign | 0x7C00u);
		if (exponent <= 0)
		{
			if (exponent < -10)
				return (granny_uint16) sign;
			const uint32_t m = mantissa | 0x800000u;
			return (granny_uint16) (sign | (m >> (14 - exponent)));
		}
		return (granny_uint16) (sign | ((uint32_t) exponent << 10) | (mantissa >> 13));
	}

	double ReadScalar(granny_member_type eType, const uint8_t * p)
	{
		switch (eType)
		{
			case GrannyReal32Member:		{ float v; memcpy(&v, p, 4); return v; }
			case GrannyInt8Member:			return (double) (int8_t) p[0];
			case GrannyUInt8Member:			return (double) p[0];
			case GrannyBinormalInt8Member:	return (double) (int8_t) p[0] / 127.0;
			case GrannyNormalUInt8Member:	return (double) p[0] / 255.0;
			case GrannyInt16Member:			{ int16_t v; memcpy(&v, p, 2); return v; }
			case GrannyUInt16Member:		{ uint16_t v; memcpy(&v, p, 2); return v; }
			case GrannyBinormalInt16Member:	{ int16_t v; memcpy(&v, p, 2); return v / 32767.0; }
			case GrannyNormalUInt16Member:	{ uint16_t v; memcpy(&v, p, 2); return v / 65535.0; }
			case GrannyInt32Member:			{ int32_t v; memcpy(&v, p, 4); return v; }
			case GrannyUInt32Member:		{ uint32_t v; memcpy(&v, p, 4); return v; }
			case GrannyReal16Member:		{ uint16_t v; memcpy(&v, p, 2); return HalfToFloat(v); }
			default:						return 0.0;
		}
	}

	template <typename T>
	T ClampRound(double v, double lo, double hi)
	{
		v = std::floor(v + 0.5);
		if (v < lo)
			v = lo;
		if (v > hi)
			v = hi;
		return (T) v;
	}

	template <typename T>
	T ClampTruncate(double v, double lo, double hi)
	{
		if (!(v >= lo))
			v = lo;
		if (v > hi)
			v = hi;
		return (T) v;
	}

	void WriteScalar(granny_member_type eType, uint8_t * p, double v)
	{
		switch (eType)
		{
			case GrannyReal32Member:		{ float f = (float) v; memcpy(p, &f, 4); break; }
			case GrannyInt8Member:			{ int8_t i = ClampTruncate<int8_t>(v, -128.0, 127.0); memcpy(p, &i, 1); break; }
			case GrannyUInt8Member:			{ uint8_t i = ClampTruncate<uint8_t>(v, 0.0, 255.0); memcpy(p, &i, 1); break; }
			case GrannyBinormalInt8Member:	{ int8_t i = ClampRound<int8_t>(v * 127.0, -127.0, 127.0); memcpy(p, &i, 1); break; }
			case GrannyNormalUInt8Member:	{ uint8_t i = ClampRound<uint8_t>(v * 255.0, 0.0, 255.0); memcpy(p, &i, 1); break; }
			case GrannyInt16Member:			{ int16_t i = ClampTruncate<int16_t>(v, -32768.0, 32767.0); memcpy(p, &i, 2); break; }
			case GrannyUInt16Member:		{ uint16_t i = ClampTruncate<uint16_t>(v, 0.0, 65535.0); memcpy(p, &i, 2); break; }
			case GrannyBinormalInt16Member:	{ int16_t i = ClampRound<int16_t>(v * 32767.0, -32767.0, 32767.0); memcpy(p, &i, 2); break; }
			case GrannyNormalUInt16Member:	{ uint16_t i = ClampRound<uint16_t>(v * 65535.0, 0.0, 65535.0); memcpy(p, &i, 2); break; }
			case GrannyInt32Member:			{ int32_t i = ClampTruncate<int32_t>(v, -2147483648.0, 2147483647.0); memcpy(p, &i, 4); break; }
			case GrannyUInt32Member:		{ uint32_t i = ClampTruncate<uint32_t>(v, 0.0, 4294967295.0); memcpy(p, &i, 4); break; }
			case GrannyReal16Member:		{ uint16_t h = FloatToHalf((float) v); memcpy(p, &h, 2); break; }
			default: break;
		}
	}

	int Width(const granny_data_type_definition * pMember)
	{
		return pMember->ArrayWidth > 0 ? pMember->ArrayWidth : 1;
	}

	bool NamesEqual(const char * a, const char * b)
	{
		if (!a || !b)
			return false;
		return strcmp(a, b) == 0;
	}

	granny_data_type_definition gs_akPNT332VertexType[] =
	{
		{ GrannyReal32Member, GrannyVertexPositionName, NULL, 3, { 0, 0, 0 }, 0 },
		{ GrannyReal32Member, GrannyVertexNormalName, NULL, 3, { 0, 0, 0 }, 0 },
		{ GrannyReal32Member, GrannyVertexTextureCoordinatesName "0", NULL, 2, { 0, 0, 0 }, 0 },
		{ GrannyEndMember, NULL, NULL, 0, { 0, 0, 0 }, 0 },
	};
}

namespace CynexGranny
{
	int GetMemberCount(const granny_data_type_definition * pType)
	{
		int n = 0;
		if (pType)
			while (pType[n].Type != GrannyEndMember)
				++n;
		return n;
	}

	int GetMemberSize(const granny_data_type_definition * pMember)
	{
		int elementSize = 0;
		switch (pMember->Type)
		{
			case GrannyInlineMember:					elementSize = GetObjectSize(pMember->ReferenceType); break;
			case GrannyReferenceMember:					elementSize = (int) sizeof(void *); break;
			case GrannyReferenceToArrayMember:			elementSize = (int) (sizeof(granny_int32) + sizeof(void *)); break;
			case GrannyArrayOfReferencesMember:			elementSize = (int) (sizeof(granny_int32) + sizeof(void *)); break;
			case GrannyVariantReferenceMember:			elementSize = (int) (2 * sizeof(void *)); break;
			case GrannyReferenceToVariantArrayMember:	elementSize = (int) (sizeof(void *) + sizeof(granny_int32) + sizeof(void *)); break;
			case GrannyStringMember:					elementSize = (int) sizeof(char *); break;
			case GrannyTransformMember:					elementSize = (int) sizeof(granny_transform); break;
			case GrannyEmptyReferenceMember:			elementSize = (int) sizeof(void *); break;
			default:
				if (IsScalarType(pMember->Type))
					elementSize = ScalarSize(pMember->Type);
				break;
		}
		return elementSize * Width(pMember);
	}

	int GetObjectSize(const granny_data_type_definition * pType)
	{
		int size = 0;
		if (pType)
			for (const granny_data_type_definition * p = pType; p->Type != GrannyEndMember; ++p)
				size += GetMemberSize(p);
		return size;
	}

	const granny_data_type_definition * FindMember(const granny_data_type_definition * pType, const char * c_szName, int * piOffset)
	{
		if (!pType || !c_szName)
			return NULL;

		int offset = 0;
		for (const granny_data_type_definition * p = pType; p->Type != GrannyEndMember; ++p)
		{
			if (NamesEqual(p->Name, c_szName))
			{
				if (piOffset)
					*piOffset = offset;
				return p;
			}
			offset += GetMemberSize(p);
		}
		return NULL;
	}

	void ConvertMember(const granny_data_type_definition * pSrcMember, const void * c_pvSrc, const granny_data_type_definition * pDstMember, void * pvDst)
	{
		const uint8_t * pSrc = static_cast<const uint8_t *>(c_pvSrc);
		uint8_t * pDst = static_cast<uint8_t *>(pvDst);
		const int dstSize = GetMemberSize(pDstMember);

		if (!pSrcMember)
		{
			memset(pDst, 0, dstSize);
			return;
		}

		if (IsScalarType(pSrcMember->Type) && IsScalarType(pDstMember->Type))
		{
			const int srcWidth = Width(pSrcMember);
			const int dstWidth = Width(pDstMember);
			const int srcStride = ScalarSize(pSrcMember->Type);
			const int dstStride = ScalarSize(pDstMember->Type);
			if (pSrcMember->Type == pDstMember->Type)
			{
				const int common = (srcWidth < dstWidth ? srcWidth : dstWidth) * srcStride;
				memcpy(pDst, pSrc, common);
				if (dstSize > common)
					memset(pDst + common, 0, dstSize - common);
				return;
			}
			for (int i = 0; i < dstWidth; ++i)
			{
				if (i < srcWidth)
					WriteScalar(pDstMember->Type, pDst + i * dstStride, ReadScalar(pSrcMember->Type, pSrc + i * srcStride));
				else
					memset(pDst + i * dstStride, 0, dstStride);
			}
			return;
		}

		// Cynex: pointer/inline gibi uyeler sadece ayni tipteyse kopyalanir
		if (pSrcMember->Type == pDstMember->Type && GetMemberSize(pSrcMember) == dstSize
			&& (pSrcMember->Type != GrannyInlineMember || pSrcMember->ReferenceType == pDstMember->ReferenceType))
		{
			memcpy(pDst, pSrc, dstSize);
			return;
		}

		if (pSrcMember->Type == GrannyInlineMember && pDstMember->Type == GrannyInlineMember)
		{
			const int srcObject = GetObjectSize(pSrcMember->ReferenceType);
			const int dstObject = GetObjectSize(pDstMember->ReferenceType);
			const int srcWidth = Width(pSrcMember);
			const int dstWidth = Width(pDstMember);
			for (int i = 0; i < dstWidth; ++i)
			{
				uint8_t * pDstObject = pDst + i * dstObject;
				if (i >= srcWidth)
				{
					memset(pDstObject, 0, dstObject);
					continue;
				}
				const uint8_t * pSrcObject = pSrc + i * srcObject;
				int dstOffset = 0;
				for (const granny_data_type_definition * d = pDstMember->ReferenceType; d->Type != GrannyEndMember; ++d)
				{
					int srcOffset = 0;
					const granny_data_type_definition * s = FindMember(pSrcMember->ReferenceType, d->Name, &srcOffset);
					ConvertMember(s, s ? pSrcObject + srcOffset : NULL, d, pDstObject + dstOffset);
					dstOffset += GetMemberSize(d);
				}
			}
			return;
		}

		memset(pDst, 0, dstSize);
	}

	int BuildVertexCopyPlan(const granny_data_type_definition * pSrcType, const granny_data_type_definition * pDstType, SVertexCopyOp * pOps, int iMaxOps, const char * const * c_pszSkipNames, int iSkipCount)
	{
		int count = 0;
		int dstOffset = 0;
		for (const granny_data_type_definition * d = pDstType; d && d->Type != GrannyEndMember; ++d)
		{
			const int dstSize = GetMemberSize(d);
			bool isSkipped = false;
			for (int i = 0; i < iSkipCount && !isSkipped; ++i)
				isSkipped = NamesEqual(d->Name, c_pszSkipNames[i]);

			if (!isSkipped && count < iMaxOps)
			{
				SVertexCopyOp & rOp = pOps[count++];
				int srcOffset = 0;
				rOp.pSrcMember = FindMember(pSrcType, d->Name, &srcOffset);
				rOp.pDstMember = d;
				rOp.iSrcOffset = srcOffset;
				rOp.iDstOffset = dstOffset;
				rOp.iSize = 0;
				if (rOp.pSrcMember && rOp.pSrcMember->Type == d->Type && Width(rOp.pSrcMember) == Width(d) && IsScalarType(d->Type))
					rOp.iSize = dstSize;
			}
			dstOffset += dstSize;
		}
		return count;
	}

	void RunVertexCopyPlan(const SVertexCopyOp * c_pOps, int iOpCount, const uint8_t * c_pSrc, uint8_t * pDst)
	{
		for (int i = 0; i < iOpCount; ++i)
		{
			const SVertexCopyOp & rOp = c_pOps[i];
			if (rOp.iSize)
				memcpy(pDst + rOp.iDstOffset, c_pSrc + rOp.iSrcOffset, rOp.iSize);
			else
				ConvertMember(rOp.pSrcMember, rOp.pSrcMember ? c_pSrc + rOp.iSrcOffset : NULL, rOp.pDstMember, pDst + rOp.iDstOffset);
		}
	}
}

GRANNY_DYNLINKDATA(granny_data_type_definition *) GrannyPNT332VertexType = gs_akPNT332VertexType;

// Cynex: EterGrnLib sonucu 32'ye bolup uye sayisini buluyor; 64bit'te de x86 boyutu (32 bayt/uye) doner
GRANNY_DYNLINK(granny_int32x) GrannyGetTotalTypeSize(granny_data_type_definition const * TypeDefinition)
{
	if (!TypeDefinition)
		return 0;
	return (GetMemberCount(TypeDefinition) + 1) * 32;
}

GRANNY_DYNLINK(bool) GrannyFindMatchingMember(granny_data_type_definition const * SourceType, void const * SourceObject, char const * DestMemberName, granny_variant * Result)
{
	int offset = 0;
	const granny_data_type_definition * pMember = (SourceObject && DestMemberName) ? FindMember(SourceType, DestMemberName, &offset) : NULL;
	if (!pMember)
	{
		if (Result)
		{
			Result->Type = NULL;
			Result->Object = NULL;
		}
		return false;
	}

	if (Result)
	{
		Result->Type = const_cast<granny_data_type_definition *>(pMember);
		Result->Object = const_cast<uint8_t *>(static_cast<const uint8_t *>(SourceObject) + offset);
	}
	return true;
}

GRANNY_DYNLINK(void) GrannyConvertSingleObject(granny_data_type_definition const * SourceType, void const * SourceObject, granny_data_type_definition const * DestType, void * DestObject, granny_conversion_handler * OverrideHandler)
{
	(void) OverrideHandler;
	if (!DestType || !DestObject)
		return;

	uint8_t * pDst = static_cast<uint8_t *>(DestObject);
	const uint8_t * pSrc = static_cast<const uint8_t *>(SourceObject);
	int dstOffset = 0;
	for (const granny_data_type_definition * d = DestType; d->Type != GrannyEndMember; ++d)
	{
		int srcOffset = 0;
		const granny_data_type_definition * s = pSrc ? FindMember(SourceType, d->Name, &srcOffset) : NULL;
		ConvertMember(s, s ? pSrc + srcOffset : NULL, d, pDst + dstOffset);
		dstOffset += GetMemberSize(d);
	}
}
