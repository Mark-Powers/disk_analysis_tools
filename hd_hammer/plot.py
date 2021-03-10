#!/usr/bin/python3

import numpy
import matplotlib.pyplot as plt

data = numpy.genfromtxt("log.csv", delimiter=',').transpose()
x = data[0,:]
x = [p - x[0] for p in x]
y = data[1,:]
plt.xlabel("time (s)")
plt.ylabel("latency (ms)")
plt.plot(x, y)
plt.show()
