all: time

clean:
	rm sdlview time fftview showmax

CFLAGS= -Wall -O3 -march=native
CFLAGSD= -Wall -O0 -g -march=native

time : time.c
	gcc $(CFLAGS) -o time time.c

