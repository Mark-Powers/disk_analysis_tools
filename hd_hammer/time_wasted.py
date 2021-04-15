import sys

# given an integer, calculate the total portion of time actually doing work

t = int(sys.argv[1])
f = open("log.csv")
nums = [int(line.strip().split(",")[1]) for line in f.readlines()]
print((t - sum(nums)) / t)

