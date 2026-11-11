# -*- coding: utf-8 -*-
# AKRAVON: "calisiyor" (oto av vb.) dugme efekti gorselleri - beyaz uretilir, oyunda yesile boyanir
#  btn_run_ring : dugme kenarindan disari yayilip solan dalga halkasi (iki halka sirayla)
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


def Ring():
	# ince, kenarlari yumusak halka (dugme kenarinda); oyunda buyuyup solarak dalga gibi yayilir
	n = SIZE * SS
	c = (n - 1) / 2.0
	ys, xs = np.mgrid[0:n, 0:n].astype(np.float32)
	r = np.sqrt((xs - c) ** 2 + (ys - c) ** 2)
	ring_r = 66.0 * SS
	width = 3.0 * SS
	core = np.clip(1.0 - np.abs(r - ring_r) / width, 0.0, 1.0) ** 1.1
	halo = np.clip(1.0 - np.abs(r - ring_r) / (8.0 * SS), 0.0, 1.0) ** 2.2 * 0.5
	alpha = np.clip(core + halo, 0.0, 1.0)
	out = np.zeros((n, n, 4), np.uint8)
	out[..., :3] = 255
	out[..., 3] = (alpha * 255).astype(np.uint8)
	return Image.fromarray(out, 'RGBA').resize((SIZE, SIZE), Image.LANCZOS)


def Glow():
	n = SIZE * SS
	c = (n - 1) / 2.0
	ys, xs = np.mgrid[0:n, 0:n].astype(np.float32)
	r = np.sqrt((xs - c) ** 2 + (ys - c) ** 2)
	inner = 66.0 * SS
	# kenara dogru artan ic isima: merkez saydam, kenarda belirgin; dis tarafta keskin kesim
	t = np.clip(r / inner, 0.0, 1.0)
	alpha = (t ** 2.6) * 0.78
	alpha[r > inner] = 0.0
	edge = np.clip((inner - r) / (2.0 * SS), 0.0, 1.0)
	alpha *= edge
	out = np.zeros((n, n, 4), np.uint8)
	out[..., :3] = 255
	out[..., 3] = (alpha * 255).astype(np.uint8)
	return Image.fromarray(out, 'RGBA').resize((SIZE, SIZE), Image.LANCZOS)


if __name__ == '__main__':
	out = sys.argv[1] if len(sys.argv) > 1 else OUT
	os.makedirs(out, exist_ok = True)
	Ring().save(os.path.join(out, 'btn_run_ring.png'))
	Glow().save(os.path.join(out, 'btn_run_glow.png'))
	print('btn_run_ring / btn_run_glow ->', os.path.abspath(out))
