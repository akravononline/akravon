// Cynex: android girdi durumu - tus tablosu ve dokunmalar
#include "CynexInput.h"
#include <string.h>
#include <mutex>
#include <time.h>

namespace
{
	std::mutex g_lock;
	unsigned char g_keys[256];
	SCynexTouch g_touches[CYNEX_MAX_TOUCH];
	int g_touchCount = 0;

	uint32_t NowMs()
	{
		timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return (uint32_t)(ts.tv_sec * 1000ull + ts.tv_nsec / 1000000ull);
	}

	int FindTouch(int id)
	{
		for (int i = 0; i < g_touchCount; ++i)
			if (g_touches[i].id == id)
				return i;
		return -1;
	}

	// Cynex: kalkan/iptal edilen dokunmalar bir kare okunabilsin, sonraki basista temizlenir
	void PurgeEnded()
	{
		int n = 0;
		for (int i = 0; i < g_touchCount; ++i)
			if (g_touches[i].phase < 2)
				g_touches[n++] = g_touches[i];
		g_touchCount = n;
	}
}

extern "C" {

void CynexInput_SetKey(int dik, int down)
{
	if (dik <= 0 || dik > 255)
		return;
	std::lock_guard<std::mutex> l(g_lock);
	g_keys[dik] = down ? 0x80 : 0;
}

void CynexInput_GetKeyboardState(unsigned char state[256])
{
	std::lock_guard<std::mutex> l(g_lock);
	memcpy(state, g_keys, sizeof(g_keys));
}

void CynexInput_ReleaseAllKeys(void)
{
	std::lock_guard<std::mutex> l(g_lock);
	memset(g_keys, 0, sizeof(g_keys));
	g_touchCount = 0;
}

void CynexInput_TouchBegin(int id, float x, float y)
{
	std::lock_guard<std::mutex> l(g_lock);
	PurgeEnded();
	int i = FindTouch(id);
	if (i < 0)
	{
		if (g_touchCount >= CYNEX_MAX_TOUCH)
			return;
		i = g_touchCount++;
	}
	SCynexTouch& t = g_touches[i];
	t.id = id;
	t.x = t.downX = x;
	t.y = t.downY = y;
	t.downTime = NowMs();
	t.phase = 0;
}

void CynexInput_TouchMove(int id, float x, float y)
{
	std::lock_guard<std::mutex> l(g_lock);
	const int i = FindTouch(id);
	if (i < 0 || g_touches[i].phase >= 2)
		return;
	g_touches[i].x = x;
	g_touches[i].y = y;
	g_touches[i].phase = 1;
}

void CynexInput_TouchEnd(int id, float x, float y, int cancelled)
{
	std::lock_guard<std::mutex> l(g_lock);
	const int i = FindTouch(id);
	if (i < 0)
		return;
	g_touches[i].x = x;
	g_touches[i].y = y;
	g_touches[i].phase = cancelled ? 3 : 2;
}

int CynexInput_GetTouchCount(void)
{
	std::lock_guard<std::mutex> l(g_lock);
	return g_touchCount;
}

int CynexInput_GetTouch(int index, struct SCynexTouch* out)
{
	std::lock_guard<std::mutex> l(g_lock);
	if (index < 0 || index >= g_touchCount || !out)
		return 0;
	*out = g_touches[index];
	return 1;
}

}	// extern "C"
