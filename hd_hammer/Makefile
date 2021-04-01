all: time

clean:
	rm time

CFLAGS= -Wall -O3 -march=native
CFLAGSD= -Wall -O0 -g -march=native

time : time.c
	gcc $(CFLAGSD) -o time time.c

