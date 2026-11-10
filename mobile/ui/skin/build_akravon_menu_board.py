# -*- coding: utf-8 -*-
# AKRAVON: ok tablosu zemini - envanter/ekipman penceresindeki tahtanin (build_akravon_charwin.Board) ayni cizimi, 9 parca.
# cikti: <out>/mobile_ui/akravon/hud/menu_board_{tl,t,tr,l,c,r,bl,b,br}.png (kose = 12 px)
import os
import sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import build_akravon_charwin as cw

OUT = sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/menu_out'
CORNER = 12
SIZE = 300

cw.OUT_DIR = sys.argv[2] if len(sys.argv) > 2 else 'C:/AkravonTest/ui_out'
board = cw.Board(SIZE, SIZE)
dest = os.path.join(OUT, 'mobile_ui', 'akravon', 'hud')
os.makedirs(dest, exist_ok = True)
far = SIZE - CORNER
pieces = {
	'tl': (0, 0, CORNER, CORNER), 'tr': (far, 0, SIZE, CORNER),
	'bl': (0, far, CORNER, SIZE), 'br': (far, far, SIZE, SIZE),
	't': (CORNER, 0, CORNER + 1, CORNER), 'b': (CORNER, far, CORNER + 1, SIZE),
	'l': (0, CORNER, CORNER, CORNER + 1), 'r': (far, CORNER, SIZE, CORNER + 1),
	'c': (CORNER, CORNER, far, far),
}
for (key, box) in pieces.items():
	board.crop(box).save(os.path.join(dest, 'menu_board_%s.png' % key))
print('tamam', dest)
