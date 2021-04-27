A collection of various tools to collect hard drive data in real time

- `analysis.py`: A script to analyze log files from `hd\_hammer`
- `data.py`: Lists of different data sets
- `filebench_scripts/`: Some sample scripts from filebench
- `fio_scripts/`: Some scripts that run fio
- `hd_hammer/`: A custom tool that generates a workload on a hard drive and
  measures from it.


# Install Guide
These steps were used to install this repository from a fresh installation of Xubuntu.

1. Follow [this guide](https://www.myheap.com/cnc-stuff/linuxcnc-emc2/92-my-heap-articles/computer-numerical-control/linuxcnc/written-tutorials/198-compiling-a-realtime-kernel-for-linuxcnc.html) to install the linux RT patch.
2. Install needed software:
  - Essential tools: `sudo apt install git curl smartmontools`
  - `analysis.py` dependencies: 
```
sudo apt install python3-pip
pip3 install matplotlib spectrum nfft bayesian-changepoint-detection
pip3 install --user numpy scipy==1.2.1 matplotlib
```
  - `hd\_hammer` dependencies: 
```
curl -fsSL https://www.phidgets.com/downloads/setup_linux | sudo -E bash -
sudo apt-get install -y libphidget22 libphidget22-dev
```
3. Clone this repository

# hd\_hammer usage
1. Modify `config.h` parameters as desired
2. Build by running `make` in the `hd\_hammer` directory
- executable `time` runs timing tests.
  - Usage: `./time -f FILE\_NO -l LOG -r`
  - Here, `FILE\_NO` is which file to run on (which disk), `LOG` is the log file name, and `-r` indicates to run tests (as opposed to `-a` which allocates files (used when doing direct IO without using the raw disk))
  - See `config.h` for configuration options prior to building
- executable `time_set_sequence` runs timing tests using a set pattern to measure the latency of specific disk movements. The sequence follows the pattern: 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, ... which if running for long enough would test all jumps from all positions.
3. Modify `run.sh`. Here, set `type_label` so that generated log files have useful names. You can also modify the number of iterations run
4. Run `run.sh` to run many tests in sequence.
