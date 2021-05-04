#!/usr/bin/python3


entries = []
last_pos = None
with open("../log_big_sg.csv") as f:
    for line in f.readlines():
        if line.startswith("#"):
            continue
        parts = line.split(",")
        cpuLatency = int(parts[1])
        pos = int(parts[2])
        entries.append(cpuLatency)

def less_than_x(arr, x):
    count = 0
    for i in arr:
        if i < x:
            count += 1
    return count

print(less_than_x(entries, 8.5e7)/len(entries))
print(less_than_x(entries, 7.84e7)/len(entries))
print(less_than_x(entries, 7.4e7)/len(entries))

