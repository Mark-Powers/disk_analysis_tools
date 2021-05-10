#!/usr/bin/python3

import sys
import numpy
import matplotlib.pyplot as plt

if len(sys.argv) <= 2:
    print("Usage: ./plot_continuous_and_accel.py <continuous_log> <accel_log>")
    sys.exit(0)

threshold = None
if len(sys.argv) > 3:
    threshold = int(sys.argv[3])

percentile_index = 1
if len(sys.argv) > 4:
    percentile_index = 1+int(sys.argv[4])

continuous_data = numpy.genfromtxt(sys.argv[1], delimiter=',').transpose()
times_raw = continuous_data[0,:]
count_raw = continuous_data[percentile_index,:]

accel_data = numpy.genfromtxt(sys.argv[2], delimiter=',').transpose()
accel_time = accel_data[0,:]
accel_x = accel_data[1,:]
accel_y = accel_data[2,:]
accel_z = accel_data[3,:]

def between(x, y, z):
    return y < x and x < z

def overlap(intLst1, intLst2):
    overlap = 0
    unoverlap = 0
    for int1 in intLst1:
        for int2 in intLst2:
            if between(int1[0], int2[0], int2[1]) and between(int1[1], int2[0], int2[1]) :
                overlap += int1[1]-int1[0]
                unoverlap += int2[1]-int1[1]
                unoverlap += int1[0]-int2[0]
            elif between(int2[0], int1[0], int1[1]) and between(int2[1], int1[0], int1[1]) :
                overlap += int2[1]-int2[0]
                unoverlap += int1[1]-int2[1]
                unoverlap += int2[0]-int1[0]
            elif between(int1[0], int2[0], int2[1]) :
                overlap += int2[1] - int1[0]
                unoverlap += int1[1]-int2[1]
                unoverlap += int1[0]-int2[0]
            elif between(int1[1], int2[0], int2[1]) :
                overlap += int1[1] - int2[0]
                unoverlap += int2[1]-int1[1]
                unoverlap += int2[0]-int1[0]
    return (overlap, unoverlap)

accel_senitivity = 0.045
accel_events = []
time_start = 0
time_end = 0
for index in range(0, len(accel_time), 10):
    if time_start == 0 and (max(*accel_z[index:index+10]) >= 1+accel_senitivity or min(*accel_z[index:index+10])<=1-accel_senitivity):
        time_start = accel_time[index]
    if max(*accel_z[index:index+10]) < 1+accel_senitivity and min(*accel_z[index:index+10])>1-accel_senitivity:
        time_end = accel_time[index]
        if time_start != 0 and time_start < time_end:
            accel_events.append([time_start, time_end])
        time_start = 0
        time_end = 0

if threshold is None:
    max_score = None
    max_threshold = 1
    best_events = []
    for threshold in range(30):#int(max(count_raw)+1)):
        events = []
        time_start = 0
        time_end = 0
        for index in range(len(times_raw)):
            if time_start == 0 and count_raw[index] >= threshold:
                time_start = times_raw[index]
            if count_raw[index] < threshold:
                time_end = times_raw[index]
                if time_start != 0 and time_start < time_end:
                    events.append([time_start, time_end])
                time_start = 0
                time_end = 0
        result = overlap(events, accel_events)
        weight = 0.20
        print(threshold, result, result[0] - weight * result[1])
        if max_score is None or result[0] - weight * result[1] > max_score:
            max_score = result[0] - weight * result[1]
            max_threshold = threshold
            best_events = events
    print("max threshold:", max_threshold, "max score:", max_score)
else:
    best_events = []
    time_start = 0
    time_end = 0
    for index in range(len(times_raw)):
        if time_start == 0 and count_raw[index] >= threshold:
            time_start = times_raw[index]
        if count_raw[index] < threshold:
            time_end = times_raw[index]
            if time_start != 0 and time_start < time_end:
                best_events.append([time_start, time_end])
            time_start = 0
            time_end = 0


fig=plt.figure("acceleration and vibration - " + sys.argv[1])
gs = fig.add_gridspec(2, hspace=0)
axs = gs.subplots(sharex=True)
axs[0].set_xlabel("time")
axs[0].set_ylabel("Gs")
axs[0].plot(accel_time, accel_z)
for event in accel_events:
    axs[0].axvspan(event[0], event[1], color='y', alpha=0.5, lw=0)
axs[1].plot([min(*accel_time, *times_raw), max(*accel_time, *times_raw)], [threshold, threshold])
axs[1].plot(times_raw, count_raw, color='r')
axs[1].set_ylabel("count above " + str(94+percentile_index)+ "%tile")
for event in best_events:
    axs[1].axvspan(event[0], event[1], color='y', alpha=0.5, lw=0)
plt.show()

