# Cynex: EterLib/IME.cpp -> port/EterLib/IME_mobile.cpp
# CIME'nin metin duzenleme kodu aynen kalir, yalnizca TSF (msctf) siniflari bos karsilikla degisir.
# Orijinal IME.cpp degisirse bu betik yeniden calistirilir.
import os, sys

src = sys.argv[1]   # client/source/EterLib/IME.cpp
out = sys.argv[2]   # mobile/port/EterLib/IME_mobile.cpp

raw = open(src, 'rb').read()
crlf = b'\r\n' in raw
lines = raw.decode('latin1').replace('\r\n', '\n').split('\n')

def find(text, start=0):
	for i in range(start, len(lines)):
		if lines[i].startswith(text):
			return i
	raise SystemExit('bulunamadi: ' + text)

tsf_doc = find('//  CTsfUiLessMode') - 1          # yorum blogunun ilk satiri (////...)
cicero_end = find('static CDisableCicero g_disableCicero;')
tsf_impl = find('BOOL CTsfUiLessMode::SetupSinks()')
wm_begin = find('LRESULT CIME::WMInputLanguage(')

stub = '''// Cynex: mobilde TSF yok - ayni arayuzle bos siniflar
class CTsfUiLessMode
{
public:
	static BOOL SetupSinks() { return FALSE; }
	static void ReleaseSinks() {}
	static BOOL CurrentInputLocaleIsIme() { return FALSE; }
	static void UpdateImeState(BOOL bResetCompartmentEventSink = FALSE) {}
	static void EnableUiUpdates(bool bEnable) {}
};

class CDisableCicero
{
public:
	void Initialize() {}
	void Uninitialize() {}
	void DisableCiceroOnThisWnd(HWND hwnd) {}
};
static CDisableCicero g_disableCicero;'''.split('\n')

head = []
for l in lines[:tsf_doc]:
	if l.strip() in ('#include "msctf.h"', '#include <oleauto.h>'):
		continue
	if l.strip() == '#include "StdAfx.h"':
		l = '#include "../../../client/source/EterLib/StdAfx.h"'
	elif l.startswith('#include "'):
		l = '#include "../../../client/source/EterLib/' + l[len('#include "'):]
	head.append(l)

body = ['// Cynex: EterLib/IME.cpp mobil karsiligi - tools/gen_ime_mobile.py uretti', ''] + head + stub \
	+ lines[cicero_end + 1:tsf_impl] + lines[wm_begin:]

text = '\n'.join(body)
# Cynex: yazi alani odaklaninca telefon klavyesi acilsin
for fn, show in (('EnableCaptureInput', 1), ('DisableCaptureInput', 0)):
	sig = 'void CIME::%s()\n{\n' % fn
	if sig not in text:
		raise SystemExit('bulunamadi: ' + fn)
	text = text.replace(sig, sig + '\tCynexPlatform_ShowKeyboard(%d);\n' % show, 1)
text = text.replace('#include "../../../client/source/EterLib/StdAfx.h"',
	'#include "../../../client/source/EterLib/StdAfx.h"\n#include "../../platform/CynexPlatform.h"', 1)
if crlf:
	text = text.replace('\n', '\r\n')
os.makedirs(os.path.dirname(out), exist_ok=True)
open(out, 'wb').write(text.encode('latin1'))
print('yazildi:', out, len(body), 'satir (orijinal', len(lines), ')')
