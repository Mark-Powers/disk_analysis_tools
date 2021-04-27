The Seagate Barracuda 5400 RPM drive seems to show vibration the best (compared
to the WD drives). We use this drive to compare across tests.

The each measurements is aligned along the `random` data column, so that the
same sequence of random reads/writes is compared to the identical sequence.
Then, we construct a histogram with 100 bins, and see which ones have unusally
high counts that are not the interval with 0 (where we expect most measurements
to lie). We see that rand-write has more bins being seen with high counts, and
has them for all 45Hz vibration series. Note that the counts for rand-read will
be higher in general, as the sampling rate is greater with reads. This indicates
that while both are impacted by vibration, writing shows the effects more.
```
difference values for rand-read:
	 sg_barr-rand-read-none2_1.csv
		 157 from -1505019.026666671 to -98449.5
	 sg_barr-rand-read-none2_3.csv
	 sg_barr-rand-read-none2_2.csv
	 sg_barr-rand-read-none3_3.csv
		 1338 from -4238931.239999995 to -16169.25
	 sg_barr-rand-read-none3_2.csv
		 1348 from 15567.333333328366 to 4237803.626666665
	 sg_barr-rand-read-none3_1.csv
	 sg_barr-rand-read-45Hz-7.6mmps_2.csv
		 770 from -4266455.040000007 to -43509.666666671634
	 sg_barr-rand-read-45Hz-7.6mmps_3.csv
		 575 from 53803.166666671634 to 4275500.546666667
	 sg_barr-rand-read-45Hz-7.6mmps_1.csv

difference values for rand-write:
	 sg_barr-rand-write-none2_2.csv
		 224 from -4408987.600000009 to -139461.7500000149
	 sg_barr-rand-write-none2_3.csv
	 sg_barr-rand-write-none2_1.csv
	 sg_barr-rand-write-none3_3.csv
	 sg_barr-rand-write-none3_1.csv
	 sg_barr-rand-write-none3_2.csv
	 sg_barr-rand-write-15Hz-7.6mmps_1.csv
		 80 from -4015897.8200000077 to -472479.40000000596
	 sg_barr-rand-write-15Hz-7.6mmps_2.csv
		 80 from -4768846.11999999 to -487353.58333332837
	 sg_barr-rand-write-15Hz-7.6mmps_3.csv
	 sg_barr-rand-write-30Hz-7.6mmps_3.csv
		 38 from -4031731.670000002 to -495264.90000000596
	 sg_barr-rand-write-30Hz-7.6mmps_2.csv
		 295 from -4317603.439999998 to -47372.4999999851
	 sg_barr-rand-write-30Hz-7.6mmps_1.csv
	 sg_barr-rand-write-45Hz-7.6mmps_2.csv
		 101 from -7481990.513333321 to -388109.549999997
		 35 from 27987414.303333342 to 35081295.26666668
		 91 from 35081295.26666668 to 42175176.23000002
	 sg_barr-rand-write-45Hz-7.6mmps_1.csv
		 28 from 42463915.5 to 92258973.0
	 sg_barr-rand-write-45Hz-7.6mmps_3.csv
		 147 from 13326899.766666666 to 73714129.65
```

