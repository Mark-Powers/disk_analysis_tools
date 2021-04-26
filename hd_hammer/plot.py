#!/usr/bin/python3

import numpy
import matplotlib.pyplot as plt
import sys

filename = "log.csv"
if len(sys.argv) > 1:
    filename = sys.argv[1]
data = numpy.genfromtxt(filename, delimiter=',').transpose()
x = data[0,:]
x = [p - x[0] for p in x]
y = data[1,:]
#plt.ylim(0, 200)
plt.title(filename)
plt.xlabel("time (s)")
plt.ylabel("cpu cycles")
plt.plot(x, y, '.')
plt.show()
