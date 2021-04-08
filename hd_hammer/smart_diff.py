#!/usr/bin/python3
import sys

file_name = sys.argv[1]
before = {}
after = {}
with open(file_name) as f:
    current = before
    for line in f.readlines():
        line = line.strip()
        if line.startswith("before") or line.startswith("device") or line.startswith("metadata"):
            continue
        elif line.startswith("after"):
            current = after
        else:
            parts = line.rsplit(",", 1)
            current[parts[0]] = parts[1]

for key in before:
    if before[key] != after[key]:
        print(key, before[key], after[key], sep="\t")

