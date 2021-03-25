all: time

clean:
	rm time

CFLAGS= -Wall -O3 -march=native
CFLAGSD= -Wall -O0 -g -march=native

time : time_raw.c
	gcc $(CFLAGSD) -o time time_raw.c

