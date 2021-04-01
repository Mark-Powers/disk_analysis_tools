#!/usr/bin/python3
# Questions
# - Smaller portion of time having noise?
# - Spectral analysis?

import csv
import numpy
import itertools
import os
import matplotlib.pyplot as plt
import spectrum
import nfft
import math

import scipy
from scipy.stats import pearsonr, kstest
from scipy.interpolate import CubicSpline
from scipy.fft import fft, ifft

groups_include = [
        "sdd-write-none",
        "sdd-write-10hz-0.02G",
        "sdd-write-20hz-0.07G",
        "sdd-write-25hz-0.12G",
        "sdd-write-30hz-0.10G",
        "sdd-write-35hz-0.12G",
        "sdd-write-40hz-0.17G",
        "sdd-write-45hz-0.13G",
        "sdd-write-50hz-0.16G",
    ]
'''
        "-write-none",
        "-write-10hz-0.02G",
        "-write-10hz-",
        "-write-20hz-0.07G",
        "-write-25hz-0.12G",
        "-write-30hz-0.10G",
        "-write-35hz-0.12G",
        "-write-40hz-0.17G",
        "-write-45hz-0.13G",
        "-write-50hz-0.16G",
'''
def main():
    base_dir = "/home/markp/disk_analysis_tools/hd_hammer/logs/"

    groups = {}
    for g in groups_include:
        groups[g] = filter_dir(base_dir, g)

    data_sets = {}
    for name, files in groups.items():
        data_sets[name] = []
        read_csv(files, data_sets[name])
    #normalize_size(data_sets)

    #plot_single(data_sets["sdb-write-0-1k_"][0], "0")
    #plot_single(data_sets["sdb-write-seq-1k_"][0], "seq")
    #plot_single(data_sets["sdb-read-r_"][0], "rand")
    #plot_single(data_sets["sdb-read_"][0], "rand")
    #plot_single(data_sets["sdb-read-avg_"][0], "rand")
    #plot_each(data_sets, False)
    summary(data_sets)
    run_kstest(data_sets)
    #plot_each(data_sets, True)
    #run_pearson_correlation(data_sets)
    #run_fft(data_sets)
    #run_i_fft(data_sets)
    #run_nfft(data_sets)
    #run_periodogram(data_sets)
    #run_i_periodogram(data_sets)

def filter_dir(base_dir, phrase):
    items = []
    for x in os.listdir(base_dir):
        if phrase in x:
            items.append(base_dir + x)
    return items

def read_csv(file_list, ds_list):
    for f in file_list:
        data = numpy.genfromtxt(f, delimiter=',').transpose()
        print(f)
        ds_list.append({ 
            "t": data[0,:],
            "ds": data[1,:],
            "name": os.path.basename(f)
            })

def summary(groups):
    s = "name"
    print(f"{s:20s}\t50%\t\t75%\t\t90%\t\t99%\t\t99.5%\t\tmean")
    for name, ds_list in groups.items():
        #print(name)
        x50 = []
        x75 = []
        x90 = []
        x99 = []
        x995 = []
        xMean = []
        for ds in ds_list:
            #print(f'\t{numpy.percentile(ds["ds"], 50):.2f}\t{numpy.percentile(ds["ds"], 75):.2f}\t{numpy.percentile(ds["ds"], 90):.2f}\t{numpy.percentile(ds["ds"], 99):.2f}\t{numpy.percentile(ds["ds"], 99.5):.2f}\t{numpy.mean(ds["ds"]):.2f}')
            x50.append(numpy.percentile(ds["ds"], 50))
            x75.append(numpy.percentile(ds["ds"], 75))
            x90.append(numpy.percentile(ds["ds"], 90))
            x99.append(numpy.percentile(ds["ds"], 99))
            x995.append(numpy.percentile(ds["ds"], 99.5))
            xMean.append(numpy.mean(ds["ds"]))
        print(f"{name:20s}\t{mean(x50):.2f}\t{mean(x75):.2f}\t{mean(x90):.2f}\t{mean(x99):.2f}\t{mean(x995):.2f}\t{mean(xMean):.2f}")

def mean(lst):
    return sum(lst)/len(lst)

def threshhold(groups, thres, prefix=""):
    for name, ds_list in groups.items():
        count = 0
        for ds in ds_list:
            for item in ds["ds"]:
                if item > thres:
                    count += 1
        print(prefix, name, count)

def plot_single(ds, label, line=True):
    plt.figure(label)
    if line:
        plt.plot(ds["t"], ds["ds"])
    else:
        plt.plot(ds["t"], ds["ds"], ".")
    plt.show()


def plot_each(groups, on_one=False):
    for name, ds_list in groups.items():
        fig = plt.figure(name)
        size = 1 if on_one else len(ds_list)
        gs = fig.add_gridspec(size, hspace=1)
        axs = gs.subplots(sharex=True, sharey=True)
        for i in range(len(ds_list)):
            name = ds_list[i]["name"].split("_")[-1][:-4]
            ax = axs if on_one else axs[i]
            if on_one:
                ax.set_ylabel(name)
            else:
                ax.set_ylabel("CPU cycles")
            ax.plot(ds_list[i]["t"], ds_list[i]["ds"], '.')
        plt.show()


def normalize_size(groups):
    offset = 50
    m = None
    for name, ds_list in groups.items():
        for ds in ds_list:
            if m is None or m > len(ds["ds"]):
                m = len(ds["ds"])
    # duplicate measurements don't work for cubic spline
    # TODO when included, allows running splide, but NFFT does not show anything.
    m -= offset
    if m % 2 == 1:
        m -= 1
    for name, ds_list in groups.items():
        for ds in ds_list:
            ds["t"] = ds["t"][offset:m]
            ds["ds"] = ds["ds"][offset:m]

def run_kstest(data_sets):
    # Calculate pearson scores for (x_i, y_j)
    pearson_scores = {}
    name1, ds_list1 = list(data_sets.items())[0]
    for i in range(len(data_sets)):
        name2, ds_list2 = list(data_sets.items())[i]
        for element in itertools.product(ds_list1, ds_list2):
            name1 = element[0]["name"]
            name2 = element[1]["name"]
            if name1 == name2:
                continue
            p = kstest(element[0]["ds"], element[1]["ds"])
            if name1 not in pearson_scores:
                pearson_scores[name1] = {}
            if p[1] > 0.01:
                pearson_scores[name1][name2] = p[0]

    # Print scores
    print(pearson_scores)
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
        #ax.set_xlim(0, 1)
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

def run_pearson_correlation(data_sets):
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

'''
    # Plot averages
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
            ax.plot(numpy.mean(scores), 0, '.')
        i+=1
    plt.show()

    # Plot scores on line plot
    fig = plt.figure("Pearson Coefficients")
    size = len(groups_include)
    gs = fig.add_gridspec(size, hspace=1)
    axs = gs.subplots(sharex=True, sharey=True)
    i = 0
    for group_name in groups_include:
        ax = axs[i]
        ax.set_ylim(0, 1)
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
            ax.plot(scores)
        i+=1
    plt.show()
    '''

# https://stackoverflow.com/questions/57435660/how-to-compute-nfft
#def compute_nfft(sample_values, sample_instants):
def compute_nfft(sample_instants, sample_values):
    N = len(sample_instants)
    T = sample_instants[-1] - sample_instants[0]
    x = numpy.linspace(0.0, 1.0 / (2.0 * T), N // 2)
    y = nfft.nfft(sample_instants, sample_values)
    y = 2.0 / N * numpy.abs(y[0:N // 2])
    #plt.plot(x, y)
    #plt.show()
    return (x, y)

def run_periodogram(data_sets):
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

def index_of_duplicate(ds):
    for i in range(len(ds["t"])-1):
        if ds["t"][i] == ds["t"][i+1]:
            return i
    return len(ds["t"])

def run_i_periodogram(data_sets):
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
            index = index_of_duplicate(ds_none[i])
            cs = CubicSpline(ds_none[i]["t"][:index], ds_none[i]["ds"][:index])
            times = numpy.arange(ds_none[i]["t"][0],ds_none[i]["t"][-1],1000/sampling_rate)
            values = cs(times)
            p = spectrum.Periodogram(values, sampling=sampling_rate)
            p.run()
            x, y = p.frequencies(), 10 * spectrum.tools.log10(p.psd)
            axs[i][0].plot(x[2:], y[2:])
        for i in range(len(ds_none)):
            index = index_of_duplicate(ds_none[i])
            cs = CubicSpline(ds_vibration[i]["t"][:index], ds_vibration[i]["ds"][:index])
            times = numpy.arange(ds_vibration[i]["t"][0],ds_vibration[i]["t"][-1],1000/sampling_rate)
            values = cs(times)
            p = spectrum.Periodogram(values, sampling=sampling_rate)
            p.run()
            x, y = p.frequencies(), 10 * spectrum.tools.log10(p.psd)
            axs[i][1].plot(x[2:], y[2:])
        plt.show()

def run_nfft(data_sets):
    #compute_nfft(instants, values)
    # TODO what is going on?
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
            index = index_of_duplicate(ds_none[i])
            cs = CubicSpline(ds_none[i]["t"][:index], ds_none[i]["ds"][:index])
            times = numpy.arange(ds_none[i]["t"][0],ds_none[i]["t"][-1],1000/sampling_rate)
            values = cs(times)
            y = fft(values)[1:]
            axs[i][0].plot(y)
        for i in range(len(ds_none)):
            index = index_of_duplicate(ds_vibration[i])
            cs = CubicSpline(ds_vibration[i]["t"][:index], ds_vibration[i]["ds"][:index])
            times = numpy.arange(ds_vibration[i]["t"][0],ds_vibration[i]["t"][-1],1000/sampling_rate)
            values = cs(times)
            y = fft(values)[1:]
            axs[i][1].plot(y)
        plt.show()

if __name__ == "__main__":
    main()
