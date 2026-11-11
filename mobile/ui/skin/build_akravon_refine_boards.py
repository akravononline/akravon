# -*- coding: utf-8 -*-
# AKRAVON: yukseltme penceresi (630 genis) icin tahta gorselleri: akrboard_630x<288..624 step 24>.png
import os
import sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import build_akravon_charwin as cw
OUT = os.path.join(HERE, '..', '..', 'gorseller', 'mobile_ui', 'akravon', 'hud')
os.makedirs(OUT, exist_ok = True)
for height in range(288, 745, 24):
    cw.Board(630, height).save(os.path.join(OUT, 'akrboard_630x%d.png' % height))
print('ok')
