all: time

clean:
	rm time

CFLAGS= -Wall -O3 -march=native -pthread -lphidget22

time : time.c config.h
	gcc -o time time.c $(CFLAGS) 

