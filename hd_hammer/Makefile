all: time

clean:
	rm sdlview time fftview showmax

CFLAGS= -Wall -O3 -march=native -g
CFLAGSD= -Wall -O0 -g -march=native

time : time2.c
	gcc $(CFLAGS) -o time time2.c

