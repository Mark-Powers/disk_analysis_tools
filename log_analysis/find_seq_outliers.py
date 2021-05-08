import sys

last = 0

entries = []
times = []
with open(sys.argv[1]) as f:
    for line in f.readlines():
        if line.startswith("#"):
            continue
        parts = line.split(",")
        entries.append(int(parts[1]))
        times.append(float(parts[0]))
for i in range(len(entries)):
    if entries[i] > 7.3e7:
        print(i, entries[i], i - last, times[i]-entries[last], sep="\t")
        last = i
