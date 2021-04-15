#!/usr/bin/python3
import sys

file_name = sys.argv[1]

print(file_name, end="")
before = {}
after = {}
with open(file_name) as f:
    current = before
    for line in f.readlines():
        line = line.strip()
        if line.startswith("before") or line.startswith("device") or line.startswith("metadata") or line.startswith("name") or line.startswith("#"):
            continue
        elif line.startswith("after"):
            current = after
        else:
            parts = line.rsplit(",", 1)
            current[parts[0]] = parts[1]

end = False
for key in before:
    if key in after and before[key] != after[key]:
        if not end:
            print()
        end = True
        print("\t", key, before[key], after[key], sep="\t")
if not end:
    print()

