CFLAGS= -Wall -O3 -march=native -pthread -lphidget22

all: bin/time_standard bin/time_alternating bin/accel bin/time_max_seek


bin/accel: accel.c
	@printf "Building accel\n"
	@mkdir -p bin
	gcc -o bin/accel accel.c $(CFLAGS) 

bin/time.o: time.c
	@printf "Building time.o\n"
	@mkdir -p bin
	gcc -c time.c $(CFLAGS) 
	@mv time.o bin/

bin/max_seek_sequence.o : sequence_algorithm.h max_seek_sequence.c
	@printf "Building max_seek_sequence.o\n"
	@mkdir -p bin
	gcc -c max_seek_sequence.c $(CFLAGS)
	@mv max_seek_sequence.o bin/

bin/random_sequence.o : sequence_algorithm.h random_sequence.c
	@printf "Building random_sequence.o\n"
	@mkdir -p bin
	gcc -c random_sequence.c $(CFLAGS)
	@mv random_sequence.o bin/

bin/alternating_sequence.o : sequence_algorithm.h alternating_sequence.c
	@printf "Building alternating_sequence.o\n"
	@mkdir -p bin
	gcc -c alternating_sequence.c $(CFLAGS)
	@mv alternating_sequence.o bin/

bin/time_standard : config.h bin/random_sequence.o bin/time.o
	@printf "Building time_standard\n"
	@mkdir -p bin
	gcc -o bin/time_standard bin/time.o bin/random_sequence.o $(CFLAGS) 

bin/time_alternating : config.h bin/alternating_sequence.o bin/time.o
	@printf "Building time_alternating\n"
	@mkdir -p bin
	gcc -o bin/time_alternating bin/time.o bin/alternating_sequence.o $(CFLAGS) 

bin/time_max_seek : config.h bin/max_seek_sequence.o bin/time.o
	@printf "Building time_max_seek\n"
	@mkdir -p bin
	gcc -o bin/time_max_seek bin/time.o bin/max_seek_sequence.o $(CFLAGS) 
