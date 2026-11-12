# -*- coding: utf-8 -*-
# AKRAVON: ticaret penceresi tahtasi akrboard_668x380.png (envanterle ayni cizim)
import os
import sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import build_akravon_charwin as cw
OUT = os.path.join(HERE, '..', '..', 'gorseller', 'mobile_ui', 'akravon', 'hud')
cw.Board(668, 380).save(os.path.join(OUT, 'akrboard_668x380.png'))
print('ok')
