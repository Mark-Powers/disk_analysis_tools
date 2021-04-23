all: time time_set_sequence

clean:
	rm time

CFLAGS= -Wall -O3 -march=native -pthread -lphidget22

time : time.c config.h
	gcc -o time time.c $(CFLAGS) 

time_set_sequence : time_set_sequence.c config.h
	gcc -o time_set_sequence time_set_sequence.c $(CFLAGS) 

