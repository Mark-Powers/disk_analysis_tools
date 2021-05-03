A collection of various tools to collect hard drive data in real time

- `log_analysis`: Scripts to analyze log files from `hd\_hammer`
- `hd_hammer/`: A custom tool that generates a workload on a hard drive and
  measures from it.
- `fio_scripts/`: Some scripts that run fio (replaced by `hd_hammer`)

# Install Guide
These steps were used to install this repository from a fresh installation of Xubuntu.

1. Follow [this guide](https://www.myheap.com/cnc-stuff/linuxcnc-emc2/92-my-heap-articles/computer-numerical-control/linuxcnc/written-tutorials/198-compiling-a-realtime-kernel-for-linuxcnc.html) to install the linux RT patch.
2. Install needed software:
  - Essential tools: `sudo apt install git curl smartmontools`
  - `log_analysis` dependencies: 
```
sudo apt install python3-pip
pip3 install matplotlib spectrum nfft bayesian-changepoint-detection
pip3 install --user numpy scipy==1.2.1 matplotlib
```
  - `hd\_hammer` acceleration dependencies: 
```
curl -fsSL https://www.phidgets.com/downloads/setup_linux | sudo -E bash -
sudo apt-get install -y libphidget22 libphidget22-dev
```
3. Clone this repository

# Installing a new hard drive
When installing a new hard drive, not much needs to be done to format it, since
we are just using the raw disk for measurements anyway, and reading from the raw
`/dev` file.

This command will print out the serial numbers of disks, and which device they
are on. It ignores the main SDD (`sda`).

`ls -l /dev/disk/by-id/ata* | grep -E 'sd[b-z]'`

Next, update `hd_hammer/run.sh` to disable write caching on this device.
Assuming it is on `/dev/sdg`, the command would be:

`sudo hdparm -f -F -W 0 /dev/sdg`

Update the for loop max bound and the case statement to run this drive and give
it a prefix. Add to the top of `hd_hammer/config.h` to include the device in the
array of filenames, and the array of file sizes to use (i.e. the disk size).

# hd_hammer usage
1. Modify `config.h` parameters as desired
2. Build by running `make` in the `hd\_hammer` directory
- executable `time` runs timing tests.
  - Usage: `./time -f FILE\_NO -l LOG -r`
  - Here, `FILE\_NO` is which file to run on (which disk), `LOG` is the log file name, and `-r` indicates to run tests (as opposed to `-a` which allocates files (used when doing direct IO without using the raw disk))
  - See `config.h` for configuration options prior to building
- executable `time_set_sequence` runs timing tests using a set pattern to measure the latency of specific disk movements. The sequence follows the pattern: 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, ... which if running for long enough would test all jumps from all positions.
3. Modify `run.sh`. Here, set `type_label` so that generated log files have useful names. You can also modify the number of iterations run
4. Run `run.sh` to run many tests in sequence.

# analysis.py usage
1. Add a new list of prefixes to form groups with in `data.py`
2. Modify the line `groups_with = ` to use your new group
3. Modify the `main()` function to run any tests you want.

# Interpreting log files
Log files are named indicating which hard drive it is measured from, the type of
vibration (none indicates none) giving both Hz and rough velocity (sometimes
this number increased/decreased during the test). The first 10 seconds of the
run are a warmup period, and so are not included in the log file. The columns of
each csv file are `timestamp, latency, position, accel_x, accel_y, accel_z`
where the latency is either in milliseconds or CPU cycles elapsed (set in
`config.h`), the position is the random position where the read/write occured
(if not using random, then this field will just be 0/undefined). The
acceleration measures come from the accelerometer, where the z axis is vertical.

Log files have lines starting with # to express runtime parameters. These are
ignored when reading in the data, but still are present for later usage.

# Smart data
Smart data is gotten from the disk at the start of a series of iterations using
`smart.py`. It is rerun at the end of the iterations. This data is put into one
log file with the heads BEFORE and AFTER to indicate which series of data is
from which run. Using `smart_diff.py`, you can check for any difference between
these two runs. To run these in bulk, run `cd smart; ls | xargs -l1 ../smart_diff.py`. 
"Vendor specific attributes" are ones that aren't known in
detail.


