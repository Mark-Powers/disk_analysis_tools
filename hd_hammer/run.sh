#!/bin/bash

#prefix=direct_none
prefix=circuitous_none
iterations=10

set -e

make

sudo hdparm -W 0 /dev/sdb

for (( i=1; i<=$iterations; i++ ))
do
	log_file=logs/$prefix\_$i.csv
	./time -r > $log_file
	head -n -1 $log_file | sponge $log_file
	echo "$log_file complete"

	espeak "finished $i"
	echo "finished $i"
done

sudo hdparm -W 1 /dev/sdb

espeak "completed all"
echo "completed all"

