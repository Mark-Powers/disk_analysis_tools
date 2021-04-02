#!/bin/bash

iterations=5
file=0
type_label=none-random-rt

set -e

make

sudo hdparm -W 0 /dev/sdb
sudo hdparm -W 0 /dev/sdc
sudo hdparm -W 0 /dev/sdd
#sudo hdparm -W 0 /dev/sde


for (( file=1; file<=3; file++ ))
do
	case $file in
		1)
			prefix=sdb-$type_label
			;;
		2)
			prefix=sdc-$type_label
			;;
		3)
			prefix=sdd-$type_label
			;;
		*)
			prefix=backup-$type_label
			;;
	esac

	sudo ./time -a -f $file

	for (( i=1; i<=$iterations; i++ ))
	do
		log_file=logs/$prefix\_$i.csv
		sudo nice -n -20 ./time -r -f $file -l $log_file
#		head -n -1 $log_file | sponge $log_file
		echo "$log_file complete"

		#espeak "finished $i"
		echo "finished $i"
	done
done

sudo hdparm -W 1 /dev/sdb
sudo hdparm -W 1 /dev/sdc
sudo hdparm -W 1 /dev/sdd
#sudo hdparm -W 1 /dev/sde

#espeak "completed all"
echo "completed all"

