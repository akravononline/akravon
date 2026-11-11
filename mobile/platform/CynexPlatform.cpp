// Cynex: android yasam dongusu + girdi -> win32 mesaj kuyrugu, java koprusu
#include "CynexPlatform.h"
#include "CynexInput.h"
#include <android_native_app_glue.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/configuration.h>
#include <jni.h>
#include <atomic>
#include <deque>
#include <mutex>
#include <string>
#include <string.h>
#include <windows.h>

extern "C" void CynexMss_Suspend(void);
extern "C" void CynexMss_Resume(void);

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "CynexMobil", __VA_ARGS__)

namespace
{
	struct SMessage
	{
		uint32_t msg;
		uintptr_t wParam;
		intptr_t lParam;
		uint32_t cursorSeq;		// Cynex: oyun imleci okuyana kadar bekler
	};

	android_app* g_app = nullptr;
	ANativeWindow* g_window = nullptr;
	CynexWindowCallback g_windowCallback = nullptr;
	bool g_focus = false;

	int g_screenW = 0, g_screenH = 0;
	int g_renderW = 0, g_renderH = 0;

	std::mutex g_queueLock;
	std::deque<SMessage> g_queue;

	std::atomic<int> g_mouseX{0}, g_mouseY{0};
	std::atomic<uint32_t> g_cursorSeq{0};
	unsigned char g_vkeys[256];

	int g_primaryPointer = -1;

	std::string g_dataPath, g_deviceId, g_clipboard;

	// ---------------------------------------------------------------- jni
	JNIEnv* Env()
	{
		static thread_local JNIEnv* s_env = nullptr;
		if (!s_env && g_app)
			g_app->activity->vm->AttachCurrentThread(&s_env, nullptr);
		return s_env;
	}

	jmethodID Method(JNIEnv* env, const char* name, const char* sig)
	{
		jclass cls = env->GetObjectClass(g_app->activity->clazz);
		jmethodID id = env->GetMethodID(cls, name, sig);
		env->DeleteLocalRef(cls);
		if (!id)
			env->ExceptionClear();
		return id;
	}

	void CallVoid(const char* name, const char* sig, ...)
	{
		JNIEnv* env = Env();
		if (!env)
			return;
		jmethodID id = Method(env, name, sig);
		if (!id)
			return;
		va_list va;
		va_start(va, sig);
		env->CallVoidMethodV(g_app->activity->clazz, id, va);
		va_end(va);
		if (env->ExceptionCheck())
			env->ExceptionClear();
	}

	std::string CallString(const char* name)
	{
		JNIEnv* env = Env();
		if (!env)
			return std::string();
		jmethodID id = Method(env, name, "()Ljava/lang/String;");
		if (!id)
			return std::string();
		jstring s = (jstring)env->CallObjectMethod(g_app->activity->clazz, id);
		if (env->ExceptionCheck())
			env->ExceptionClear();
		std::string out;
		if (s)
		{
			const char* p = env->GetStringUTFChars(s, nullptr);
			out = p ? p : "";
			env->ReleaseStringUTFChars(s, p);
			env->DeleteLocalRef(s);
		}
		return out;
	}

	int CallInt(const char* name, const char* sig, ...)
	{
		JNIEnv* env = Env();
		if (!env)
			return 0;
		jmethodID id = Method(env, name, sig);
		if (!id)
			return 0;
		va_list va;
		va_start(va, sig);
		const jint r = env->CallIntMethodV(g_app->activity->clazz, id, va);
		va_end(va);
		if (env->ExceptionCheck())
		{
			env->ExceptionClear();
			return 0;
		}
		return r;
	}

	// ---------------------------------------------------------------- kuyruk
	void Post(uint32_t msg, uintptr_t w, intptr_t l, bool waitCursor = false)
	{
		std::lock_guard<std::mutex> lk(g_queueLock);
		g_queue.push_back({ msg, w, l, waitCursor ? g_cursorSeq.load() + 1 : 0 });
	}

	// Cynex: unicode harf -> oyunun kod sayfasinda WM_CHAR
	void PostChar(wchar_t wc)
	{
		if (wc < 32)
			return;
		char mb[8];
		const int n = WideCharToMultiByte(CP_ACP, 0, &wc, 1, mb, sizeof(mb), nullptr, nullptr);
		for (int j = 0; j < n; ++j)
			Post(WM_CHAR, (unsigned char)mb[j], 1);
	}

	LPARAM MouseLParam()
	{
		return MAKELPARAM((WORD)g_mouseX.load(), (WORD)g_mouseY.load());
	}

	WPARAM MouseKeys()
	{
		WPARAM k = 0;
		if (g_vkeys[VK_LBUTTON]) k |= MK_LBUTTON;
		if (g_vkeys[VK_RBUTTON]) k |= MK_RBUTTON;
		if (g_vkeys[VK_SHIFT]) k |= MK_SHIFT;
		if (g_vkeys[VK_CONTROL]) k |= MK_CONTROL;
		return k;
	}

	void SetMouseFromWindow(float x, float y)
	{
		// Cynex: ekran koordinati -> cizim koordinati
		if (g_screenW > 0 && g_renderW > 0 && g_screenW != g_renderW)
			x = x * g_renderW / g_screenW;
		if (g_screenH > 0 && g_renderH > 0 && g_screenH != g_renderH)
			y = y * g_renderH / g_screenH;
		g_mouseX = (int)x;
		g_mouseY = (int)y;
	}

	void PostSize(bool minimized)
	{
		const int w = g_renderW ? g_renderW : g_screenW;
		const int h = g_renderH ? g_renderH : g_screenH;
		Post(WM_SIZE, minimized ? SIZE_MINIMIZED : SIZE_RESTORED, minimized ? 0 : MAKELPARAM(w, h));
	}

	void ApplyGeometry()
	{
		if (!g_window)
			return;
		g_screenW = ANativeWindow_getWidth(g_window);
		g_screenH = ANativeWindow_getHeight(g_window);
		if (g_renderW > 0 && g_renderH > 0 && (g_renderW != g_screenW || g_renderH != g_screenH))
			ANativeWindow_setBuffersGeometry(g_window, g_renderW, g_renderH, 0);
		else
			ANativeWindow_setBuffersGeometry(g_window, 0, 0, 0);
	}

	// ---------------------------------------------------------------- tuslar
	struct SKeyMap { int vk; int dik; };

	SKeyMap MapKey(int32_t code)
	{
		static const unsigned char s_letterDik[26] = {
			0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32,
			0x31, 0x18, 0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C };
		static const unsigned char s_numpadDik[10] = { 0x52, 0x4F, 0x50, 0x51, 0x4B, 0x4C, 0x4D, 0x47, 0x48, 0x49 };

		if (code >= AKEYCODE_A && code <= AKEYCODE_Z)
			return { 'A' + (code - AKEYCODE_A), s_letterDik[code - AKEYCODE_A] };
		if (code >= AKEYCODE_0 && code <= AKEYCODE_9)
			return { '0' + (code - AKEYCODE_0), code == AKEYCODE_0 ? 0x0B : 0x02 + (code - AKEYCODE_1) };
		if (code >= AKEYCODE_F1 && code <= AKEYCODE_F12)
		{
			const int i = code - AKEYCODE_F1;
			return { VK_F1 + i, i < 10 ? 0x3B + i : 0x57 + (i - 10) };
		}
		if (code >= AKEYCODE_NUMPAD_0 && code <= AKEYCODE_NUMPAD_9)
			return { VK_NUMPAD0 + (code - AKEYCODE_NUMPAD_0), s_numpadDik[code - AKEYCODE_NUMPAD_0] };

		switch (code)
		{
			case AKEYCODE_BACK:			return { VK_ESCAPE, 0x01 };	// Cynex: geri tusu = esc
			case AKEYCODE_ESCAPE:		return { VK_ESCAPE, 0x01 };
			case AKEYCODE_ENTER:		return { VK_RETURN, 0x1C };
			case AKEYCODE_NUMPAD_ENTER:	return { VK_RETURN, 0x9C };
			case AKEYCODE_DEL:			return { VK_BACK, 0x0E };
			case AKEYCODE_FORWARD_DEL:	return { VK_DELETE, 0xD3 };
			case AKEYCODE_TAB:			return { VK_TAB, 0x0F };
			case AKEYCODE_SPACE:		return { VK_SPACE, 0x39 };
			case AKEYCODE_DPAD_UP:		return { VK_UP, 0xC8 };
			case AKEYCODE_DPAD_DOWN:	return { VK_DOWN, 0xD0 };
			case AKEYCODE_DPAD_LEFT:	return { VK_LEFT, 0xCB };
			case AKEYCODE_DPAD_RIGHT:	return { VK_RIGHT, 0xCD };
			case AKEYCODE_SHIFT_LEFT:	return { VK_SHIFT, 0x2A };
			case AKEYCODE_SHIFT_RIGHT:	return { VK_SHIFT, 0x36 };
			case AKEYCODE_CTRL_LEFT:	return { VK_CONTROL, 0x1D };
			case AKEYCODE_CTRL_RIGHT:	return { VK_CONTROL, 0x9D };
			case AKEYCODE_ALT_LEFT:		return { VK_MENU, 0x38 };
			case AKEYCODE_ALT_RIGHT:	return { VK_MENU, 0xB8 };
			case AKEYCODE_CAPS_LOCK:	return { VK_CAPITAL, 0x3A };
			case AKEYCODE_PAGE_UP:		return { VK_PRIOR, 0xC9 };
			case AKEYCODE_PAGE_DOWN:	return { VK_NEXT, 0xD1 };
			case AKEYCODE_MOVE_HOME:	return { VK_HOME, 0xC7 };
			case AKEYCODE_MOVE_END:		return { VK_END, 0xCF };
			case AKEYCODE_INSERT:		return { VK_INSERT, 0xD2 };
			case AKEYCODE_MINUS:		return { 0xBD, 0x0C };
			case AKEYCODE_EQUALS:		return { 0xBB, 0x0D };
			case AKEYCODE_GRAVE:		return { 0xC0, 0x29 };
			case AKEYCODE_COMMA:		return { 0xBC, 0x33 };
			case AKEYCODE_PERIOD:		return { 0xBE, 0x34 };
			case AKEYCODE_SLASH:		return { 0xBF, 0x35 };
			case AKEYCODE_SEMICOLON:	return { 0xBA, 0x27 };
			case AKEYCODE_APOSTROPHE:	return { 0xDE, 0x28 };
			case AKEYCODE_LEFT_BRACKET:	return { 0xDB, 0x1A };
			case AKEYCODE_RIGHT_BRACKET:return { 0xDD, 0x1B };
			case AKEYCODE_BACKSLASH:	return { 0xDC, 0x2B };
			default:					return { 0, 0 };
		}
	}

	int32_t OnKey(AInputEvent* e)
	{
		const int32_t code = AKeyEvent_getKeyCode(e);
		const int32_t action = AKeyEvent_getAction(e);
		if (code == AKEYCODE_VOLUME_UP || code == AKEYCODE_VOLUME_DOWN || code == AKEYCODE_VOLUME_MUTE)
			return 0;	// Cynex: ses tuslari sisteme kalsin

		const SKeyMap k = MapKey(code);
		if (action == AKEY_EVENT_ACTION_MULTIPLE)
			return k.vk ? 1 : 0;

		// Cynex: yazilabilir karakter (harita disi tuslar dahil, orn. trk klavye) java KeyCharacterMap'ten
		if (action == AKEY_EVENT_ACTION_DOWN && !g_vkeys[VK_CONTROL] && !g_vkeys[VK_MENU])
		{
			const int uc = CallInt("keyToChar", "(III)I", (jint)AInputEvent_getDeviceId(e), (jint)code, (jint)AKeyEvent_getMetaState(e));
			if (uc >= 32)
				PostChar((wchar_t)uc);
		}
		if (!k.vk)
			return 0;

		const bool down = action == AKEY_EVENT_ACTION_DOWN;
		const bool extended = (k.dik & 0x80) != 0;
		LPARAM l = 1 | ((k.dik & 0x7F) << 16) | (extended ? (1 << 24) : 0);
		if (!down)
			l |= (1u << 30) | (1u << 31);
		else if (g_vkeys[k.vk])
			l |= (1 << 30);

		g_vkeys[k.vk] = down ? 0x80 : 0;
		CynexInput_SetKey(k.dik, down);

		const bool alt = g_vkeys[VK_MENU] && k.vk != VK_MENU;
		Post(alt ? (down ? WM_SYSKEYDOWN : WM_SYSKEYUP) : (down ? WM_KEYDOWN : WM_KEYUP), k.vk, l);

		// Cynex: kontrol karakterleri
		if (down && !alt)
		{
			int ch = 0;
			switch (k.vk)
			{
				case VK_BACK: ch = 8; break;
				case VK_TAB: ch = 9; break;
				case VK_RETURN: ch = 13; break;
				case VK_ESCAPE: ch = 27; break;
			}
			if (ch)
				Post(WM_CHAR, ch, l);
		}
		return 1;
	}

	// ---------------------------------------------------------------- dokunma / fare
	int32_t OnMotion(AInputEvent* e)
	{
		const int32_t action = AMotionEvent_getAction(e);
		const int32_t masked = action & AMOTION_EVENT_ACTION_MASK;
		const size_t index = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
		const int32_t source = AInputEvent_getSource(e);

		// Cynex: gercek fare (ldplayer / otg) - sag tik ve tekerlek de calissin
		if ((source & AINPUT_SOURCE_MOUSE) == AINPUT_SOURCE_MOUSE)
		{
			SetMouseFromWindow(AMotionEvent_getX(e, 0), AMotionEvent_getY(e, 0));
			static int32_t s_buttons = 0;
			const int32_t buttons = AMotionEvent_getButtonState(e);
			const int32_t changed = buttons ^ s_buttons;
			if (changed & AMOTION_EVENT_BUTTON_PRIMARY)
			{
				const bool down = (buttons & AMOTION_EVENT_BUTTON_PRIMARY) != 0;
				g_vkeys[VK_LBUTTON] = down ? 0x80 : 0;
				Post(down ? WM_LBUTTONDOWN : WM_LBUTTONUP, MouseKeys(), MouseLParam(), true);
			}
			if (changed & AMOTION_EVENT_BUTTON_SECONDARY)
			{
				const bool down = (buttons & AMOTION_EVENT_BUTTON_SECONDARY) != 0;
				g_vkeys[VK_RBUTTON] = down ? 0x80 : 0;
				Post(down ? WM_RBUTTONDOWN : WM_RBUTTONUP, MouseKeys(), MouseLParam(), true);
			}
			if (changed & AMOTION_EVENT_BUTTON_TERTIARY)
			{
				const bool down = (buttons & AMOTION_EVENT_BUTTON_TERTIARY) != 0;
				Post(down ? WM_MBUTTONDOWN : WM_MBUTTONUP, MouseKeys(), MouseLParam(), true);
			}
			s_buttons = buttons;

			if (masked == AMOTION_EVENT_ACTION_SCROLL)
			{
				const float v = AMotionEvent_getAxisValue(e, AMOTION_EVENT_AXIS_VSCROLL, 0);
				if (v != 0.0f)
					Post(WM_MOUSEWHEEL, MAKEWPARAM(MouseKeys(), (WORD)(short)(v * WHEEL_DELTA)), MouseLParam());
			}
			// Cynex: butonsuz fare tiklamasi (eski cihazlar) dokunma gibi islenir
			if (buttons || masked == AMOTION_EVENT_ACTION_HOVER_MOVE || masked == AMOTION_EVENT_ACTION_SCROLL)
				return 1;
		}

		switch (masked)
		{
			case AMOTION_EVENT_ACTION_DOWN:
			case AMOTION_EVENT_ACTION_POINTER_DOWN:
			{
				const int id = AMotionEvent_getPointerId(e, index);
				const float x = AMotionEvent_getX(e, index), y = AMotionEvent_getY(e, index);
				CynexInput_TouchBegin(id, x, y);
				if (g_primaryPointer < 0)
				{
					// Cynex: ilk parmak sol fare tusu
					g_primaryPointer = id;
					SetMouseFromWindow(x, y);
					g_vkeys[VK_LBUTTON] = 0x80;
					Post(WM_LBUTTONDOWN, MouseKeys(), MouseLParam(), true);
				}
				break;
			}

			case AMOTION_EVENT_ACTION_MOVE:
			{
				const size_t count = AMotionEvent_getPointerCount(e);
				for (size_t i = 0; i < count; ++i)
				{
					const int id = AMotionEvent_getPointerId(e, i);
					const float x = AMotionEvent_getX(e, i), y = AMotionEvent_getY(e, i);
					CynexInput_TouchMove(id, x, y);
					if (id == g_primaryPointer)
						SetMouseFromWindow(x, y);
				}
				break;
			}

			case AMOTION_EVENT_ACTION_UP:
			case AMOTION_EVENT_ACTION_POINTER_UP:
			case AMOTION_EVENT_ACTION_CANCEL:
			{
				const bool cancel = masked == AMOTION_EVENT_ACTION_CANCEL;
				const size_t count = AMotionEvent_getPointerCount(e);
				for (size_t i = 0; i < count; ++i)
				{
					if (!cancel && masked == AMOTION_EVENT_ACTION_POINTER_UP && i != index)
						continue;
					const int id = AMotionEvent_getPointerId(e, i);
					const float x = AMotionEvent_getX(e, i), y = AMotionEvent_getY(e, i);
					CynexInput_TouchEnd(id, x, y, cancel);
					if (id == g_primaryPointer)
					{
						SetMouseFromWindow(x, y);
						g_vkeys[VK_LBUTTON] = 0;
						Post(WM_LBUTTONUP, MouseKeys(), MouseLParam(), true);
						g_primaryPointer = -1;
					}
				}
				break;
			}
		}
		return 1;
	}

	int32_t OnInput(android_app*, AInputEvent* e)
	{
		switch (AInputEvent_getType(e))
		{
			case AINPUT_EVENT_TYPE_KEY: return OnKey(e);
			case AINPUT_EVENT_TYPE_MOTION: return OnMotion(e);
		}
		return 0;
	}

	// ---------------------------------------------------------------- yasam dongusu
	void OnCmd(android_app* app, int32_t cmd)
	{
		switch (cmd)
		{
			case APP_CMD_INIT_WINDOW:
				g_window = app->window;
				ApplyGeometry();
				LOGI("pencere %dx%d (cizim %dx%d)", g_screenW, g_screenH, g_renderW, g_renderH);
				if (g_windowCallback)
					g_windowCallback(g_window, 1);
				PostSize(false);
				break;

			case APP_CMD_TERM_WINDOW:
				if (g_windowCallback && g_window)
					g_windowCallback(g_window, 0);
				g_window = nullptr;
				PostSize(true);
				break;

			case APP_CMD_WINDOW_RESIZED:
			case APP_CMD_CONFIG_CHANGED:
				if (g_window)
				{
					const int w = ANativeWindow_getWidth(g_window), h = ANativeWindow_getHeight(g_window);
					if (w != g_screenW || h != g_screenH)
					{
						ApplyGeometry();
						PostSize(false);
					}
				}
				break;

			case APP_CMD_GAINED_FOCUS:
				g_focus = true;
				Post(WM_ACTIVATEAPP, WA_ACTIVE, 0);
				break;

			case APP_CMD_LOST_FOCUS:
				g_focus = false;
				memset(g_vkeys, 0, sizeof(g_vkeys));
				CynexInput_ReleaseAllKeys();
				g_primaryPointer = -1;
				Post(WM_ACTIVATEAPP, WA_INACTIVE, 0);
				break;

			case APP_CMD_PAUSE:
				CynexMss_Suspend();
				break;

			case APP_CMD_RESUME:
				CynexMss_Resume();
				break;

			case APP_CMD_DESTROY:
				Post(WM_QUIT, 0, 0);
				break;
		}
	}
}

extern "C" {

void CynexPlatform_Init(struct android_app* app)
{
	g_app = app;
	app->onAppCmd = OnCmd;
	app->onInputEvent = OnInput;

	if (app->activity->externalDataPath)
		g_dataPath = app->activity->externalDataPath;
	else if (app->activity->internalDataPath)
		g_dataPath = app->activity->internalDataPath;

	// Cynex: pencere gelmeden oyun baslamasin
	while (!g_window && !app->destroyRequested)
		CynexPlatform_PumpEvents(-1);
}

struct android_app* CynexPlatform_GetApp(void) { return g_app; }
struct ANativeWindow* CynexPlatform_GetWindow(void) { return g_window; }
int CynexPlatform_GetWidth(void) { return g_renderW ? g_renderW : g_screenW; }
int CynexPlatform_GetHeight(void) { return g_renderH ? g_renderH : g_screenH; }
int CynexPlatform_GetScreenWidth(void) { return g_screenW; }
int CynexPlatform_GetScreenHeight(void) { return g_screenH; }
int CynexPlatform_GetDensityDpi(void)
{
	if (!g_app || !g_app->config)
		return 0;
	const int d = AConfiguration_getDensity(g_app->config);
	return d > 0 && d < 2000 ? d : 0;
}
int CynexPlatform_HasFocus(void) { return g_focus; }
int CynexPlatform_IsDestroyRequested(void) { return g_app && g_app->destroyRequested; }

void CynexPlatform_SetRenderSize(int width, int height)
{
	if (width == g_renderW && height == g_renderH)
		return;
	g_renderW = width;
	g_renderH = height;
	ApplyGeometry();
}

void CynexPlatform_PumpEvents(int timeoutMs)
{
	if (!g_app)
		return;
	int events;
	android_poll_source* source;
	int timeout = timeoutMs;
	while (ALooper_pollOnce(timeout, nullptr, &events, (void**)&source) >= 0)
	{
		if (source)
			source->process(g_app, source);
		if (g_app->destroyRequested)
			break;
		timeout = 0;
	}
}

void CynexPlatform_PostMessage(uint32_t msg, uintptr_t wParam, intptr_t lParam)
{
	Post(msg, wParam, lParam);
}

int CynexPlatform_PeekMessage(uint32_t* msg, uintptr_t* wParam, intptr_t* lParam, int remove)
{
	std::lock_guard<std::mutex> lk(g_queueLock);
	if (g_queue.empty())
		return 0;
	const SMessage& m = g_queue.front();
	// Cynex: tiklama, oyun yeni imlec konumunu bir kez okuduktan sonra gitsin (ui secimi dogru pencerede olsun)
	if (m.cursorSeq && (int32_t)(g_cursorSeq.load() - m.cursorSeq) < 0)
		return 0;
	*msg = m.msg;
	*wParam = m.wParam;
	*lParam = m.lParam;
	if (remove)
		g_queue.pop_front();
	return 1;
}

int CynexPlatform_GetMousePos(int* x, int* y)
{
	*x = g_mouseX;
	*y = g_mouseY;
	return 1;
}

void CynexPlatform_MarkCursorRead(void)
{
	g_cursorSeq++;
}

void CynexPlatform_SetMousePos(int x, int y)
{
	g_mouseX = x;
	g_mouseY = y;
}

int CynexPlatform_IsVKeyDown(int vk)
{
	return vk >= 0 && vk < 256 && g_vkeys[vk];
}

void CynexPlatform_SetWindowCallback(CynexWindowCallback cb)
{
	g_windowCallback = cb;
}

void CynexPlatform_GetSafeInsets(int* out4)
{
	// AKRAVON: Window.getDecorView().getRootWindowInsets().getDisplayCutout() guvenli alan degerleri (Android 9+)
	out4[0] = out4[1] = out4[2] = out4[3] = 0;
	JNIEnv* env = Env();
	if (!env || !g_app)
		return;
	jobject act = g_app->activity->clazz;
	jclass actCls = env->GetObjectClass(act);
	jmethodID getWindow = env->GetMethodID(actCls, "getWindow", "()Landroid/view/Window;");
	env->DeleteLocalRef(actCls);
	if (!getWindow) { env->ExceptionClear(); return; }
	jobject window = env->CallObjectMethod(act, getWindow);
	if (!window || env->ExceptionCheck()) { env->ExceptionClear(); return; }
	jclass winCls = env->GetObjectClass(window);
	jmethodID getDecor = env->GetMethodID(winCls, "getDecorView", "()Landroid/view/View;");
	env->DeleteLocalRef(winCls);
	jobject decor = getDecor ? env->CallObjectMethod(window, getDecor) : nullptr;
	env->DeleteLocalRef(window);
	if (!decor || env->ExceptionCheck()) { env->ExceptionClear(); return; }
	jclass viewCls = env->GetObjectClass(decor);
	jmethodID getInsets = env->GetMethodID(viewCls, "getRootWindowInsets", "()Landroid/view/WindowInsets;");
	env->DeleteLocalRef(viewCls);
	if (!getInsets) { env->ExceptionClear(); env->DeleteLocalRef(decor); return; }
	jobject insets = env->CallObjectMethod(decor, getInsets);
	env->DeleteLocalRef(decor);
	if (!insets || env->ExceptionCheck()) { env->ExceptionClear(); return; }
	jclass insCls = env->GetObjectClass(insets);
	jmethodID getCut = env->GetMethodID(insCls, "getDisplayCutout", "()Landroid/view/DisplayCutout;");
	env->DeleteLocalRef(insCls);
	jobject cut = getCut ? env->CallObjectMethod(insets, getCut) : nullptr;
	if (!getCut) env->ExceptionClear();
	env->DeleteLocalRef(insets);
	if (!cut || env->ExceptionCheck()) { env->ExceptionClear(); return; }
	jclass cutCls = env->GetObjectClass(cut);
	const char* names[4] = { "getSafeInsetLeft", "getSafeInsetTop", "getSafeInsetRight", "getSafeInsetBottom" };
	for (int i = 0; i < 4; ++i)
	{
		jmethodID m = env->GetMethodID(cutCls, names[i], "()I");
		if (m)
			out4[i] = (int)env->CallIntMethod(cut, m);
		else
			env->ExceptionClear();
	}
	env->DeleteLocalRef(cutCls);
	env->DeleteLocalRef(cut);
}

void CynexPlatform_OpenURL(const char* url)
{
	JNIEnv* env = Env();
	if (!env || !url)
		return;
	jstring s = env->NewStringUTF(url);
	CallVoid("openUrl", "(Ljava/lang/String;)V", s);
	env->DeleteLocalRef(s);
}

void CynexPlatform_Vibrate(int milliseconds)
{
	CallVoid("vibrate", "(I)V", (jint)milliseconds);
}

void CynexPlatform_ShowKeyboard(int show)
{
	// Cynex: ayni durum tekrar gelirse java'ya gitme
	static int s_shown = -1;
	show = show ? 1 : 0;
	if (show == s_shown)
		return;
	s_shown = show;
	CallVoid("showKeyboard", "(Z)V", (jboolean)(show ? JNI_TRUE : JNI_FALSE));
}

// AKRAVON: galeri secimi android'de henuz yok (iOS once) - hemen iptal doner
static int s_pickState = 0;
void CynexPlatform_PickImage(int kind, int width, int height, const char* destPath) { s_pickState = 3; }
int CynexPlatform_PickImageState(void) { return s_pickState; }

void CynexPlatform_SetClipboardText(const char* utf8)
{
	JNIEnv* env = Env();
	if (!env)
		return;
	jstring s = env->NewStringUTF(utf8 ? utf8 : "");
	CallVoid("setClipboard", "(Ljava/lang/String;)V", s);
	env->DeleteLocalRef(s);
}

const char* CynexPlatform_GetClipboardText(void)
{
	g_clipboard = CallString("getClipboard");
	return g_clipboard.c_str();
}

const char* CynexPlatform_GetDataPath(void)
{
	return g_dataPath.c_str();
}

const char* CynexPlatform_GetDeviceId(void)
{
	if (g_deviceId.empty())
	{
		g_deviceId = CallString("getCynexDeviceId");
		if (g_deviceId.empty())
			g_deviceId = "cynex";
	}
	return g_deviceId.c_str();
}

void CynexPlatform_ShowMessage(const char* title, const char* text)
{
	__android_log_print(ANDROID_LOG_WARN, "CynexMobil", "[%s] %s", title ? title : "", text ? text : "");
	JNIEnv* env = Env();
	if (!env)
		return;
	jstring t = env->NewStringUTF(title ? title : "");
	jstring m = env->NewStringUTF(text ? text : "");
	CallVoid("showMessage", "(Ljava/lang/String;Ljava/lang/String;)V", t, m);
	env->DeleteLocalRef(t);
	env->DeleteLocalRef(m);
}

// Cynex: java klavyesinden gelen yazi -> WM_CHAR (oyunun kod sayfasinda)
JNIEXPORT void JNICALL Java_com_cynex_mobil_CynexActivity_nativeOnText(JNIEnv* env, jclass, jstring text)
{
	if (!text)
		return;
	const jsize len = env->GetStringLength(text);
	const jchar* chars = env->GetStringChars(text, nullptr);
	for (jsize i = 0; i < len; ++i)
		PostChar((wchar_t)chars[i]);
	env->ReleaseStringChars(text, chars);
}

}	// extern "C"
