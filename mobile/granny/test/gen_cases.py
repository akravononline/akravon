# Cynex: dogrulama senaryo listesi uretici
# Cynex: kullanim: python gen_cases.py <pack_koku> <cikti.txt> [senaryo_sayisi]
import os
import sys


def rel(root, path):
    return os.path.relpath(path, root).replace('\\', '/')


def gr2_files(d):
    try:
        return sorted(f for f in os.listdir(d) if f.lower().endswith('.gr2'))
    except OSError:
        return []


def main():
    root = sys.argv[1]
    out = sys.argv[2]
    limit = int(sys.argv[3]) if len(sys.argv) > 3 else 150
    cases = []

    # Cynex: karakterler - govde + sac (link) + lod (copymotion) + silah (attach) + genel animasyonlar
    weapon_dir = os.path.join(root, 'etc', 'ymir work', 'item', 'weapon')
    weapons = gr2_files(weapon_dir)
    for pack in ('pc', 'pc2', 'pc3'):
        base = os.path.join(root, pack, 'ymir work', pack)
        if not os.path.isdir(base):
            continue
        for job in sorted(os.listdir(base)):
            d = os.path.join(base, job)
            if not os.path.isdir(d):
                continue
            models = [f for f in gr2_files(d) if '_lod_' not in f]
            anims_dir = os.path.join(d, 'general')
            anims = gr2_files(anims_dir)
            hairs = gr2_files(os.path.join(d, 'hair'))
            for i, m in enumerate(models[:6]):
                fields = ['model:' + rel(root, os.path.join(d, m))]
                lod = m[:-4] + '_lod_01.gr2'
                if os.path.exists(os.path.join(d, lod)):
                    fields.append('lod:' + rel(root, os.path.join(d, lod)))
                if hairs:
                    fields.append('link:' + rel(root, os.path.join(d, 'hair', hairs[i % len(hairs)])))
                if weapons:
                    fields.append('attach:' + rel(root, os.path.join(weapon_dir, weapons[(i * 7) % len(weapons)])))
                picked = [a for a in ('wait.gr2', 'run.gr2', 'walk.gr2') if a in anims][:2] + anims[i:i + 2]
                for a in picked:
                    fields.append('anim:' + rel(root, os.path.join(anims_dir, a)))
                if len(fields) > 1:
                    cases.append('|'.join(fields))

    # Cynex: canavar / npc - klasor adiyla ayni model + klasordeki animasyonlar
    dirs = []
    for pack in ('monster', 'monster2', 'npc', 'npc2', 'patchadditions1', 'patchadditions2', 'season', 'zone'):
        base = os.path.join(root, pack)
        for dp, dn, fn in os.walk(base):
            name = os.path.basename(dp).lower()
            files = [f for f in fn if f.lower().endswith('.gr2')]
            model = [f for f in files if f.lower() == name + '.gr2']
            if model and len(files) >= 2:
                dirs.append((dp, model[0], sorted(f for f in files if f != model[0] and '_lod_' not in f.lower())))
    dirs.sort()
    special = ('doctor', 'pig_young1', 'crustacean_officer', 'plechi_valentine2023_pet4', 'manticore_boss2', 'minidungeon_pawn')
    step = max(1, len(dirs) // max(1, limit - len(cases)))
    chosen = [x for i, x in enumerate(dirs) if i % step == 0 or os.path.basename(x[0]).lower() in special]
    for dp, model, anims in chosen:
        if not anims:
            continue
        fields = ['model:' + rel(root, os.path.join(dp, model))]
        lod = model[:-4] + '_lod_01.gr2'
        if os.path.exists(os.path.join(dp, lod)):
            fields.append('lod:' + rel(root, os.path.join(dp, lod)))
        for a in anims[:4]:
            fields.append('anim:' + rel(root, os.path.join(dp, a)))
        cases.append('|'.join(fields))

    # Cynex: statik modeller (silah, bina/zindan blogu) - animasyonsuz da veri karsilastirmasi yapilir
    statics = []
    for dp, dn, fn in os.walk(os.path.join(root, 'property')):
        statics += [os.path.join(dp, f) for f in fn if f.lower().endswith('.gr2')]
    for dp, dn, fn in os.walk(os.path.join(root, 'zone')):
        statics += [os.path.join(dp, f) for f in fn if f.lower().endswith('.gr2')]
    statics.sort()
    for p in statics[::max(1, len(statics) // 40)]:
        cases.append('model:' + rel(root, p))

    with open(out, 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(cases) + '\n')
    print(len(cases), 'senaryo')


if __name__ == '__main__':
    main()
