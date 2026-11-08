#!/bin/sh
# Cynex: yalnizca /usr/barbaros2 altindan calisan surecleri kapat (CYNEXTM srv1-* adlarina dokunmaz)
for p in $(pgrep -f '^\./srv1-'); do
	if procstat -f $p 2>/dev/null | awk '$3=="cwd"{print $NF}' | grep -q '^/usr/barbaros2/'; then
		kill $p
	fi
done
sleep 5
echo kapandi
