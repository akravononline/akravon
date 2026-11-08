# -*- coding: utf-8 -*-
# AKRAVON: HUD dugmelerine hafif "eski Metin2" eskitmesi (antik bronz-altin ton, tane, kenar patinasi); build_akravon_hud'dan SONRA calisir.
# ayni zamanda beceri yuvasi altindaki "ayar" ve "sayfa cevir" ikonlarini yeni dugme plakalarindan uretir.
import os
import sys
import numpy as np
from PIL import Image, ImageFilter

OUT = sys.argv[1] if len(sys.argv) > 1 else 'C:/AkravonTest/ui_out'
HUD = os.path.join(OUT, 'mobile_ui', 'akravon', 'hud')
STRENGTH = 0.55
SKIP = ('btn_attack', 'btn_alert', 'btn_send', 'btn_collapse', 'btn_pin')
rng = np.random.RandomState(7)
def lerp(a, b, t):
    return a + (b - a) * t


def age(im, strength):
    a = np.asarray(im.convert('RGBA')).astype(np.float32)
    alpha = a[..., 3:4]
    rgb = a[..., :3]
    h, w = rgb.shape[:2]
    lum = (rgb @ np.array([0.299, 0.587, 0.114], dtype=np.float32)) / 255.0
    sat = rgb.max(2) - rgb.min(2)
    # metal / ikon: parlak ve soluk alanlar -> antik bronz-altin; lacivert zemin koyu kalir
    gold_dark = np.array([58, 38, 14], dtype=np.float32)
    gold_mid = np.array([158, 112, 52], dtype=np.float32)
    gold_hi = np.array([226, 186, 108], dtype=np.float32)
    t = np.clip(lum, 0, 1)[..., None]
    toned = np.where(t < 0.5, gold_dark + (gold_mid - gold_dark) * (t / 0.5), gold_mid + (gold_hi - gold_mid) * ((t - 0.5) / 0.5))
    isNavy = ((rgb[..., 2] > rgb[..., 0] * 1.15) & (lum < 0.30))[..., None]
    out = np.where(isNavy, rgb * 0.82 + np.array([6, 4, 0], dtype=np.float32), lerp(rgb, toned, strength))
    # ince taneli doku + lekeler (yaslanmis metal)
    grain = rng.normal(0, 7, (h, w, 1)).astype(np.float32)
    blot = np.asarray(Image.fromarray((rng.rand(h // 6 + 1, w // 6 + 1) * 255).astype(np.uint8)).resize((w, h), Image.BICUBIC).filter(ImageFilter.GaussianBlur(2))).astype(np.float32)[..., None] / 255.0
    out = out + grain * (1 - 0.5 * isNavy) + (blot - 0.5) * 34 * strength
    # kenarlarda koyulasma (patina)
    yy, xx = np.mgrid[0:h, 0:w]
    d = np.hypot(xx - w / 2, yy - h / 2) / (w / 2)
    out = out * (1 - 0.28 * strength * np.clip(d - 0.55, 0, 1)[..., None] * 2)
    out = np.clip(out, 0, 255)
    res = np.concatenate([out, alpha], axis=2).astype(np.uint8)
    return Image.fromarray(res, 'RGBA')




def main():
    n = 0
    for f in sorted(os.listdir(HUD)):
        if not (f.startswith('btn_') and f.endswith('.png')) or f.startswith(SKIP):
            continue
        p = os.path.join(HUD, f)
        age(Image.open(p), STRENGTH).save(p)
        n += 1
    # ayar (cark anahtari) ve sayfa cevir ikonlari: ayni eskitilmis plakadan, 256 px
    for (dst, srcName) in (('hud_skill_edit', 'btn_wrench_normal'), ('hud_skill_pages', 'btn_refresh_normal')):
        Image.open(os.path.join(HUD, srcName + '.png')).convert('RGBA').resize((256, 256), Image.LANCZOS).save(os.path.join(HUD, dst + '.png'))
    print('eskitildi:', n)


main()
