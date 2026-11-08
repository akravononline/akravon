# Cynex: barbaros2 test - portlar +5000 (zeph2 30xxx ile cakismasin), mysql 3310
import re, glob, shutil, os
os.chdir('/usr/barbaros2/game/srv1')
files = glob.glob('auth*/CONFIG') + glob.glob('chan/*/*/CONFIG') + ['db/conf.txt']
for f in files:
    s = open(f, encoding='latin1').read()
    if not os.path.exists(f + '.orj'):
        shutil.copy(f, f + '.orj')
    else:
        s = open(f + '.orj', encoding='latin1').read()
    s = re.sub(r'^((?:PORT|P2P_PORT|DB_PORT):\s*)30(\d\d\d)', lambda m: m.group(1) + str(35000 + int(m.group(2))), s, flags=re.M)
    s = re.sub(r'^(BIND_PORT\s*=\s*)30000', r'\g<1>35000', s, flags=re.M)
    s = re.sub(r'(127\.0\.0\.1[^\n]*?) 3306\b', r'\1 3310', s)
    s = s.replace('"127.0.0.1 account mt2 mt2 3306"', '"127.0.0.1 account mt2 mt2 3310"')
    open(f, 'w', encoding='latin1').write(s)
print(len(files), 'dosya')
