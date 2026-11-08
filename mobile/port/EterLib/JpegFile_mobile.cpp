// Cynex: EterLib/JpegFile.cpp mobil karsiligi - libjpeg yerine stb
#include "../../../client/source/EterLib/StdAfx.h"
#include "../../../client/source/EterLib/JpegFile.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO_FALLBACK_
#include "stb_image_write.h"
#include "stb_image.h"

#include <vector>

namespace
{
	void WriteToFile(void* ctx, void* data, int size)
	{
		fwrite(data, 1, size, (FILE*)ctx);
	}

	struct SMemWriter
	{
		unsigned char* dest;
		int capacity;
		int size;
	};

	void WriteToMem(void* ctx, void* data, int size)
	{
		SMemWriter* w = (SMemWriter*)ctx;
		const int n = (w->size + size <= w->capacity) ? size : w->capacity - w->size;
		if (n > 0)
			memcpy(w->dest + w->size, data, n);
		w->size += size;
	}
}

int jpeg_save_to_file(unsigned char* data, int width, int height, int quality, FILE* fi)
{
	if (!fi)
		return 0;
	return stbi_write_jpg_to_func(WriteToFile, fi, width, height, 3, data, quality) ? 1 : 0;
}

int jpeg_save(unsigned char* data, int width, int height, int quality, const char* filename)
{
	FILE* fi = fopen(filename, "wb");
	if (!fi)
		return 0;
	const int r = jpeg_save_to_file(data, width, height, quality, fi);
	fclose(fi);
	return r;
}

int jpeg_save_to_mem(unsigned char* data, int width, int height, int quality, unsigned char* dest, int destsize)
{
	SMemWriter w = { dest, destsize, 0 };
	if (!stbi_write_jpg_to_func(WriteToMem, &w, width, height, 3, data, quality) || w.size > destsize)
		return 0;
	return w.size;
}

int jpeg_load(const char* filename, unsigned char** dest, int* width, int* height)
{
	int comp = 0;
	unsigned char* pixels = stbi_load(filename, width, height, &comp, 3);
	if (!pixels)
		return 0;
	const size_t size = size_t(*width) * size_t(*height) * 3;
	*dest = (unsigned char*)malloc(size);
	memcpy(*dest, pixels, size);
	stbi_image_free(pixels);
	return 1;
}

int jpeg_load_from_mem(unsigned char* _data, int size, unsigned char* dest, int width, int height)
{
	int w = 0, h = 0, comp = 0;
	unsigned char* pixels = stbi_load_from_memory(_data, size, &w, &h, &comp, 3);
	if (!pixels)
		return 0;
	if (w != width || h != height)
	{
		stbi_image_free(pixels);
		return 0;
	}
	memcpy(dest, pixels, size_t(w) * size_t(h) * 3);
	stbi_image_free(pixels);
	return 1;
}
