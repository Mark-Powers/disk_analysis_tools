#!/usr/bin/python3

import matplotlib.pyplot as plt

entries = {}
last_pos = None
with open("log.csv") as f:
    for line in f.readlines():
        parts = line.split(",")
        cpuLatency = int(parts[1])
        pos = int(parts[2])
        if last_pos is not None:
            jump = pos - last_pos
            if jump not in entries:
                entries[jump] = []
            entries[jump].append(cpuLatency)
        last_pos = pos
with open("log2.csv") as f:
    for line in f.readlines():
        parts = line.split(",")
        cpuLatency = int(parts[1])
        pos = int(parts[2])
        if last_pos is not None:
            jump = pos - last_pos
            if jump not in entries:
                entries[jump] = []
            entries[jump].append(cpuLatency)
        last_pos = pos
x = []
y = []

count_below_1e6 = 0
count_below_1e7 = 0
count_below_5e6 = 0
i = 0
total = 0
for jump, values in entries.items():
    i += 1
    # Skip warm up values
    if i < 1000:
        continue
    if len(values) == 2:
        total += 1
        diff = abs(values[1]-values[0])
        x.append(jump)
        y.append(diff)
        if diff < 1e6:
            count_below_1e6 += 1
        if diff < 5e6:
            count_below_5e6 += 1
        if diff < 1e7:
            count_below_1e7 += 1
    #print(jump, values, (values[1]-values[0]) < 1e7, sep="\t")
    #if i > 10000:
        #break
print(count_below_1e6/total, count_below_5e6/total, count_below_1e7/total)
plt.plot(x, y, ".")
plt.show()

