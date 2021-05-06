CFLAGS= -Wall -O3 -march=native -pthread -lphidget22
PRE := $(mkdir -p bin)

all: bin/time_standard bin/time_alternating bin/accel bin/time_big_jump bin/time_continuous_random bin/time_continuous_jump

clean: 
	rm bin/*.o 
	rm -f bin/time_standard bin/time_alternating bin/accel bin/time_big_jump

bin/accel: accel.c
	@printf "Building accel\n\t"
	gcc -o bin/accel accel.c $(CFLAGS) 

bin/util.o : util.h util.c
	@printf "Building util.o\n\t"
	gcc -c util.c $(CFLAGS)
	@mv util.o bin/

bin/run_log.o : run.h config.h util.h sequence_algorithm.h run_log.c 
	@printf "Building run_log.o\n\t"
	gcc -c run_log.c $(CFLAGS)
	@mv run_log.o bin/

bin/run_continuous.o : run.h config.h util.h sequence_algorithm.h run_continuous.c 
	@printf "Building run_continuous.o\n\t"
	gcc -c run_continuous.c $(CFLAGS)
	@mv run_continuous.o bin/


bin/time.o: config.h run.h util.h time.c
	@printf "Building time.o\n\t"
	gcc -c time.c $(CFLAGS) 
	@mv time.o bin/

bin/big_jump_sequence.o : sequence_algorithm.h big_jump_sequence.c
	@printf "Building big_jump_sequence.o\n\t"
	gcc -c big_jump_sequence.c $(CFLAGS)
	@mv big_jump_sequence.o bin/

bin/random_sequence.o : sequence_algorithm.h random_sequence.c
	@printf "Building random_sequence.o\n\t"
	gcc -c random_sequence.c $(CFLAGS)
	@mv random_sequence.o bin/

bin/alternating_sequence.o : sequence_algorithm.h alternating_sequence.c
	@printf "Building alternating_sequence.o\n\t"
	gcc -c alternating_sequence.c $(CFLAGS)
	@mv alternating_sequence.o bin/

bin/time_standard : bin/random_sequence.o bin/time.o bin/run_log.o bin/util.o
	@printf "Building time_standard\n\t"
	gcc -o bin/time_standard bin/time.o bin/random_sequence.o bin/run_log.o bin/util.o $(CFLAGS) 

bin/time_alternating : bin/alternating_sequence.o bin/time.o bin/run_log.o bin/util.o
	@printf "Building time_alternating\n\t"
	gcc -o bin/time_alternating bin/time.o bin/alternating_sequence.o bin/run_log.o bin/util.o $(CFLAGS) 


bin/time_big_jump : bin/big_jump_sequence.o bin/time.o bin/run_log.o bin/util.o
	@printf "Building big_jump_sequence\n\t"
	gcc -o bin/time_big_jump bin/time.o bin/big_jump_sequence.o bin/run_log.o bin/util.o $(CFLAGS) 

bin/time_continuous_jump : bin/big_jump_sequence.o bin/time.o bin/run_continuous.o bin/util.o
	@printf "Building time_continuous\n\t"
	gcc -o bin/time_continuous_jump bin/time.o bin/big_jump_sequence.o bin/run_continuous.o bin/util.o $(CFLAGS) 

bin/time_continuous_random : bin/random_sequence.o bin/time.o bin/run_continuous.o bin/util.o
	@printf "Building time_continuous_random\n\t"
	gcc -o bin/time_continuous_random bin/time.o bin/random_sequence.o bin/run_continuous.o bin/util.o $(CFLAGS) 

