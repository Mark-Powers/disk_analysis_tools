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

#define SIZE 100
#define PERCENTILE 0.97
#define ABOVE_THRESHOLD_LIMIT 5
#define SIZE_MULTIPLIER 20

unsigned long long times[SIZE];

size_t log_index = 0;

FILE *file, *log_fp;

int compare (const void * a, const void * b) {
   long long diff = ( *(long long*)a - *(long long*)b );
   if(diff < 0) return -1;
   if(diff > 0) return 1;
   return 0;
}

void *run(void *arguments) {
  struct my_args *args = arguments;

  srand(123);

  file = open_fd(args->allocate, args->filename);
  log_fp = fopen(args->log_filename, "w+");
  fprintf(stderr, "#BUF_SIZE: %d\n"
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


  fprintf(stderr, "warming up...\n");
  // WARMUP
  for(int i = 0; i < SIZE_MULTIPLIER*SIZE; i++){
	if (SEEK_TYPE == RANDOM_SEEK) {
	      fseek(file, nextPos(), SEEK_SET);
	}	
	task(file);
  }

  fprintf(stderr, "getting control data...\n");
  // GATHER CONTROL DATA
  long long baseline[SIZE*SIZE_MULTIPLIER];
  for(int i = 0; i < SIZE*SIZE_MULTIPLIER; i++){
	if (SEEK_TYPE == RANDOM_SEEK) {
	      fseek(file, nextPos(), SEEK_SET);
	}	
    	long long start = __rdtsc();
	task(file);
	baseline[i] = __rdtsc() - start;	
  }
  qsort(baseline, SIZE*SIZE_MULTIPLIER, sizeof(long long), compare);
  long long threshold = baseline[(int)(SIZE*SIZE_MULTIPLIER*PERCENTILE)];
  fprintf(stderr, "threshold set to %lld\n", threshold);

  // COPY INITIAL DATA
  int index = 0;
  int count_above_thres = 0;
  for(int i = 0; i < SIZE; i++){
	times[i] = baseline[i];
	if(times[i] > threshold){
		count_above_thres++;
	}
  }

  struct timespec ts;

  // GATHER NEW DATA
  for(;;){
	if (SEEK_TYPE == RANDOM_SEEK) {
	      fseek(file, nextPos(), SEEK_SET);
	}	
	long long start = __rdtsc();
	task(file);
	long long new_time = __rdtsc() - start;	

	long long old_time = times[index];
	times[index] = new_time;
	index++;
	index %= SIZE;

	if(old_time > threshold){
		//fprintf(stderr, "old was above!! %lld\n", old_time);
		count_above_thres--;
	}
	if(new_time > threshold){
		//fprintf(stderr, "new is above!! %lld\n", new_time);
		count_above_thres++;
	}

	if(count_above_thres > ABOVE_THRESHOLD_LIMIT){
		clock_gettime(CLOCK_REALTIME, &ts);
		fprintf(stderr, "ABOVE THRESHOLD!! %d with latency %lld at time %f\n", count_above_thres, new_time, ts.tv_sec + 1e-9*ts.tv_nsec);
	} else {
		//fprintf(stderr, "count: %d %lld\n", count_above_thres, new_time);
	}
  }

  fclose(file);
  return 0;
}


