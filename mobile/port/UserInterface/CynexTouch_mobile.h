// Cynex: dokunmatik katman - ortak arayuz
#pragma once

struct SCynexTouchConfig
{
	int mobileHud = 1;
	float deadZone = 0.15f;		// joystick yaricapinin orani
	float joystickSize = 0.0f;	// 0 = ekran boyuna gore
	float attackSize = 0.0f;
	float returnPitch = 14.0f;	// surukleme bitince kameranin dondugu egim
	int cameraReturn = 1;
	float dragSens = 1.0f;
	float pinchSens = 1.0f;
	int postFilter = 1;		// AKRAVON: 0 kapali, 1 dogal, 2 canli
	int viewRange = 0;		// AKRAVON: 0 uzak, 1 orta, 2 kisa
	int textureQuality = 2;	// AKRAVON: 0 dusuk, 1 orta, 2 yuksek
	int fpsLimit = 60;		// AKRAVON: 30 / 60
};

enum
{
	CYNEX_TOUCH_REGION_WORLD = 0,
	CYNEX_TOUCH_REGION_UI = 1,
	CYNEX_TOUCH_REGION_NATIVE = 2,
};

SCynexTouchConfig& CynexTouch_Config();
void CynexTouch_SaveConfig();

void CynexTouch_Update();
bool CynexTouch_FilterMouseButton(int iEvent, int x, int y);	// 0 bas, 1 birak, 2 cift tik
int CynexTouch_GetActiveCount();
bool CynexTouch_GetActive(int index, int* piID, int* piX, int* piY, int* piRegion);
void CynexTouch_SetDoubleTapGate(bool bEnable);
void CynexTouch_StartCameraReturn();	// Cynex: kamerayi ayarlanan egime yumusakca dondur
