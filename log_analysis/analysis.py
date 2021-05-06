#!/usr/bin/python3

import csv, itertools, os, math
import numpy
import matplotlib.pyplot as plt
import spectrum
import nfft
import scipy
from scipy.stats import pearsonr, kstest
from scipy.interpolate import CubicSpline
from scipy.fft import fft, ifft
import scipy.fftpack
from functools import partial
from data import *

# Which group to use, from data.py
groups_include = sg_barr_big_jump

def main():
    # Which directory to search for log files in
    base_dir = "../hd_hammer/logs/"

    # Construct groups of filenames based on names in groups_include
    groups = {}
    for g in groups_include:
        groups[g] = filter_dir(base_dir, g)

    # Read the csv files into data_sets
    data_sets = {}
    for name, files in groups.items():
        data_sets[name] = []
        read_csv(files, data_sets[name])
    # Some tests require all files to have the same size
    normalize_size(data_sets)

    # Run tests on data_sets
    '''
    sample tests:
    plot_each
        summary
        threshhold
        plot_each
        run_kstest
        plot_sparsity
        align
        run_fft
        run_i_fft
        run_nfft
        run_periodogram
        run_i_periodogram
        run_pearson_correlation
    '''
    summary(data_sets)
    rolling_avg(data_sets)
    #plot_each(data_sets, True)
    #align(data_sets, thres=20, plotPDF=True)

def rolling_avg(groups):
    none_list = groups[groups_include[0]]
    y = []
    for ds in none_list:
        for j in range(0, len(ds["ds"])):
            y.append(ds["ds"][j])
    mx = numpy.percentile(y, 99)
    print(mx)
    size = 100
    for name, ds_list in groups.items():
        for ds in ds_list:
            j = 0
            while j < len(ds["ds"])-size:
                c = above_threshold(ds["ds"][j:j+size], mx)
                # If more than 5/100 above 97%tile, then vibration
                if c > 3:
                    print(ds["name"], "vibration at", ds["t"][j], c)
                    j += size
                j += 1
            plt.plot(ds["t"], ds["ds"])
        plt.show()


def filter_dir(base_dir, phrase):
    '''
    Returns a list of files within the directory base_dir that contain
    the given phrase
    '''
    items = []
    for x in os.listdir(base_dir):
        if phrase in x:
            items.append(base_dir + x)
    return items

def read_csv(file_list, ds_list):
    '''
    For each file in file_list, read it in and add it to the list ds_list
    '''
    for f in file_list:
        data = numpy.genfromtxt(f, delimiter=',').transpose()
        # Gives names to each column from the data
        ds_list.append({ 
            "t": data[0,:],
            "ds": data[1,:],
            "rand": data[2,:],
            "name": os.path.basename(f)
            })

def position_of(v, arr):
    '''
    Find the first index of v in arr
    '''
    for i in range(len(arr)):
        if arr[i] == v:
            return i
    return -1

def align(groups, plotPDF=False, thres=20):
    '''
    Align all elements of a data series based on the "rand" field.
    '''
    ds1 = None
    ds2 = None
    # Find random number that shows up in all ds rand arrays
    max_random_num = None
    for name, ds_list in groups.items():
        for ds in ds_list:
            if max_random_num is None:
                max_random_num = ds["rand"][0]
            else:
                pos = position_of(max_random_num, ds["rand"])
                if pos < 0:
                    max_random_num = ds["rand"][0]
    # Slice each field in all data series to start at the index of max_random_num
    for name, ds_list in groups.items():
        for ds in ds_list:
            i = position_of(max_random_num, ds["rand"])
            ds["ds"] = ds["ds"][i:]
            ds["t"] = ds["t"][i:]
            ds["rand"] = ds["rand"][i:]
    # Find the min_length of all data series
    min_length = None
    for name, ds_list in groups.items():
        for j in range(len(ds_list)-1, -1, -1):
            if min_length is None or len(ds_list[j]["ds"]) < min_length:
                min_length = len(ds_list[j]["ds"])
    # Average the series of the first test type (the no vibration series)
    none_avg = None
    for name, ds_list in groups.items():
        if name == groups_include[0]:
            avg = [0]*min_length
            for i in range(len(ds_list)):
                for j in range(min_length):
                    avg[j] += ds_list[i]["ds"][j]
            avg = [x / len(ds_list) for x in avg]
            none_avg = avg
    # Print the sum of all the difference values (normed by the length) between
    # the new aligned series
    print("difference values:")
    for name, ds_list in groups.items():
        if name != groups_include[0]:
            if plotPDF:
                plt.figure(name+" aligning")
                plt.ylabel("count")
                plt.xlabel("difference")
            for ds in ds_list:
                # Get the difference between this data series and the control average.
                l = array_sub(ds["ds"], none_avg)
                # Put it into a histogram
                l = numpy.histogram(l, bins=100)
                # Print the bins with high counts that don't overlap with 0
                print("\t", ds["name"])
                for i in range(len(l[0])):
                    if l[0][i] > thres and not(l[1][i] < 0 and l[1][i+1]>0):
                        print("\t\t", l[0][i], "from", l[1][i], "to", l[1][i+1])
                if plotPDF:
                    plt.plot(l[1][:-1], l[0])
            if plotPDF:
                plt.show()

def array_sub(a1, a2):
    '''
    Subtract each element in a1 to the corresponding element in a2
    '''
    return [x[0] - x[1] for x in zip(a1, a2)]

def summary(groups):
    '''
    Print out statistical summaries of the data
    '''
    print("Summary:")
    s = "name"
    print(f"{s:30s}\t50%\t\t75%\t\t90%\t\t99%\t\t99.5%\t\tmean")
    for name, ds_list in groups.items():
        #print(name)
        x50 = []
        x75 = []
        x90 = []
        x99 = []
        x995 = []
        xMean = []
        for ds in ds_list:
            x50.append(numpy.percentile(ds["ds"], 50))
            x75.append(numpy.percentile(ds["ds"], 75))
            x90.append(numpy.percentile(ds["ds"], 90))
            x99.append(numpy.percentile(ds["ds"], 99))
            x995.append(numpy.percentile(ds["ds"], 99.5))
            xMean.append(numpy.mean(ds["ds"]))
        print(f"{name:30s}", end="")
        print(f"\t{mean(x50):.0f}\t{mean(x75):.0f}\t{mean(x90):.0f}\t{mean(x99):.0f}\t{mean(x995):.0f}\t{mean(xMean):.0f}")

def mean(lst):
    '''
    Return the arithmetic mean of a list
    '''
    return sum(lst)/len(lst)

def above_threshold(lst, thres):
    '''
    Return the count of elements in lst greater than thres
    '''
    count = 0
    for i in lst:
        if i > thres:
            count += 1
    return count

def threshhold(groups, thres):
    '''
    Print out how many items in each dataset are above thres
    '''
    print("Checking threshold", thres) 
    for name, ds_list in groups.items():
        print(name)
        window = int(len(ds_list[0]["ds"])/30)
        times = []
        for ds in ds_list:
            i = 0
            while i < len(ds["ds"])-window:
                if above_threshold(ds["ds"][i:i+window], thres) >= 3:
                    times.append(ds["t"][i])
                    i += window
                    break
                i += int(window/10)
        if len(times) > 0:
            print(f"\t{times}")

def plot_sparsity(groups, thres):
    '''
    Attempt to plot the sparsity of elements
    '''
    for name, ds_list in groups.items():
        plt.figure(name + " sparsity")
        for ds in ds_list:
            data = ds["t"]
            i = 0
            lst = []
            while i < len(data)-2:
                lst.append(min(data[i+1] - data[i], data[i+2]-data[i+1]))
                i+=1
            plt.plot(lst)
        plt.show()

def plot_each(groups, on_one=False, use_time=True):
    '''
    Plot each group on a plot, with all data series from that group together, 
    or on their own subplots depending on on_one. If use_time is false, just
    plot using indices only as the x axis
    '''
    for name, ds_list in groups.items():
        fig = plt.figure(name)
        size = 1 if on_one else len(ds_list)
        gs = fig.add_gridspec(size, hspace=1)
        axs = gs.subplots(sharex=True, sharey=True)
        for i in range(len(ds_list)):
            name = ds_list[i]["name"].split("_")[-1][:-4]
            ax = axs if on_one else axs[i]
            ax.set_ylim(0, 2e8)
            if on_one:
                ax.set_ylabel(name)
            else:
                ax.set_ylabel("CPU cycles")
            if use_time:
                ax.plot(ds_list[i]["t"], ds_list[i]["ds"], '.')
            else:
                ax.plot(ds_list[i]["ds"], '.')
        plt.show()


def normalize_size(groups):
    '''
    Normalize all groups to the same size. Use offset to further trim away elements
    '''
    offset = 0
    m = None
    for name, ds_list in groups.items():
        for ds in ds_list:
            if m is None or m > len(ds["ds"]):
                m = len(ds["ds"])
    m -= offset
    if m % 2 == 1:
        m -= 1
    for name, ds_list in groups.items():
        for ds in ds_list:
            ds["t"] = ds["t"][offset:m]
            ds["ds"] = ds["ds"][offset:m]

def run_kstest(data_sets):
    '''
    Run the two sample ks-test on each series compared to the first control
    series, and print any with a significant p value
    '''
    print("KS-test")
    # Calculate ks-test scores for (x_i, y_j)
    scores = {}
    name1, ds_list1 = list(data_sets.items())[0]
    for i in range(len(data_sets)):
        name2, ds_list2 = list(data_sets.items())[i]
        print(name2, end="")
        count = 0
        total = 0
        for element in itertools.product(ds_list1, ds_list2):
            name1 = element[0]["name"]
            name2 = element[1]["name"]
            if name1 == name2:
                continue
            total += 1
            p = kstest(element[0]["ds"], element[1]["ds"])
            if name1 not in scores:
                scores[name1] = {}
            if p[1] < 0.05: # If p is low, there is significant difference
                scores[name1][name2] = p[0]
                count += 1
        print("\t", count, "significant difference comparisons out of", total)

def run_pearson_correlation(data_sets):
    '''
    calcualte the pearson correlation between the control group and all other groups
    '''
    # Calculate pearson scores for (x_i, y_j)
    pearson_scores = {}
    name1, ds_list1 = list(data_sets.items())[0]
    for i in range(len(data_sets)):
        name2, ds_list2 = list(data_sets.items())[i]
        print(name1, name2)
        for element in itertools.product(ds_list1, ds_list2):
            name1 = element[0]["name"]
            name2 = element[1]["name"]
            if name1 == name2:
                continue
            p = pearsonr(element[0]["ds"], element[1]["ds"])
            p = abs(p[0])
            if name1 not in pearson_scores:
                pearson_scores[name1] = {}
            pearson_scores[name1][name2] = p

    # Print scores
    print()
    for row_name, row in pearson_scores.items():
        for col_name, val in row.items():
            print(row_name, col_name, val)

    # Plot scores on one dimension
    fig = plt.figure("Pearson Coefficients Avg.")
    size = len(groups_include)
    gs = fig.add_gridspec(size, hspace=1)
    axs = gs.subplots(sharex=True, sharey=True)
    i = 0
    for group_name in groups_include:
        ax = axs[i]
        ax.set_xlim(0, 1)
        ax.set_ylabel(group_name)
        group_tests = {}
        for row_name, row in pearson_scores.items():
            for col_name, val in row.items():
                if group_name not in col_name:
                    continue
                if col_name not in group_tests:
                    group_tests[col_name] = []
                group_tests[col_name].append(val)
        print(group_tests)
        for test, scores in group_tests.items():
            ax.plot(scores, [0 for x in scores], '.')
        i+=1
    plt.show()


def run_periodogram(data_sets):
    '''
    Run a periodogram on the data sets
    '''
    ds_none = list(data_sets.items())[0][1]
    ds_none_name = list(data_sets.items())[0][0]
    for i in range(1, len(data_sets)):
        ds_vibration = list(data_sets.items())[i][1]
        ds_vibration_name = list(data_sets.items())[i][0]
        fig = plt.figure("Uniform Periodogram " + ds_none_name + " (left), " +ds_vibration_name + " (right)")
        gs = fig.add_gridspec(len(ds_none), 2, hspace=1)
        axs = gs.subplots()
        for i in range(len(ds_none)):
            data = ds_none[i]["ds"]
            p = spectrum.Periodogram(data, sampling=(len(data)/30))
            p.run()
            x, y = p.frequencies(), 10 * spectrum.tools.log10(p.psd)
            axs[i][0].plot(x[2:], y[2:])
        for i in range(len(ds_none)):
            data = ds_vibration[i]["ds"]
            p = spectrum.Periodogram(data, sampling=(len(data)/30))
            p.run()
            x, y = p.frequencies(), 10 * spectrum.tools.log10(p.psd)
            axs[i][1].plot(x[2:], y[2:])
        plt.show()


def run_i_periodogram(data_sets):
    '''
    run the periodogram using a cubic spline
    '''
    sampling_rate = 30
    ds_none = list(data_sets.items())[0][1]
    ds_none_name = list(data_sets.items())[0][0]
    for i in range(1, len(data_sets)):
        ds_vibration = list(data_sets.items())[i][1]
        ds_vibration_name = list(data_sets.items())[i][0]
        fig = plt.figure("Cubic Periodogram " + ds_none_name + " (left), " +ds_vibration_name + " (right)")

        gs = fig.add_gridspec(len(ds_none), 2, hspace=1)
        axs = gs.subplots()
        for i in range(len(ds_none)):
            cs = CubicSpline(ds_none[i]["t"], ds_none[i]["ds"])
            times = numpy.arange(ds_none[i]["t"][0],ds_none[i]["t"][-1],1000/sampling_rate)
            values = cs(times)
            p = spectrum.Periodogram(values, sampling=sampling_rate)
            p.run()
            x, y = p.frequencies(), 10 * spectrum.tools.log10(p.psd)
            axs[i][0].plot(x[2:], y[2:])
        for i in range(len(ds_none)):
            cs = CubicSpline(ds_vibration[i]["t"], ds_vibration[i]["ds"])
            times = numpy.arange(ds_vibration[i]["t"][0],ds_vibration[i]["t"][-1],1000/sampling_rate)
            values = cs(times)
            p = spectrum.Periodogram(values, sampling=sampling_rate)
            p.run()
            x, y = p.frequencies(), 10 * spectrum.tools.log10(p.psd)
            axs[i][1].plot(x[2:], y[2:])
        plt.show()

# https://stackoverflow.com/questions/57435660/how-to-compute-nfft
#def compute_nfft(sample_values, sample_instants):
def compute_nfft(sample_instants, sample_values):
    '''
    Following the procedure from the linked stackoverflow, compute a 
    nonuniform fft.
    '''
    N = len(sample_instants)
    T = sample_instants[-1] - sample_instants[0]
    x = numpy.linspace(0.0, 1.0 / (2.0 * T), N // 2)
    y = nfft.nfft(sample_instants, sample_values)
    y = 2.0 / N * numpy.abs(y[0:N // 2])
    #plt.plot(x, y)
    #plt.show()
    return (x, y)

def run_nfft(data_sets):
    '''
    Plot the nfft from compute_nfft
    '''
    ds_none = list(data_sets.items())[0][1]
    ds_none_name = list(data_sets.items())[0][0]
    for i in range(1, len(data_sets)):
        ds_vibration = list(data_sets.items())[i][1]
        ds_vibration_name = list(data_sets.items())[i][0]
        fig = plt.figure("Non-uniform FFT " + ds_none_name + " (left), " +ds_vibration_name + " (right)")
        gs = fig.add_gridspec(len(ds_none), 2, hspace=1)
        axs = gs.subplots()
        for i in range(len(ds_none)):
            x, y = compute_nfft(ds_none[i]["ds"], ds_none[i]["t"])
            axs[i][0].plot(x, y)
        for i in range(len(ds_none)):
            x, y = compute_nfft(ds_vibration[i]["ds"], ds_vibration[i]["t"])
            axs[i][1].plot(x, y)
        plt.show()

def run_fft(data_sets):
    '''
    Plot the fft 
    '''
    ds_none = list(data_sets.items())[0][1]
    ds_none_name = list(data_sets.items())[0][0]
    for i in range(1, len(data_sets)):
        ds_vibration = list(data_sets.items())[i][1]
        ds_vibration_name = list(data_sets.items())[i][0]
        fig = plt.figure("FFT " + ds_none_name + " (left), " +ds_vibration_name + " (right)")
        gs = fig.add_gridspec(len(ds_none), 2, hspace=1)
        axs = gs.subplots()
        for i in range(len(ds_none)):
            x = ds_none[i]["ds"]
            y = fft(x)[1:]
            axs[i][0].plot(y)
        for i in range(len(ds_none)):
            x = ds_vibration[i]["ds"]
            y = fft(x)[1:]
            axs[i][1].plot(y)
        plt.show()

def run_i_fft(data_sets):
    '''
    Plot the cubicspline interpolated fft
    '''
    sampling_rate = 30
    ds_none = list(data_sets.items())[0][1]
    ds_none_name = list(data_sets.items())[0][0]
    for i in range(1, len(data_sets)):
        ds_vibration = list(data_sets.items())[i][1]
        ds_vibration_name = list(data_sets.items())[i][0]
        fig = plt.figure("Cubic FFT " + ds_none_name + " (left), " +ds_vibration_name + " (right)")
        gs = fig.add_gridspec(len(ds_none), 2, hspace=1)
        axs = gs.subplots()
        for i in range(len(ds_none)):
            cs = CubicSpline(ds_none[i]["t"], ds_none[i]["ds"])
            times = numpy.arange(ds_none[i]["t"][0],ds_none[i]["t"][-1],1000/sampling_rate)
            values = cs(times)
            y = fft(values)[1:]
            axs[i][0].plot(y)
        for i in range(len(ds_none)):
            cs = CubicSpline(ds_vibration[i]["t"], ds_vibration[i]["ds"])
            times = numpy.arange(ds_vibration[i]["t"][0],ds_vibration[i]["t"][-1],1000/sampling_rate)
            values = cs(times)
            y = fft(values)[1:]
            axs[i][1].plot(y)
        plt.show()

if __name__ == "__main__":
    main()
