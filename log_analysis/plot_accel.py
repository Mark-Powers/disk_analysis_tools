#!/usr/bin/python3

import numpy
import matplotlib.pyplot as plt
import sys
from spectrum import Periodogram
import spectrum

from scipy.fft import fft, fftfreq


filename = "log.csv"
if len(sys.argv) > 1:
    filename = sys.argv[1]
data = numpy.genfromtxt(filename, delimiter=',').transpose()
x = data[0,:]
a_x = data[1,:]
a_y = data[2,:]
a_z = data[3,:]
#plt.ylim(0, 200)
plt.title(filename)
plt.xlabel("time (s)")
plt.ylabel("acceleration")
plt.plot(x, a_x)
plt.plot(x, a_y)
plt.plot(x, a_z)
plt.show()

N = len(a_z)
# sample spacing
T = (x[-1]-x[0])/N
yf = fft(a_z)
xf = fftfreq(N, T)[:N//2]

plt.plot(xf[1:], 2.0/N * numpy.abs(yf[1:N//2]))
plt.grid()
plt.show()

'''
p = Periodogram(a_z, sampling=len(a_x)/(x[-1]-x[0]))
p.run()
print(p)
plt.plot(p.frequencies(), 10 * spectrum.tools.log10(p.psd) )
plt.show()
'''
