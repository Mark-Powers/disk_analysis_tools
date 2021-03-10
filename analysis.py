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
from scipy.stats import pearsonr
from scipy.interpolate import CubicSpline
from scipy.fft import fft, ifft

groups_include = [
        "none_lat",
        "vib_5hz_lat",
        "vib_10hz_lat",
        "vib_20hz_lat",
        "vib_30hz_lat",
    ]
def main():
    dirs = ["/home/mark/dev/disk_analysis/basic/",
            "/home/mark/dev/disk_analysis/pen_taps_64_queue/",
            "/home/mark/dev/disk_analysis/pen_taps_4_queue/"]
    #base_dir = dirs[1]
    base_dir = "/media/mark/Backup/fio/fig/"

    groups = {}
    for g in groups_include:
        groups[g] = filter_dir(base_dir, g)

    data_sets = {}
    for name, files in groups.items():
        data_sets[name] = []
        read_csv(files, data_sets[name])
    normalize_size(data_sets)

    #plot_single(ds_none[0], "none 0")
    #plot_each(data_sets, False)
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
        if "lat" in f:
            data = numpy.genfromtxt(f, delimiter=',').transpose()
            ds_list.append({ 
                "t": data[0,:],
                "ds": data[1,:],
                "name": os.path.basename(f)
                })

def plot_single(ds, label):
    plt.figure(label)
    plt.plot(ds["t"], ds["ds"])
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
            ax.set_ylabel(name)
            ax.plot(ds_list[i]["t"], ds_list[i]["ds"])
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

def run_pearson_correlation(data_sets):
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
    fig = plt.figure("Pearson Coefficients")
    gs = fig.add_gridspec(2)#, hspace=0)
    axs = gs.subplots(sharex=True, sharey=True)
    axs[0].set_ylim(0, 1)
    for f, scores in pearson_scores["none"].items():
        axs[0].plot(scores)
    axs[0].set_title("none")
    for f, scores in pearson_scores["vib"].items():
        axs[1].plot(scores)
    axs[1].set_title("vibration")
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
            axs[i][0].plot(x, y)
        for i in range(len(ds_none)):
            data = ds_vibration[i]["ds"]
            p = spectrum.Periodogram(data, sampling=(len(data)/30))
            p.run()
            x, y = p.frequencies(), 10 * spectrum.tools.log10(p.psd)
            axs[i][1].plot(x, y)
        plt.show()

def index_of_duplicate(ds):
    for i in range(len(ds["t"])-1):
        if ds["t"][i] == ds["t"][i+1]:
            return i
    return len(ds["t"])

def run_i_periodogram(data_sets):
    sampling_rate = 25
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
            axs[i][0].plot(x, y)
        for i in range(len(ds_none)):
            index = index_of_duplicate(ds_none[i])
            cs = CubicSpline(ds_vibration[i]["t"][:index], ds_vibration[i]["ds"][:index])
            times = numpy.arange(ds_vibration[i]["t"][0],ds_vibration[i]["t"][-1],1000/sampling_rate)
            values = cs(times)
            p = spectrum.Periodogram(values, sampling=sampling_rate)
            p.run()
            x, y = p.frequencies(), 10 * spectrum.tools.log10(p.psd)
            axs[i][1].plot(x, y)
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
    sampling_rate = 25
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
