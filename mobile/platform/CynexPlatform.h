// Cynex: android platform hizmetleri (pencere, olay pompasi, java koprusu)
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct android_app;
struct ANativeWindow;

void CynexPlatform_Init(struct android_app* app);
struct android_app* CynexPlatform_GetApp(void);
struct ANativeWindow* CynexPlatform_GetWindow(void);

// Cynex: oyunun cizim boyutu (pencere boyutu). ekran farkliysa yuzey olceklenir, dokunmalar cevrilir
int CynexPlatform_GetWidth(void);
int CynexPlatform_GetHeight(void);
void CynexPlatform_SetRenderSize(int width, int height);
// Cynex: cihaz ekraninin gercek boyutu
int CynexPlatform_GetScreenWidth(void);
int CynexPlatform_GetScreenHeight(void);
// AKRAVON: android ekran yogunlugu (dpi); bilinmiyorsa 0
int CynexPlatform_GetDensityDpi(void);
// AKRAVON: centik / yuvarlak kose guvenli alani (fiziksel piksel): sol, ust, sag, alt
void CynexPlatform_GetSafeInsets(int* out4);

int CynexPlatform_HasFocus(void);
int CynexPlatform_IsDestroyRequested(void);

// Cynex: looper olaylarini isle, win32 mesajlarina cevir. timeoutMs: 0 bekleme yok, -1 olay gelene kadar
void CynexPlatform_PumpEvents(int timeoutMs);

// win32 mesaj kuyrugu (compat/winuser.cpp kullanir)
void CynexPlatform_PostMessage(uint32_t msg, uintptr_t wParam, intptr_t lParam);
int CynexPlatform_PeekMessage(uint32_t* msg, uintptr_t* wParam, intptr_t* lParam, int remove);
int CynexPlatform_GetMousePos(int* x, int* y);
// Cynex: GetCursorPos cagirir - bekleyen tiklamalari serbest birakir
void CynexPlatform_MarkCursorRead(void);
void CynexPlatform_SetMousePos(int x, int y);
int CynexPlatform_IsVKeyDown(int vk);

// pencere degisimi dinleyicisi (d3d8 katmani yuzeyi yeniden kurar)
typedef void (*CynexWindowCallback)(struct ANativeWindow* window, int created);
void CynexPlatform_SetWindowCallback(CynexWindowCallback cb);

void CynexPlatform_OpenURL(const char* url);
void CynexPlatform_Vibrate(int milliseconds);
void CynexPlatform_ShowKeyboard(int show);
void CynexPlatform_SetClipboardText(const char* utf8);
const char* CynexPlatform_GetClipboardText(void);
const char* CynexPlatform_GetDataPath(void);
const char* CynexPlatform_GetDeviceId(void);
void CynexPlatform_ShowMessage(const char* title, const char* text);

// AKRAVON: galeriden gorsel sec (lonca imzasi / simgesi). kind 0 = imza (BMP), 1 = simge (JPEG); gorsel ortadan kirpilip width x height'a kucultulur
// ve destPath'e kaydedilir (goreli yol = oyun klasoru). durum: 0 bosta, 1 secim suruyor, 2 kaydedildi, 3 iptal / hata
void CynexPlatform_PickImage(int kind, int width, int height, const char* destPath);
int CynexPlatform_PickImageState(void);

#ifdef __cplusplus
}
#endif
