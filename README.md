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
  - Essential tools: `sudo apt install git`
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

