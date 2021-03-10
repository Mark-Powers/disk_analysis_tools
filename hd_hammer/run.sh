#!/bin/bash
set -e
make
echo "test" > /media/mark/Backup/.keepalive
sudo hdparm -W 0 /dev/sdb
./time > log.csv
sudo hdparm -W 1 /dev/sdb
head -n -1 log.csv | sponge log.csv
./plot.py

