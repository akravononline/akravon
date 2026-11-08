// Cynex: .gr2 yukleyici - sadece gr2conv ciktisi (sikistirmasiz, little-endian, yerel pointer boyu)
#include "granny_internal.h"

#include <new>

using namespace CynexGranny;

namespace
{
	struct SMagic
	{
		granny_uint32 au[4];
	};

	const SMagic c_kMagic64LE = { { 0x5e499be5u, 0x141f636fu, 0xa9eb131eu, 0xc4edbe90u } };
	const SMagic c_kMagic32LE = { { 0xc06cde29u, 0x2b53a4bau, 0xa5b7f525u, 0xeee266f6u } };
	const SMagic c_kMagic64BE = { { 0xe3d49531u, 0x624fdc20u, 0x3ad036ccu, 0x89ff82b1u } };
	const SMagic c_kMagic32BE = { { 0xb595110eu, 0x4bb5a56au, 0x502828ebu, 0x04b37825u } };
	const SMagic c_kMagicOld = { { 0xcab067b8u, 0x0fb16df8u, 0x7e8c7284u, 0x1e00195eu } };

	// Cynex: dosya yerel pointer boyunda olmali
	const SMagic & NativeMagic()
	{
		return sizeof(void *) == 8 ? c_kMagic64LE : c_kMagic32LE;
	}

	bool MagicEquals(const granny_grn_file_magic_value * c_pMagic, const SMagic & c_rkMagic)
	{
		return memcmp(c_pMagic->MagicValue, c_rkMagic.au, sizeof(c_rkMagic.au)) == 0;
	}

	struct SSectionRange
	{
		const uint8_t *	pBegin;
		size_t			size;
	};

	// Cynex: granny_file'in arkasinda tutulan ozel bilgi
	struct SFile
	{
		granny_file		kPublic;
		SSectionRange *	pRanges;
		int				iSectionCount;
	};

	SFile * ToFile(granny_file * pFile)
	{
		return reinterpret_cast<SFile *>(pFile);
	}

	void DestroyFile(SFile * pFile)
	{
		if (!pFile)
			return;

		granny_file & rPublic = pFile->kPublic;
		for (int i = 0; i < pFile->iSectionCount; ++i)
		{
			if (pFile->pRanges[i].size)
				OnMemoryRangeFreed(pFile->pRanges[i].pBegin, pFile->pRanges[i].size);
			if (rPublic.Sections && rPublic.Sections[i] && !rPublic.IsUserMemory[i])
				AlignedFree(rPublic.Sections[i]);
		}

		AlignedFree(rPublic.SourceMagicValue);	// Cynex: Header magic'in ardindaki ic gosterici; ayrilan blogun basi SourceMagicValue
		AlignedFree(rPublic.Sections);
		AlignedFree(rPublic.Marshalled);
		AlignedFree(rPublic.IsUserMemory);
		AlignedFree(pFile->pRanges);
		pFile->~SFile();
		AlignedFree(pFile);
	}

	bool RangeOk(size_t offset, size_t size, size_t total)
	{
		return offset <= total && size <= total - offset;
	}
}

GRANNY_DYNLINK(granny_file *) GrannyReadEntireFileFromMemory(granny_int32x MemorySize, void const * Memory)
{
	const uint8_t * pBytes = static_cast<const uint8_t *>(Memory);
	const size_t size = MemorySize > 0 ? (size_t) MemorySize : 0;

	if (!pBytes || size < sizeof(granny_grn_file_magic_value) + sizeof(granny_grn_file_header))
	{
		CYNEX_LOG(GrannyErrorLogMessage, GrannyFileReadingLogMessage, "File is too small to be a Granny file");
		return NULL;
	}

	granny_grn_file_magic_value kMagic;
	memcpy(&kMagic, pBytes, sizeof(kMagic));

	if (!MagicEquals(&kMagic, NativeMagic()))
	{
		const bool isGranny = MagicEquals(&kMagic, c_kMagic64LE) || MagicEquals(&kMagic, c_kMagic32LE)
			|| MagicEquals(&kMagic, c_kMagic64BE) || MagicEquals(&kMagic, c_kMagic32BE) || MagicEquals(&kMagic, c_kMagicOld);
		if (isGranny)
			CYNEX_LOG(GrannyErrorLogMessage, GrannyFileReadingLogMessage, "Granny file is not in %d-bit little-endian layout (convert it with gr2conv)", (int) (sizeof(void *) * 8));
		else
			CYNEX_LOG(GrannyErrorLogMessage, GrannyFileReadingLogMessage, "File is not a Granny file");
		return NULL;
	}

	const size_t headerSize = kMagic.HeaderSize;
	if (headerSize < sizeof(granny_grn_file_magic_value) + sizeof(granny_grn_file_header) || headerSize > size)
	{
		CYNEX_LOG(GrannyErrorLogMessage, GrannyFileReadingLogMessage, "Invalid Granny header size %u", (unsigned) headerSize);
		return NULL;
	}

	granny_grn_file_header kHeader;
	memcpy(&kHeader, pBytes + sizeof(granny_grn_file_magic_value), sizeof(kHeader));

	if (kHeader.Version != GrannyCurrentGRNFileVersion || kHeader.TypeTag != GrannyCurrentGRNStandardTag)
	{
		CYNEX_LOG(GrannyErrorLogMessage, GrannyFileReadingLogMessage, "Granny file revision %u / type tag 0x%08X is not current (convert it with gr2conv)", kHeader.Version, kHeader.TypeTag);
		return NULL;
	}

	const size_t sectionCount = kHeader.SectionArrayCount;
	const size_t sectionArrayOffset = sizeof(granny_grn_file_magic_value) + (size_t) kHeader.SectionArrayOffset;
	if (sectionCount == 0 || sectionCount > GrannyMaximumSectionCount
		|| !RangeOk(sectionArrayOffset, sectionCount * sizeof(granny_grn_section), headerSize))
	{
		CYNEX_LOG(GrannyErrorLogMessage, GrannyFileReadingLogMessage, "Invalid Granny section table");
		return NULL;
	}

	SFile * pFile = new (AlignedAlloc(sizeof(SFile), 16)) SFile();
	granny_file & rPublic = pFile->kPublic;
	memset(&rPublic, 0, sizeof(rPublic));

	pFile->iSectionCount = (int) sectionCount;
	pFile->pRanges = AllocArray<SSectionRange>(sectionCount);
	rPublic.Sections = AllocArray<void *>(sectionCount);
	rPublic.Marshalled = AllocArray<bool>(sectionCount);
	rPublic.IsUserMemory = AllocArray<bool>(sectionCount);
	rPublic.SectionCount = (granny_int32x) sectionCount;
	for (size_t i = 0; i < sectionCount; ++i)
	{
		pFile->pRanges[i].pBegin = NULL;
		pFile->pRanges[i].size = 0;
		rPublic.Sections[i] = NULL;
		rPublic.Marshalled[i] = true;
		rPublic.IsUserMemory[i] = false;
	}

	// Cynex: header kopyasi (magic + header + section tablosu)
	uint8_t * pHeaderCopy = static_cast<uint8_t *>(AlignedAlloc(headerSize, 16));
	memcpy(pHeaderCopy, pBytes, headerSize);
	rPublic.SourceMagicValue = reinterpret_cast<granny_grn_file_magic_value *>(pHeaderCopy);
	rPublic.Header = reinterpret_cast<granny_grn_file_header *>(pHeaderCopy + sizeof(granny_grn_file_magic_value));
	rPublic.IsByteReversed = 0;

	const granny_grn_section * pSections = reinterpret_cast<const granny_grn_section *>(pHeaderCopy + sectionArrayOffset);

	for (size_t i = 0; i < sectionCount; ++i)
	{
		const granny_grn_section & rSection = pSections[i];
		if (rSection.ExpandedDataSize == 0)
			continue;

		if (rSection.Format != GrannyNoCompression || rSection.DataSize != rSection.ExpandedDataSize
			|| !RangeOk(rSection.DataOffset, rSection.DataSize, size))
		{
			CYNEX_LOG(GrannyErrorLogMessage, GrannyFileReadingLogMessage, "Section %d is compressed or truncated (convert the file with gr2conv)", (int) i);
			DestroyFile(pFile);
			return NULL;
		}

		size_t alignment = rSection.InternalAlignment;
		if (alignment < 16 || (alignment & (alignment - 1)))
			alignment = 16;

		uint8_t * pData = static_cast<uint8_t *>(AlignedAlloc(rSection.ExpandedDataSize, alignment));
		if (!pData)
		{
			CYNEX_LOG(GrannyErrorLogMessage, GrannyFileReadingLogMessage, "Out of memory reading section %d", (int) i);
			DestroyFile(pFile);
			return NULL;
		}
		memcpy(pData, pBytes + rSection.DataOffset, rSection.DataSize);
		rPublic.Sections[i] = pData;
		pFile->pRanges[i].pBegin = pData;
		pFile->pRanges[i].size = rSection.ExpandedDataSize;
	}

	// Cynex: pointer fixup - hedef adres yerel pointer olarak yazilir
	for (size_t i = 0; i < sectionCount; ++i)
	{
		const granny_grn_section & rSection = pSections[i];
		if (rSection.PointerFixupArrayCount == 0)
			continue;

		const size_t fixupBytes = (size_t) rSection.PointerFixupArrayCount * sizeof(granny_grn_pointer_fixup);
		if (!rPublic.Sections[i] || !RangeOk(rSection.PointerFixupArrayOffset, fixupBytes, size))
		{
			CYNEX_LOG(GrannyErrorLogMessage, GrannyFileReadingLogMessage, "Invalid pointer fixup table in section %d", (int) i);
			DestroyFile(pFile);
			return NULL;
		}

		uint8_t * pFrom = static_cast<uint8_t *>(rPublic.Sections[i]);
		const uint8_t * pFixup = pBytes + rSection.PointerFixupArrayOffset;
		for (granny_uint32 f = 0; f < rSection.PointerFixupArrayCount; ++f, pFixup += sizeof(granny_grn_pointer_fixup))
		{
			granny_grn_pointer_fixup kFixup;
			memcpy(&kFixup, pFixup, sizeof(kFixup));

			if (!RangeOk(kFixup.FromOffset, sizeof(void *), rSection.ExpandedDataSize) || kFixup.To.SectionIndex >= sectionCount
				|| kFixup.To.Offset > pSections[kFixup.To.SectionIndex].ExpandedDataSize)
			{
				CYNEX_LOG(GrannyErrorLogMessage, GrannyFileReadingLogMessage, "Pointer fixup %u of section %d is out of range", f, (int) i);
				DestroyFile(pFile);
				return NULL;
			}

			uint8_t * pTarget = static_cast<uint8_t *>(rPublic.Sections[kFixup.To.SectionIndex]);
			void * pValue = pTarget ? pTarget + kFixup.To.Offset : NULL;
			memcpy(pFrom + kFixup.FromOffset, &pValue, sizeof(pValue));
		}
	}

	const granny_grn_reference & rRoot = rPublic.Header->RootObject;
	if (rRoot.SectionIndex >= sectionCount || !rPublic.Sections[rRoot.SectionIndex]
		|| !RangeOk(rRoot.Offset, sizeof(granny_file_info), pSections[rRoot.SectionIndex].ExpandedDataSize))
	{
		CYNEX_LOG(GrannyErrorLogMessage, GrannyFileReadingLogMessage, "Granny file has no root object");
		DestroyFile(pFile);
		return NULL;
	}

	return &rPublic;
}

GRANNY_DYNLINK(granny_file_info *) GrannyGetFileInfo(granny_file * File)
{
	if (!File || !File->Header)
		return NULL;

	const granny_grn_reference & rRoot = File->Header->RootObject;
	if ((granny_int32x) rRoot.SectionIndex >= File->SectionCount || !File->Sections[rRoot.SectionIndex])
		return NULL;

	return reinterpret_cast<granny_file_info *>(static_cast<uint8_t *>(File->Sections[rRoot.SectionIndex]) + rRoot.Offset);
}

GRANNY_DYNLINK(void) GrannyFreeFileSection(granny_file * File, granny_int32x SectionIndex)
{
	if (!File || SectionIndex < 0 || SectionIndex >= File->SectionCount)
		return;

	SFile * pFile = ToFile(File);
	if (!File->Sections[SectionIndex])
		return;

	if (!File->IsUserMemory[SectionIndex])
	{
		SSectionRange & rRange = pFile->pRanges[SectionIndex];
		// Cynex: iskelet ve track group'lar ana bolumde (gr2conv), digerleri sadece vertex/index/doku
		if (SectionIndex == GrannyStandardMainSection)
			OnMemoryRangeFreed(rRange.pBegin, rRange.size);
		AlignedFree(File->Sections[SectionIndex]);
		rRange.pBegin = NULL;
		rRange.size = 0;
	}
	File->Sections[SectionIndex] = NULL;
}

GRANNY_DYNLINK(void) GrannyFreeFile(granny_file * File)
{
	DestroyFile(ToFile(File));
}
