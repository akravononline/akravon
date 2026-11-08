#include "StdAfx.h"
#include "tea.h"
#include <memory.h>



#define TEA_ROUND		32
#define DELTA			0x9E3779B9

void tea_code(const DWORD sz, const DWORD sy, const DWORD *key, DWORD *dest)
{
	register DWORD y = sy, z = sz, sum = 0;
	DWORD		n = TEA_ROUND;

	while (n-- > 0)
	{
		y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
		sum	+= DELTA;
		z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
	}

	*(dest++)	= y;
	*dest	= z;
}

void tea_decode(const DWORD sz, const DWORD sy, const DWORD *key, DWORD *dest)
{
#pragma warning(disable:4307)
	register DWORD y = sy, z = sz, sum = DELTA * TEA_ROUND;
#pragma warning(default:4307)

	DWORD		n = TEA_ROUND;

	while (n-- > 0)
	{
		z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
		sum -= DELTA;
		y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
	}

	*(dest++)	= y;
	*dest	= z;
}

int tea_encrypt(DWORD *dest, const DWORD *src, const DWORD * key, int size)
{
	int		i;
	int		resize;

	if (size % 8 != 0)
	{
		resize = size + 8 - (size % 8);
		memset((char *) src + size, 0, resize - size);
	}
	else
		resize = size;

	for (i = 0; i < resize >> 3; i++, dest += 2, src += 2)
		tea_code(*(src + 1), *src, key, dest);

	return (resize);
}

int tea_decrypt(DWORD *dest, const DWORD *src, const DWORD * key, int size)
{
	int		i;
	int		resize;

	if (size % 8 != 0)
		resize = size + 8 - (size % 8);
	else
		resize = size;

	for (i = 0; i < resize >> 3; i++, dest += 2, src += 2)
		tea_decode(*(src + 1), *src, key, dest);

	return (resize);
}
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240
