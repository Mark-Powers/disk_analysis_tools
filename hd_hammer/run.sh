#!/bin/bash

iterations=3
file=0
type_label=new-none2-rand-write

set -e

make

sudo hdparm -W 0 /dev/sdb
sudo hdparm -W 0 /dev/sdc
sudo hdparm -W 0 /dev/sdd
sudo hdparm -W 0 /dev/sde
sudo hdparm -W 0 /dev/sdf


for (( file=1; file<=5; file++ ))
do
	case $file in
		1)
			prefix=wd_blue-$type_label
			device=/dev/sdb
			;;
		2)
			prefix=wd_red-$type_label
			device=/dev/sdc
			;;
		3)
			prefix=wd_gold_1-$type_label
			device=/dev/sdd
			;;
		4)
			prefix=wd_gold_10-$type_label
			device=/dev/sde
			;;
		5)
			prefix=sg_barr-$type_label
			device=/dev/sdf
			;;
	esac
	
	smart_log=smart/$prefix\_smart.csv
	echo "before" > $smart_log
	sudo ./smart.py $device >> $smart_log

	#sudo ./time -a -f $file
	
	for (( i=1; i<=$iterations; i++ ))
	do
		log_file=logs/$prefix\_$i.csv
		sudo nice -n -20 ./time -r -f $file -l $log_file
#		head -n -1 $log_file | sponge $log_file
		echo "$log_file complete"
	done

	echo "after" >> $smart_log
	sudo ./smart.py $device >> $smart_log
done

sudo hdparm -W 1 /dev/sdb
sudo hdparm -W 1 /dev/sdc
sudo hdparm -W 1 /dev/sdd
sudo hdparm -W 1 /dev/sde
sudo hdparm -W 1 /dev/sdf

echo "completed all"

