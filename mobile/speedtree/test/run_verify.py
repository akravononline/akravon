# Cynex: sptverify toplu calistirici; agac basina ayri surec (dll coklu agacta heap bozabiliyor)
# Cynex: kullanim: python run_verify.py <orijinal_klasor> <donusmus_klasor> <dokum_klasor> [--x64 <sptverify_x64.exe>] [-j N]
import os, sys, re, subprocess, collections
from concurrent.futures import ThreadPoolExecutor

X86 = r"C:\CynexMobilBuild\speedtree\verify_x86\sptverify.exe"

def parse(out):
    cats = {}
    total = None
    for line in out.splitlines():
        m = re.match(r"CAT (\S+)\s+rec=(\d+) val=(\d+) miss=(\d+) struct=(\d+) bad=(\d+) maxabs=(\S+) maxrel=(\S+)(?: worst=(.*))?", line)
        if m:
            cats[m.group(1)] = dict(rec=int(m.group(2)), val=int(m.group(3)), miss=int(m.group(4)), struct=int(m.group(5)),
                                    bad=int(m.group(6)), maxabs=float(m.group(7)), maxrel=float(m.group(8)), worst=m.group(9) or "")
        m = re.match(r"TOTAL records=(\d+) badcount=(\d+)", line)
        if m:
            total = (int(m.group(1)), int(m.group(2)))
    extra = re.search(r"EXTRA (\d+)", out)
    return cats, total, int(extra.group(1)) if extra else 0

def main():
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    args = sys.argv[1:]
    x64 = None
    jobs = os.cpu_count() or 4
    if "--x64" in args:
        i = args.index("--x64"); x64 = args[i + 1]; del args[i:i + 2]
    if "-j" in args:
        i = args.index("-j"); jobs = int(args[i + 1]); del args[i:i + 2]
    orig_dir, conv_dir, dump_dir = args[:3]
    os.makedirs(dump_dir, exist_ok=True)
    names = sorted(f for f in os.listdir(conv_dir) if f.lower().endswith(".spt"))

    def run(name):
        conv = os.path.join(conv_dir, name)
        dump = os.path.join(dump_dir, name + ".bin")
        if x64:
            cmd = [x64, "--ref", dump, conv]
        else:
            cmd = [X86, os.path.join(orig_dir, name), conv, dump]
        # Cynex: dll heap bozulmasi ortama bagli; cokerse iki kez daha dene
        last = (name, -1, "")
        for attempt in range(3):
            try:
                p = subprocess.run(cmd, capture_output=True, text=True, errors="replace", timeout=600)
                last = (name, p.returncode, p.stdout)
                if "TOTAL records=" in p.stdout:
                    break
            except subprocess.TimeoutExpired:
                last = (name, -1, "TIMEOUT")
        return last

    agg = collections.OrderedDict()
    trees_bad = []
    crashed = []
    ok = 0
    records = 0
    with ThreadPoolExecutor(max_workers=jobs) as ex:
        for name, code, out in ex.map(run, names):
            cats, total, extra = parse(out)
            if total is None:
                crashed.append((name, code, out.strip().splitlines()[-1] if out.strip() else ""))
                continue
            ok += 1
            records += total[0]
            if total[1]:
                trees_bad.append((name, total[1], [(k, v) for k, v in cats.items() if v["miss"] or v["struct"] or v["bad"]][:6], extra))
            for k, v in cats.items():
                a = agg.setdefault(k, dict(trees=0, val=0, miss=0, struct=0, bad=0, maxabs=0.0, maxrel=0.0, worst=""))
                a["trees"] += 1; a["val"] += v["val"]; a["miss"] += v["miss"]; a["struct"] += v["struct"]; a["bad"] += v["bad"]
                if v["maxabs"] > a["maxabs"]:
                    a["maxabs"] = v["maxabs"]
                if v["maxrel"] > a["maxrel"]:
                    a["maxrel"] = v["maxrel"]; a["worst"] = name + " " + v["worst"]
    print("agac: %d karsilastirildi, %d calismadi, kayit %d" % (ok, len(crashed), records))
    for name, code, last in crashed:
        print("  CALISMADI %s (kod %s) %s" % (name, code, last))
    print("%-36s %6s %10s %6s %6s %6s %10s %10s" % ("kategori", "agac", "deger", "eksik", "yapi", "fark", "maxabs", "maxrel"))
    for k, a in agg.items():
        print("%-36s %6d %10d %6d %6d %6d %10.3g %10.3g  %s" % (k, a["trees"], a["val"], a["miss"], a["struct"], a["bad"], a["maxabs"], a["maxrel"], a["worst"] if a["maxrel"] > 1e-4 else ""))
    print("farkli agac: %d" % len(trees_bad))
    for name, bad, cats, extra in trees_bad[:60]:
        print("  %s bad=%d extra=%d %s" % (name, bad, extra, "; ".join("%s(m%d s%d b%d)" % (k, v["miss"], v["struct"], v["bad"]) for k, v in cats)))

if __name__ == "__main__":
    main()
