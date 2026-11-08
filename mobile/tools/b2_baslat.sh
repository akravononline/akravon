#!/bin/sh
# Cynex: barbaros2 test sunucusu (mobil deneme) - db + auth1 + ch1 (4 core) + ch99
B=/usr/barbaros2
G=$B/game/srv1
IP=192.168.1.10
if ! sockstat -4l | grep -q ":3310 "; then
	(cd $B && nohup m114/usr/local/libexec/mariadbd --defaults-file=$B/my.cnf > /dev/null 2>&1 &)
	sleep 6
fi
run() {
	cd "$1" && nohup ./$2 -I $IP > /dev/null 2>&1 &
}
run $G/db srv1-db
sleep 6
run $G/auth1 srv1-auth1
for c in 1 2 3 4; do run $G/chan/ch1/core$c srv1-ch1-core$c; sleep 2; done
run $G/chan/ch99/core99 srv1-ch99-core99
sleep 8
sockstat -4l | grep -E ":350[0-9][0-9] " | awk '{print $2, $6}' | sort -u
