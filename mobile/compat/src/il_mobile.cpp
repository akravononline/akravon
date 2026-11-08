// Cynex: devil yerine stb - lonca isareti yukle/kaydet icin gereken kadari
#include <windows.h>
#include <IL/il.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <map>
#include <vector>
#include <string.h>
#include "cynex_handle.h"

namespace
{
	struct SImage
	{
		ILuint width = 0;
		ILuint height = 0;
		std::vector<unsigned char> rgba;	// Cynex: hep ust-sol baslangicli rgba
	};

	std::map<ILuint, SImage> g_images;
	ILuint g_next = 1;
	ILuint g_bound = 0;

	SImage* Bound()
	{
		auto it = g_images.find(g_bound);
		return it == g_images.end() ? nullptr : &it->second;
	}

	// Cynex: BGRA <-> RGBA yalnizca R/B yer degistirir
	void CopyRect(SImage* img, ILuint x, ILuint y, ILuint w, ILuint h, ILenum format, unsigned char* data, bool toImage)
	{
		const bool bgra = format == IL_BGRA;
		const int ch = (format == IL_RGB) ? 3 : 4;
		for (ILuint j = 0; j < h; ++j)
		{
			if (y + j >= img->height)
				break;
			for (ILuint i = 0; i < w; ++i)
			{
				if (x + i >= img->width)
					break;
				unsigned char* p = &img->rgba[((size_t)(y + j) * img->width + (x + i)) * 4];
				unsigned char* d = data + ((size_t)j * w + i) * ch;
				if (toImage)
				{
					p[0] = bgra ? d[2] : d[0];
					p[1] = d[1];
					p[2] = bgra ? d[0] : d[2];
					p[3] = ch == 4 ? d[3] : 255;
				}
				else
				{
					d[0] = bgra ? p[2] : p[0];
					d[1] = p[1];
					d[2] = bgra ? p[0] : p[2];
					if (ch == 4)
						d[3] = p[3];
				}
			}
		}
	}
}

extern "C" {

void ILAPIENTRY ilInit(void) {}
void ILAPIENTRY ilShutDown(void) { g_images.clear(); g_bound = 0; }
ILboolean ILAPIENTRY ilEnable(ILenum) { return IL_TRUE; }
ILboolean ILAPIENTRY ilOriginFunc(ILenum) { return IL_TRUE; }

void ILAPIENTRY ilGenImages(ILsizei num, ILuint* images)
{
	for (ILsizei i = 0; i < num; ++i)
	{
		images[i] = g_next++;
		g_images[images[i]];
	}
}

void ILAPIENTRY ilBindImage(ILuint image)
{
	g_bound = image;
	g_images[image];
}

void ILAPIENTRY ilDeleteImages(ILsizei num, const ILuint* images)
{
	for (ILsizei i = 0; i < num; ++i)
	{
		g_images.erase(images[i]);
		if (g_bound == images[i])
			g_bound = 0;
	}
}

ILboolean ILAPIENTRY ilLoad(ILenum, ILconst_string fileName)
{
	SImage* img = Bound();
	if (!img || !fileName)
		return IL_FALSE;
	const std::string path = cynex::ResolvePath(fileName);
	int w, h, n;
	unsigned char* p = stbi_load(path.c_str(), &w, &h, &n, 4);
	if (!p)
		return IL_FALSE;
	img->width = (ILuint)w;
	img->height = (ILuint)h;
	img->rgba.assign(p, p + (size_t)w * h * 4);
	stbi_image_free(p);
	return IL_TRUE;
}

ILboolean ILAPIENTRY ilSave(ILenum, ILconst_string fileName)
{
	SImage* img = Bound();
	if (!img || !fileName || img->rgba.empty())
		return IL_FALSE;
	const std::string path = cynex::ResolvePath(fileName, true);
	stbi_write_tga_with_rle = 0;
	return stbi_write_tga(path.c_str(), (int)img->width, (int)img->height, 4, img->rgba.data()) ? IL_TRUE : IL_FALSE;
}

ILint ILAPIENTRY ilGetInteger(ILenum mode)
{
	SImage* img = Bound();
	if (!img)
		return 0;
	switch (mode)
	{
		case IL_IMAGE_WIDTH: return (ILint)img->width;
		case IL_IMAGE_HEIGHT: return (ILint)img->height;
	}
	return 0;
}

ILboolean ILAPIENTRY ilConvertImage(ILenum, ILenum)
{
	// Cynex: ic bicim sabit, donusum kopyalarken yapiliyor
	return Bound() ? IL_TRUE : IL_FALSE;
}

ILboolean ILAPIENTRY ilTexImage(ILuint width, ILuint height, ILuint, ILubyte, ILenum format, ILenum, void* data)
{
	SImage* img = Bound();
	if (!img)
		return IL_FALSE;
	img->width = width;
	img->height = height;
	img->rgba.assign((size_t)width * height * 4, 0);
	if (data)
		CopyRect(img, 0, 0, width, height, format, (unsigned char*)data, true);
	return IL_TRUE;
}

void ILAPIENTRY ilSetPixels(ILint x, ILint y, ILint, ILuint width, ILuint height, ILuint, ILenum format, ILenum, void* data)
{
	SImage* img = Bound();
	if (img && data && x >= 0 && y >= 0)
		CopyRect(img, (ILuint)x, (ILuint)y, width, height, format, (unsigned char*)data, true);
}

ILuint ILAPIENTRY ilCopyPixels(ILuint x, ILuint y, ILuint, ILuint width, ILuint height, ILuint, ILenum format, ILenum, void* data)
{
	SImage* img = Bound();
	if (!img || !data)
		return 0;
	CopyRect(img, x, y, width, height, format, (unsigned char*)data, false);
	return width * height * (format == IL_RGB ? 3 : 4);
}

}	// extern "C"
