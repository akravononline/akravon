// Cynex: cynex_granny ic tanimlar (dis koda acik degil)
#pragma once

#include <granny.h>

#include <cstddef>
#include <cstdint>
#include <cstring>

#if defined(_MSC_VER)
#define CYNEX_FORCEINLINE __forceinline
#else
#define CYNEX_FORCEINLINE inline __attribute__((always_inline))
#endif

namespace CynexGranny
{
	// Cynex: bellek
	void *	AlignedAlloc(size_t size, size_t alignment);
	void	AlignedFree(void * p);

	template <typename T>
	T * AllocArray(size_t count, size_t alignment = 16)
	{
		return static_cast<T *>(AlignedAlloc(count ? count * sizeof(T) : 1, alignment));
	}

	// Cynex: log
	void	Log(granny_log_message_type eType, granny_log_message_origin eOrigin, const char * c_szFile, int iLine, const char * c_szFormat, ...)
#if defined(__GNUC__) || defined(__clang__)
		__attribute__((format(printf, 5, 6)))
#endif
		;

	// Cynex: tip sistemi
	int		GetMemberCount(const granny_data_type_definition * pType);
	int		GetMemberSize(const granny_data_type_definition * pMember);
	int		GetObjectSize(const granny_data_type_definition * pType);
	const granny_data_type_definition * FindMember(const granny_data_type_definition * pType, const char * c_szName, int * piOffset);

	// Cynex: tek uye donusumu (isim eslesmesi disarida yapilir)
	void	ConvertMember(const granny_data_type_definition * pSrcMember, const void * c_pvSrc, const granny_data_type_definition * pDstMember, void * pvDst);

	// Cynex: vertex donusum plani (CopyMeshVertices ve deformer ortak)
	struct SVertexCopyOp
	{
		const granny_data_type_definition *	pSrcMember;	// Cynex: NULL ise hedef sifirlanir
		const granny_data_type_definition *	pDstMember;
		int									iSrcOffset;
		int									iDstOffset;
		int									iSize;		// Cynex: ayni tip ise duz kopya boyutu, degilse 0
	};

	int		BuildVertexCopyPlan(const granny_data_type_definition * pSrcType, const granny_data_type_definition * pDstType, SVertexCopyOp * pOps, int iMaxOps, const char * const * c_pszSkipNames, int iSkipCount);
	void	RunVertexCopyPlan(const SVertexCopyOp * c_pOps, int iOpCount, const uint8_t * c_pSrc, uint8_t * pDst);

	// Cynex: dosya bolumleri (animasyon baglama onbellegi temizligi icin)
	void	OnMemoryRangeFreed(const void * c_pvBegin, size_t size);

	// Cynex: matris / transform
	void	BuildCompositeMatrix(const granny_transform & c_rTransform, float * pMatrix4x4);
	void	MultiplyAffine4x4(float * pResult, const float * c_pA, const float * c_pB);

	extern const float c_afIdentity4x4[16];
}

#define CYNEX_LOG(type, origin, ...) CynexGranny::Log(type, origin, __FILE__, __LINE__, __VA_ARGS__)
