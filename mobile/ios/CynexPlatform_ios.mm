// Cynex iOS: yasam dongusu + dokunma/klavye -> win32 mesaj kuyrugu (CynexPlatform.h arayuzunun iOS gerceklemesi)
// Oyun dongusu (WinMain) ayri bir iplikte calisir; UIKit olaylari ana iplikte gelir ve kuyruga yazilir.
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <QuartzCore/QuartzCore.h>
#import <AudioToolbox/AudioToolbox.h>

#include "CynexPlatform.h"
#include "CynexInput.h"
#include <atomic>
#include <deque>
#include <mutex>
#include <string>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <windows.h>
#ifdef interface
#undef interface	// objbase.h: interface=struct; Objective-C @interface'i bozar
#endif

extern "C" void CynexMss_Suspend(void);
extern "C" void CynexMss_Resume(void);
extern "C" int CynexIos_GameMain(void);	// native/main_ios.mm

#define LOGI(...) do { fprintf(stderr, "[I] AKRAVON: " __VA_ARGS__); fputc('\n', stderr); } while (0)

@class CynexView;

namespace
{
	struct SMessage
	{
		uint32_t msg;
		uintptr_t wParam;
		intptr_t lParam;
		uint32_t cursorSeq;
	};

	CynexView* g_view = nullptr;
	CynexWindowCallback g_windowCallback = nullptr;
	bool g_focus = false;
	bool g_windowAnnounced = false;	// geri cagri oyun ipliginde verilir
	std::atomic<bool> g_wantWindow{false};
	std::atomic<bool> g_wantFocus{false};

	int g_screenW = 0, g_screenH = 0;
	int g_renderW = 0, g_renderH = 0;

	std::mutex g_queueLock;
	std::deque<SMessage> g_queue;

	std::atomic<int> g_mouseX{0}, g_mouseY{0};
	std::atomic<uint32_t> g_cursorSeq{0};
	unsigned char g_vkeys[256];
	int g_primaryPointer = -1;

	std::string g_dataPath, g_deviceId, g_clipboard;
	int g_safeInsets[4] = { 0, 0, 0, 0 };
	float g_scale = 1.0f;
	std::atomic<int> g_kbShown{-1};	// ekran klavyesi durumu (oyun istegi + dokunarak kapatma)

	void Post(uint32_t msg, uintptr_t w, intptr_t l, bool waitCursor = false)
	{
		std::lock_guard<std::mutex> lk(g_queueLock);
		g_queue.push_back({ msg, w, l, waitCursor ? g_cursorSeq.load() + 1 : 0 });
	}

	void PostChar(wchar_t wc)
	{
		if (wc < 32)
			return;
		char mb[8];
		const int n = WideCharToMultiByte(CP_ACP, 0, &wc, 1, mb, sizeof(mb), nullptr, nullptr);
		for (int j = 0; j < n; ++j)
			Post(WM_CHAR, (unsigned char)mb[j], 1);
	}

	LPARAM MouseLParam() { return MAKELPARAM((WORD)g_mouseX.load(), (WORD)g_mouseY.load()); }

	WPARAM MouseKeys()
	{
		WPARAM k = 0;
		if (g_vkeys[VK_LBUTTON]) k |= MK_LBUTTON;
		if (g_vkeys[VK_RBUTTON]) k |= MK_RBUTTON;
		return k;
	}

	void SetMouseFromWindow(float x, float y)
	{
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

	std::string StdString(NSString* s) { return s ? std::string([s UTF8String]) : std::string(); }
}

// ------------------------------------------------------------------ gorunum
@interface CynexView : UIView <UIKeyInput>
{
	NSMutableDictionary<NSValue*, NSNumber*>* _ids;
	int _nextId;
}
@property (nonatomic, readonly) int pixelWidth;
@property (nonatomic, readonly) int pixelHeight;
@end

@implementation CynexView

+ (Class)layerClass { return [CAEAGLLayer class]; }

- (instancetype)initWithFrame:(CGRect)frame
{
	if ((self = [super initWithFrame:frame]))
	{
		self.multipleTouchEnabled = YES;
		self.opaque = YES;
		self.contentScaleFactor = [UIScreen mainScreen].nativeScale;
		CAEAGLLayer* layer = (CAEAGLLayer*)self.layer;
		layer.opaque = YES;
		layer.drawableProperties = @{ kEAGLDrawablePropertyRetainedBacking : @NO, kEAGLDrawablePropertyColorFormat : kEAGLColorFormatRGBA8 };
		_ids = [NSMutableDictionary dictionary];
		_nextId = 0;
	}
	return self;
}

- (int)pixelWidth { return (int)(self.bounds.size.width * self.contentScaleFactor + 0.5f); }
- (int)pixelHeight { return (int)(self.bounds.size.height * self.contentScaleFactor + 0.5f); }

- (void)layoutSubviews
{
	[super layoutSubviews];
	g_scale = (float)self.contentScaleFactor;
	g_screenW = self.pixelWidth;
	g_screenH = self.pixelHeight;
	if (@available(iOS 11.0, *))
	{
		UIEdgeInsets i = self.safeAreaInsets;
		g_safeInsets[0] = (int)(i.left * g_scale);
		g_safeInsets[1] = (int)(i.top * g_scale);
		g_safeInsets[2] = (int)(i.right * g_scale);
		g_safeInsets[3] = (int)(i.bottom * g_scale);
	}
	g_wantWindow = (self.window != nil && g_screenW > 0);
	if (g_focus)
		PostSize(false);
}

// -- dokunma (piksel koordinati); ilk parmak = sol fare tusu
- (int)idFor:(UITouch*)t create:(BOOL)create
{
	NSValue* key = [NSValue valueWithPointer:(__bridge const void*)t];
	NSNumber* n = _ids[key];
	if (!n && create)
	{
		for (int i = 0; i < CYNEX_MAX_TOUCH; ++i)
		{
			bool used = false;
			for (NSNumber* v in _ids.allValues)
				if ([v intValue] == i) { used = true; break; }
			if (!used) { n = @(i); break; }
		}
		if (!n) return -1;
		_ids[key] = n;
	}
	return n ? [n intValue] : -1;
}

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
	// ekranda bir yere dokununca klavye kapanir; yazi kutusuna dokunulursa oyun yeniden acar
	if (self.isFirstResponder)
	{
		[self resignFirstResponder];
		g_kbShown = 0;
	}
	for (UITouch* t in touches)
	{
		const int id = [self idFor:t create:YES];
		if (id < 0) continue;
		const CGPoint p = [t locationInView:self];
		const float x = p.x * g_scale, y = p.y * g_scale;
		CynexInput_TouchBegin(id, x, y);
		if (g_primaryPointer < 0)
		{
			g_primaryPointer = id;
			SetMouseFromWindow(x, y);
			g_vkeys[VK_LBUTTON] = 0x80;
			Post(WM_LBUTTONDOWN, MouseKeys(), MouseLParam(), true);
		}
	}
}

- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
	for (UITouch* t in touches)
	{
		const int id = [self idFor:t create:NO];
		if (id < 0) continue;
		const CGPoint p = [t locationInView:self];
		CynexInput_TouchMove(id, p.x * g_scale, p.y * g_scale);
		if (id == g_primaryPointer)
			SetMouseFromWindow(p.x * g_scale, p.y * g_scale);
	}
}

- (void)endTouches:(NSSet<UITouch*>*)touches cancelled:(BOOL)cancelled
{
	for (UITouch* t in touches)
	{
		const int id = [self idFor:t create:NO];
		if (id < 0) continue;
		const CGPoint p = [t locationInView:self];
		const float x = p.x * g_scale, y = p.y * g_scale;
		CynexInput_TouchEnd(id, x, y, cancelled ? 1 : 0);
		if (id == g_primaryPointer)
		{
			SetMouseFromWindow(x, y);
			g_vkeys[VK_LBUTTON] = 0;
			Post(WM_LBUTTONUP, MouseKeys(), MouseLParam(), true);
			g_primaryPointer = -1;
		}
		[_ids removeObjectForKey:[NSValue valueWithPointer:(__bridge const void*)t]];
	}
}

- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event { [self endTouches:touches cancelled:NO]; }
- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event { [self endTouches:touches cancelled:YES]; }

// -- ekran klavyesi (UIKeyInput)
- (BOOL)canBecomeFirstResponder { return YES; }
- (BOOL)hasText { return YES; }

- (void)insertText:(NSString*)text
{
	for (NSUInteger i = 0; i < text.length; ++i)
	{
		const unichar c = [text characterAtIndex:i];
		if (c == '\n')
		{
			Post(WM_KEYDOWN, VK_RETURN, 1);
			Post(WM_CHAR, 13, 1);
			Post(WM_KEYUP, VK_RETURN, 1);
		}
		else
			PostChar((wchar_t)c);
	}
}

- (void)deleteBackward
{
	Post(WM_KEYDOWN, VK_BACK, 1);
	Post(WM_CHAR, 8, 1);
	Post(WM_KEYUP, VK_BACK, 1);
}

- (UIKeyboardType)keyboardType { return UIKeyboardTypeDefault; }
- (UITextAutocorrectionType)autocorrectionType { return UITextAutocorrectionTypeNo; }
- (UITextAutocapitalizationType)autocapitalizationType { return UITextAutocapitalizationTypeNone; }
@end

// ------------------------------------------------------------------ uygulama
@interface CynexViewController : UIViewController
@end

@implementation CynexViewController
- (void)loadView { self.view = [[CynexView alloc] initWithFrame:[UIScreen mainScreen].bounds]; g_view = (CynexView*)self.view; }
- (UIInterfaceOrientationMask)supportedInterfaceOrientations { return UIInterfaceOrientationMaskLandscape; }
- (BOOL)prefersStatusBarHidden { return YES; }
- (BOOL)prefersHomeIndicatorAutoHidden { return YES; }
@end

@interface CynexAppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow* window;
@end

static void* GameThread(void*)
{
	@autoreleasepool {
		// pencere hazir olana kadar bekle
		for (int i = 0; i < 500 && !g_wantWindow.load(); ++i)
			usleep(10000);
		const int ret = CynexIos_GameMain();
		LOGI("AKRAVON kapandi (%d)", ret);
	}
	exit(0);
	return nullptr;
}

@implementation CynexAppDelegate
- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
	NSArray* dirs = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	g_dataPath = StdString(dirs.firstObject);
	g_deviceId = StdString([[[UIDevice currentDevice] identifierForVendor] UUIDString]);
	[UIApplication sharedApplication].idleTimerDisabled = YES;

	self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
	self.window.rootViewController = [[CynexViewController alloc] init];
	[self.window makeKeyAndVisible];
	g_focus = true;
	g_wantFocus = true;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8 * 1024 * 1024);
	pthread_t tid;
	pthread_create(&tid, &attr, GameThread, nullptr);
	pthread_detach(tid);
	return YES;
}

- (void)applicationWillResignActive:(UIApplication*)application { g_wantFocus = false; }
- (void)applicationDidBecomeActive:(UIApplication*)application { g_wantFocus = true; }
@end

int main(int argc, char* argv[])
{
	@autoreleasepool {
		return UIApplicationMain(argc, argv, nil, NSStringFromClass([CynexAppDelegate class]));
	}
}

// ------------------------------------------------------------------ CynexPlatform.h
extern "C" {

void CynexPlatform_Init(struct android_app*) {}	// ios: pencere ana iplikte kurulur
struct android_app* CynexPlatform_GetApp(void) { return nullptr; }
struct ANativeWindow* CynexPlatform_GetWindow(void) { return g_wantWindow.load() ? (struct ANativeWindow*)(__bridge void*)g_view : nullptr; }
int CynexPlatform_GetWidth(void) { return g_renderW ? g_renderW : g_screenW; }
int CynexPlatform_GetHeight(void) { return g_renderH ? g_renderH : g_screenH; }
int CynexPlatform_GetScreenWidth(void) { return g_screenW; }
int CynexPlatform_GetScreenHeight(void) { return g_screenH; }
int CynexPlatform_GetDensityDpi(void) { return (int)(160.0f * g_scale * 1.0f); }	// ios: 1x = 160 dpi (@2x = 320, @3x = 480)
int CynexPlatform_HasFocus(void) { return g_focus ? 1 : 0; }
int CynexPlatform_IsDestroyRequested(void) { return 0; }

void CynexPlatform_SetRenderSize(int width, int height)
{
	if (width == g_renderW && height == g_renderH)
		return;
	g_renderW = width;
	g_renderH = height;
	PostSize(false);
}

void CynexPlatform_GetSafeInsets(int* out4)
{
	for (int i = 0; i < 4; ++i)
		out4[i] = g_safeInsets[i];
}

// oyun ipliginden her karede: yuzey ve odak degisimini oyuna bildir
void CynexPlatform_PumpEvents(int timeoutMs)
{
	const bool wantWindow = g_wantWindow.load();
	if (wantWindow != g_windowAnnounced)
	{
		g_windowAnnounced = wantWindow;
		if (g_windowCallback)
			g_windowCallback(wantWindow ? (struct ANativeWindow*)(__bridge void*)g_view : nullptr, wantWindow ? 1 : 0);
		if (wantWindow)
			PostSize(false);
	}
	const bool wantFocus = g_wantFocus.load();
	if (wantFocus != g_focus)
	{
		g_focus = wantFocus;
		if (g_focus) { CynexMss_Resume(); PostSize(false); }
		else { CynexMss_Suspend(); PostSize(true); }
	}
	if (timeoutMs > 0)
		usleep(timeoutMs * 1000);
}

void CynexPlatform_PostMessage(uint32_t msg, uintptr_t wParam, intptr_t lParam) { Post(msg, wParam, lParam); }

int CynexPlatform_PeekMessage(uint32_t* msg, uintptr_t* wParam, intptr_t* lParam, int remove)
{
	std::lock_guard<std::mutex> lk(g_queueLock);
	if (g_queue.empty())
		return 0;
	const SMessage& m = g_queue.front();
	if (m.cursorSeq && (int32_t)(g_cursorSeq.load() - m.cursorSeq) < 0)
		return 0;
	*msg = m.msg;
	*wParam = m.wParam;
	*lParam = m.lParam;
	if (remove)
		g_queue.pop_front();
	return 1;
}

int CynexPlatform_GetMousePos(int* x, int* y) { *x = g_mouseX; *y = g_mouseY; return 1; }
void CynexPlatform_MarkCursorRead(void) { g_cursorSeq++; }
void CynexPlatform_SetMousePos(int x, int y) { g_mouseX = x; g_mouseY = y; }
int CynexPlatform_IsVKeyDown(int vk) { return vk >= 0 && vk < 256 && g_vkeys[vk]; }
void CynexPlatform_SetWindowCallback(CynexWindowCallback cb) { g_windowCallback = cb; }

void CynexPlatform_OpenURL(const char* url)
{
	if (!url) return;
	NSString* s = [NSString stringWithUTF8String:url];
	dispatch_async(dispatch_get_main_queue(), ^{
		NSURL* u = [NSURL URLWithString:s];
		if (u) [[UIApplication sharedApplication] openURL:u options:@{} completionHandler:nil];
	});
}

void CynexPlatform_Vibrate(int milliseconds)
{
	dispatch_async(dispatch_get_main_queue(), ^{ AudioServicesPlaySystemSound(kSystemSoundID_Vibrate); });
}

void CynexPlatform_ShowKeyboard(int show)
{
	show = show ? 1 : 0;
	if (show == g_kbShown.load()) return;
	g_kbShown = show;
	dispatch_async(dispatch_get_main_queue(), ^{
		if (!g_view) return;
		if (show) [g_view becomeFirstResponder]; else [g_view resignFirstResponder];
	});
}

void CynexPlatform_SetClipboardText(const char* utf8)
{
	NSString* s = [NSString stringWithUTF8String:utf8 ? utf8 : ""];
	dispatch_async(dispatch_get_main_queue(), ^{ [UIPasteboard generalPasteboard].string = s; });
}

const char* CynexPlatform_GetClipboardText(void)
{
	__block std::string out;
	dispatch_sync(dispatch_get_main_queue(), ^{ out = StdString([UIPasteboard generalPasteboard].string); });
	g_clipboard = out;
	return g_clipboard.c_str();
}

const char* CynexPlatform_GetDataPath(void) { return g_dataPath.c_str(); }
const char* CynexPlatform_GetDeviceId(void) { return g_deviceId.empty() ? "cynex" : g_deviceId.c_str(); }

void CynexPlatform_ShowMessage(const char* title, const char* text)
{
	LOGI("[%s] %s", title ? title : "", text ? text : "");
	NSString* t = [NSString stringWithUTF8String:title ? title : ""];
	NSString* m = [NSString stringWithUTF8String:text ? text : ""];
	dispatch_async(dispatch_get_main_queue(), ^{
		UIAlertController* a = [UIAlertController alertControllerWithTitle:t message:m preferredStyle:UIAlertControllerStyleAlert];
		[a addAction:[UIAlertAction actionWithTitle:@"Tamam" style:UIAlertActionStyleDefault handler:nil]];
		[g_view.window.rootViewController presentViewController:a animated:YES completion:nil];
	});
}

}	// extern "C"
