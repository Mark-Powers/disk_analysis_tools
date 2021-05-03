#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <x86intrin.h>
#include <sys/io.h> 
#include <unistd.h>

#include "util.h"
#include "config.h"
#include "run.h"
#include "sequence_algorithm.h"

unsigned long int random_pos[LOG_SIZE];
unsigned long long timer_start[LOG_SIZE];
unsigned long long timer_end[LOG_SIZE];
long times_nsec[LOG_SIZE];
time_t times_sec[LOG_SIZE];
size_t log_index = 0;

FILE *file, *log_fp;

void *run(void *arguments) {
  struct my_args *args = arguments;

  srand(123);

  file = open_fd(args->allocate, args->filename);
  log_fp = fopen(args->log_filename, "w+");
  fprintf(log_fp, "#BUF_SIZE: %d\n"
		  "#FILE_SIZE %ld\n"
		  "#WARMUP %d\n"
		  "#SECONDS %d\n"
		  "#SEEK_TYPE %d\n"
		  "#WRITE %d\n"
		  "#DIRECT %d\n"
		  "#FILENAME %s\n"
		  "#SEQUENCE %s\n",
		  DISK_BUF_BYTES, FILE_SIZE, WARMUP_SECONDS, 
		  SECONDS, SEEK_TYPE, WRITE, DIRECT,
		  args->filename, sequence_name());

  long int file_end = 0;
  if (SEEK_TYPE == RANDOM_SEEK ) {
    // Get the index of the last valid random position in the file
    file_end = args->file_max - DISK_BUF_BYTES;
    init_sequence(file_end);
  }

  // We use clock_monotonic_raw to ensure our data is consistent while gathering
  // but when we write the log, we write the unix timestamp from when the
  // program started
  struct timespec tps, tpe, tp_init, tp_init_real;
  clock_gettime(CLOCK_REALTIME, &tp_init_real);
  clock_gettime(CLOCK_MONOTONIC_RAW, &tp_init);
  // Initialize for compiler sake
  clock_gettime(CLOCK_MONOTONIC_RAW, &tps);
  clock_gettime(CLOCK_MONOTONIC_RAW, &tpe);

  double real_ns_offset = -(tp_init.tv_sec + 1e-9 * tp_init.tv_nsec);

  unsigned long long elapased_seconds = 0;

  fprintf(log_fp, "#REAL START TIME: %f\n", tp_init.tv_sec + 1e-9 * tp_init.tv_nsec);
  while (1) {

    //fillBytes();
    if (SEEK_TYPE == RANDOM_SEEK) {
      long int pos = nextPos();
      random_pos[log_index] = pos;
      fseek(file, pos, SEEK_SET);
    } else if(SEEK_TYPE == ZERO_SEEK){
      fseek(file, 0, SEEK_SET);
    }
    // else, sequential, and so leave file position as is

    timer_start[log_index] = __rdtsc();
    task(file);
    timer_end[log_index] = __rdtsc();


    if(LOG_TIME){
	clock_gettime(CLOCK_MONOTONIC_RAW, &tpe);
        elapased_seconds = tpe.tv_sec - tp_init.tv_sec;
        if(elapased_seconds > WARMUP_SECONDS){
	   times_nsec[log_index] = tpe.tv_nsec;
	    times_sec[log_index] = tpe.tv_sec;
	    log_index++;
	    if (elapased_seconds > TOTAL_SECONDS || log_index >= LOG_SIZE) {
		break;
	    }
	}
    } else {
	if(log_index < LOG_SIZE){
	    log_index++;
	} else {
	    break;
	}
	fprintf(stderr, "Elapsed: %lld       \r", elapased_seconds - WARMUP_SECONDS);
    }
  }
  for(int i = 0; i < log_index; i++){
    if(LOG_TIME){
	fprintf(log_fp, "%f,%lld,%ld\n", times_sec[i] + 1e-9*times_nsec[i] + real_ns_offset, timer_end[i]-timer_start[i], random_pos[i]);
    } else {
	// Skip warmup entries
	if(i < WARMUP_SECONDS*LOGS_PER_SECOND_ALLOCATE){
	    continue;
	}
	fprintf(log_fp, "%d,%lld\n", i, timer_end[i]-timer_start[i]);
    }
  }

  fclose(file);
  return 0;
}


