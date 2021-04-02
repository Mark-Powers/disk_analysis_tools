all: time

clean:
	rm time

CFLAGS= -Wall -O3 -march=native -pthread

time : time.c
	gcc $(CFLAGS) -o time time.c

