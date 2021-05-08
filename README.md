A collection of various tools to collect hard drive data in real time

- `log_analysis/`: Various scripts to analyze log files from `hd\_hammer`
- `hd_hammer/`: A custom tool that generates a workload on a hard drive and
  measures from it.
- `img/`: Images that showcase various results. These are used as justification
  for parameters to `hd_hammer`
- `fio_scripts/`: Some scripts that run fio (replaced by `hd_hammer`)

# Install Guide
These steps were used to install this repository from a fresh installation of Xubuntu.

1. Follow [this guide](https://www.myheap.com/cnc-stuff/linuxcnc-emc2/92-my-heap-articles/computer-numerical-control/linuxcnc/written-tutorials/198-compiling-a-realtime-kernel-for-linuxcnc.html) to install the linux RT patch.
2. Install needed software:
  - Essential tools: 
```
sudo apt install git curl smartmontools
```
  - Install `log_analysis` dependencies: 
```
sudo apt install python3-pip
pip3 install --user numpy scipy==1.2.1 matplotlib
pip3 install spectrum nfft
```
  - Install `hd_hammer` accelerometer dependencies: 
```
curl -fsSL https://www.phidgets.com/downloads/setup_linux | sudo -E bash -
sudo apt-get install -y libphidget22 libphidget22-dev
```

3. Clone this repository. Look below for general usage instructions. 

## Installing a new hard drive
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

# Usage
## hd_hammer usage
1. Modify `config.h` parameters as desired
2. Build by running `make` in the `hd_hammer` directory
- Many executables exist (run `ls bin/time_*`) to see them. Examine `hd_hammer/README.md` for specific differences between them
  - Usage: `./bin/time_XYZ -f FILE\_NO -l LOG -r` (replacing the name of the
    executable file here)
  - Here, `FILE\_NO` is which file to run on (which disk), `LOG` is the log file name, and `-r` indicates to run tests (as opposed to `-a` which allocates files (used when doing direct IO without using the raw disk))
  - See `config.h` for configuration options prior to building
- Measure acceleration with `./bin/accel` and redirect the output into a log
  file.
3. Modify `run.sh`. Here, set `type_label` so that generated log files have useful names. You can also modify the number of iterations run, and which executable to test with
4. Run `run.sh` to run many tests in sequence.
5. Alternatively, `run.sh` is best used when gathering log files that are not
   continuous. You can also run continuous expirements by running the executable
   manually with the usage above. You should disable write caching first (copy
   the line from `run.sh`)

## log_analysis usage
### analysis.py usage
This file runs tests based on groups of log files (identified by filename
prefixes). For example, I may have log files prefixed by `wd_blue_rand_write-`
ending with `1.csv`, `2.csv`, etc. I could also create a similar dataset named
`wd_blue_rand_write_45Hz-` as a different set of data. The tests in
`analysis.py` are often made to find differences between these two sets.

1. Add a new list of prefixes to form groups with in `data.py` (e.g.
   `my_group = ["wd_blue_rand_write-", "wd_blue_rand_write_45Hz-"]`)
2. Modify the line `groups_with = ` to use your new group (e.g. `groups_with =
   my_group`)
3. Modify the `main()` function to run any test functions you want.
### Other script files:
- `compare_sequence_logs.py`: A script to compare two log files generated from
  the `alternating_sequence` seek algorithm. This compares the expected latency
  based on jump sizes. It was found that jump size alone does not determine the
  expected latency. This script likley does not need to be run any more.
- `plot_accel.py`: A script that takes the filename of a log from the
  accelerometer as an argument, and plots the three axes together. After this,
  it also plots an FFT.
- `plot.py`: Plots a single log file
- `plot_continuous_and_accel.py`: Takes in a continuous data log (a log of
  counts above percentiles), an acceleration data log, and a threshold as input. 
  Shows the acceleration data, highlighting vibration, and also the continuous
  data on the same x-axis. Continuous data is highlighted when above a given
  threshold. You may optionally add a fourth argument, which is the offset from
  the 95 percentile to use (i.e. 0 for 95, 1 for 96, 2 for 97, 3 for 998, and 4
  for 99).
- `smart.py`: When given no argument, looks for differences in the before and
  after smart data of all smart log files. When given an argument, logs smart
  data from the device specified. See the section below titled "Smart data" for
  more information about these log files

# Interpreting log files
Log files have lines starting with # to express runtime parameters. These are
ignored when reading in the data.

## Standard log files 
This section refers to log files generated by `time` executables that do not
contain the word "continuous" in the title, which are collected for a set period
of time.
Log files are named indicating which hard drive it is measured from, the type of
vibration (none indicates none) giving both Hz and rough velocity (sometimes
this number increased/decreased during the test). The first 10 seconds of the
run are a warmup period, and so are not included in the log file. The columns of
each csv file are `timestamp (sec), latency, position`
where the latency is either in milliseconds or CPU cycles elapsed (set in
`config.h`), the position is the random position where the read/write occured
(if not using random, then this field will just be 0/undefined).

## Continuous log files
This section refers to the log files generated by `time` executables that do
contain the word "continuous" in the title. 
The columns of each csv are `timestamp (sec), count over 95%tile, count over 96%tile, count over 97%time, count over 98%tile,
count over 99%tile`. Percentiles are calculated by first running a control period. Then,
for each new operation, we compare it against the percentiles and update the
counts, and log these. 

## Acceleration log files
Acceleration log files are generated by the `accel` executable (by redirecting
its standard output). 
Columns in these logs are `timestamp (sec), x accel, y accel, z accel`, where
acceleration is measured in Gs and the z axis is vertical. 

## Smart data
Smart data is gotten from the disk at the start of a series of iterations using
`smart.py`. It is rerun at the end of the iterations. This data is put into one
log file with the heads BEFORE and AFTER to indicate which series of data is
from which run. "Vendor specific attributes" are ones that aren't known in
detail.

# Example usage
This section demonstrates some example usage that has been run, and how plots
are generated.



