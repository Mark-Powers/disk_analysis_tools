#!/bin/bash

iterations=5
file=0
#type_label=none-seq-rt
type_label=50Hz-4.8mmps

set -e

make

sudo hdparm -W 0 /dev/sdb
sudo hdparm -W 0 /dev/sdc
sudo hdparm -W 0 /dev/sdd
#sudo hdparm -W 0 /dev/sde


for (( file=1; file<=1; file++ ))
do
	case $file in
		1)
			prefix=sdb-$type_label
			device=/dev/sdb
			;;
		2)
			prefix=sdc-$type_label
			device=/dev/sdc
			;;
		3)
			prefix=sdd-$type_label
			device=/dev/sdd
			;;
		*)
			prefix=backup-$type_label
			;;
	esac
	
	smart_log=smart/$prefix\_smart.csv
	echo "before" > $smart_log
	sudo ./smart.py $device >> $smart_log

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

	echo "after" >> $smart_log
	sudo ./smart.py $device >> $smart_log
done

sudo hdparm -W 1 /dev/sdb
sudo hdparm -W 1 /dev/sdc
sudo hdparm -W 1 /dev/sdd
#sudo hdparm -W 1 /dev/sde

#espeak "completed all"
echo "completed all"

