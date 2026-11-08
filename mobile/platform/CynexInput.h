// Cynex: android girdi katmani - dokunma, klavye, yazi girisi
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// klavye (DIK kodlari) - Input_mobile.cpp her karede okur
void CynexInput_SetKey(int dik, int down);
void CynexInput_GetKeyboardState(unsigned char state[256]);
void CynexInput_ReleaseAllKeys(void);

// dokunma - app.GetTouch* python fonksiyonlari
enum { CYNEX_MAX_TOUCH = 10 };

struct SCynexTouch
{
	int32_t id;
	float x;
	float y;
	float downX;
	float downY;
	uint32_t downTime;
	int32_t phase;		// 0 basti, 1 hareket, 2 kalkti, 3 iptal
};

void CynexInput_TouchBegin(int id, float x, float y);
void CynexInput_TouchMove(int id, float x, float y);
void CynexInput_TouchEnd(int id, float x, float y, int cancelled);
int CynexInput_GetTouchCount(void);
int CynexInput_GetTouch(int index, struct SCynexTouch* out);

#ifdef __cplusplus
}
#endif
