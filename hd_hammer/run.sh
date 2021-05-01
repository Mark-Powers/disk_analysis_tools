#!/bin/bash

iterations=3

# Label for log files for this type of test
type_label=rand-write-none-big_jump
# Which executable to use
#exe=./bin/time_standard
exe=./bin/time_big_jump

set -e

make

# Disable write cache, and flush stuff
sudo hdparm -f -F -W 0 /dev/sdb
sudo hdparm -f -F -W 0 /dev/sdc
sudo hdparm -f -F -W 0 /dev/sdd
sudo hdparm -f -F -W 0 /dev/sde
sudo hdparm -f -F -W 0 /dev/sdf

file=1
for (( file=1; file<=5; file++ ))
do
	# Get prefix and mountpoint for device.
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

	# Get the smart attributes
	smart_log=smart/$prefix\_smart.csv
	echo "before" > $smart_log
	sudo ../log_analysis/smart.py $device >> $smart_log

	# Allocate (only needed for FS tests)
	#sudo ./time -a -f $file

	# Run iterations
	for (( i=1; i<=$iterations; i++ ))
	do
		log_file=logs/$prefix\_$i.csv
		sudo nice -n -20 $exe -r -f $file -l $log_file
		echo "$log_file complete"
	done

	# Get new smart attributes
	echo "after" >> $smart_log
	sudo ../log_analysis/smart.py $device >> $smart_log
done

sudo hdparm -W 1 /dev/sdb
sudo hdparm -W 1 /dev/sdc
sudo hdparm -W 1 /dev/sdd
sudo hdparm -W 1 /dev/sde
sudo hdparm -W 1 /dev/sdf

echo "completed all"

