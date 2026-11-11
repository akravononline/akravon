# -*- coding: utf-8 -*-
# AKRAVON: "calisiyor" (oto av vb.) dugme efekti gorselleri - beyaz uretilir, oyunda yesile boyanir
#  btn_run_bubble : iksir kabarcigi (dugme icinde alttan yukari yukselir; oyunda 7 tane farkli hiz / boyutta)
#  btn_run_glow : dugme icinde yavas nefes alan ic isima (kenara dogru guclenir)
# tuval 176x176 (btn_alert ile ayni: dugme 136 + her yanda 20), oyunda dugmeyle ayni olcekte cizilir
import os
import sys
import numpy as np
from PIL import Image

SIZE = 176
SS = 4
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, '..', '..', 'gorseller', 'mobile_ui', 'akravon', 'hud')


def Bubble():
	# iksir kabarcigi: hafif dolgulu disk + parlak ince kenar + sol ustte kucuk isik noktasi (beyaz, oyunda yesile boyanir)
	S = 64
	n = S * SS
	c = (n - 1) / 2.0
	ys, xs = np.mgrid[0:n, 0:n].astype(np.float32)
	r = np.sqrt((xs - c) ** 2 + (ys - c) ** 2)
	R = 26.0 * SS
	fill = np.clip((R - r) / (1.5 * SS), 0.0, 1.0) * 0.28
	rim = np.clip(1.0 - np.abs(r - R) / (2.4 * SS), 0.0, 1.0) ** 1.2 * 0.95
	hx, hy = c - 9.0 * SS, c - 9.0 * SS
	spot = np.exp(-(((xs - hx) / (8.0 * SS)) ** 2 + ((ys - hy) / (3.2 * SS)) ** 2)) * 0.95
	alpha = np.clip(fill + rim + spot, 0.0, 1.0)
	out = np.zeros((n, n, 4), np.uint8)
	out[..., :3] = 255
	out[..., 3] = (alpha * 255).astype(np.uint8)
	return Image.fromarray(out, 'RGBA').resize((S, S), Image.LANCZOS)


def Dot():
	# minik parlak nokta parcacik (beyaz, oyunda yesile boyanir)
	S = 16
	n = S * SS
	c = (n - 1) / 2.0
	ys, xs = np.mgrid[0:n, 0:n].astype(np.float32)
	r = np.sqrt((xs - c) ** 2 + (ys - c) ** 2)
	alpha = np.clip(np.exp(-((r / (2.6 * SS)) ** 2)) * 1.3, 0.0, 1.0)
	out = np.zeros((n, n, 4), np.uint8)
	out[..., :3] = 255
	out[..., 3] = (alpha * 255).astype(np.uint8)
	return Image.fromarray(out, 'RGBA').resize((S, S), Image.LANCZOS)


def Glow():
	n = SIZE * SS
	c = (n - 1) / 2.0
	ys, xs = np.mgrid[0:n, 0:n].astype(np.float32)
	r = np.sqrt((xs - c) ** 2 + (ys - c) ** 2)
	# kenar isiltisi: dugme kenarinda (r=68) en parlak, disa dogru yumusakca solar, ice dogru da hafifce girer
	edge = 68.0 * SS
	# referans: kenara yapisik ince parlak halka, disa dogru kisa yumusak yayilim, iceride bir sey yok
	outer = np.where(r > edge, np.exp(-(((r - edge) / (8.0 * SS)) ** 2)), np.exp(-(((r - edge) / (3.0 * SS)) ** 2))) * 0.95
	alpha = outer
	alpha = np.clip(alpha, 0.0, 1.0)
	out = np.zeros((n, n, 4), np.uint8)
	out[..., :3] = 255
	out[..., 3] = (alpha * 255).astype(np.uint8)
	return Image.fromarray(out, 'RGBA').resize((SIZE, SIZE), Image.LANCZOS)


if __name__ == '__main__':
	out = sys.argv[1] if len(sys.argv) > 1 else OUT
	os.makedirs(out, exist_ok = True)
	Bubble().save(os.path.join(out, 'btn_run_bubble.png'))
	Dot().save(os.path.join(out, 'btn_run_dot.png'))
	Glow().save(os.path.join(out, 'btn_run_glow.png'))
	print('btn_run_bubble / btn_run_glow ->', os.path.abspath(out))
