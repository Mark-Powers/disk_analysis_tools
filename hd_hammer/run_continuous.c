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

#define WINDOW_SIZE 100
#define SIZE_MULTIPLIER 50

unsigned long long times[WINDOW_SIZE];

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

  // Open files, print out config
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
  if (SEEK_TYPE == CUSTOM_SEEK ) {
    // Get the index of the last valid random position in the file
    file_end = args->file_max - DISK_BUF_BYTES;
    init_sequence(file_end);
  }


  fprintf(stderr, "warming up...\n");
  // warmup reads 1 window of data
  for(int i = 0; i < WINDOW_SIZE; i++){
	if (SEEK_TYPE == CUSTOM_SEEK) {
	      fseek(file, nextPos(), SEEK_SET);
	}	
	task(file);
  }

  fprintf(stderr, "getting control data...\n");
  // gather control data with WINDOW_SIZE*SIZE_MULTIPLIER size data
  long long baseline[WINDOW_SIZE*SIZE_MULTIPLIER];
  for(int i = 0; i < WINDOW_SIZE*SIZE_MULTIPLIER; i++){
	if (SEEK_TYPE == CUSTOM_SEEK) {
	      fseek(file, nextPos(), SEEK_SET);
	}	
	baseline[i] = timedTask(file);	
  }
  qsort(baseline, WINDOW_SIZE*SIZE_MULTIPLIER, sizeof(long long), compare);
  long long thresholds[5];
  fprintf(stderr, "thresholds set to \n");
  for(int j = 0 ; j < 5; j++){
    thresholds[j] = baseline[(int)(WINDOW_SIZE*SIZE_MULTIPLIER*(j*0.01 + 0.95))];
    fprintf(stderr, "\t%f\t%lld\n", (j*0.01 + 0.95), thresholds[j]);
  }

  // COPY INITIAL DATA
  int index = 0;
  int count_above_thresholds[5];
  for(int j = 0 ; j < 5; j++){
	  count_above_thresholds[j] = 0;
	  for(int i = 0; i < WINDOW_SIZE; i++){
		times[i] = baseline[i];
		if(times[i] > thresholds[j]){
		    count_above_thresholds[j]++;
		}
	  }
  }

  struct timespec ts;

  // GATHER NEW DATA
  for(;;){
	if (SEEK_TYPE == CUSTOM_SEEK) {
	      fseek(file, nextPos(), SEEK_SET);
	}	
	long long new_time = timedTask(file);	

	long long old_time = times[index];
	times[index] = new_time;
	index++;
	index %= WINDOW_SIZE;

	clock_gettime(CLOCK_REALTIME, &ts);
	fprintf(log_fp, "%f", ts.tv_sec + 1e-9*ts.tv_nsec);
	// If moving out an outlier, uncount it
	for(int j = 0 ; j < 5; j++){
		if(old_time > thresholds[j]){
			count_above_thresholds[j]--;
		}
		// If moving in an outlier, count it
		if(new_time > thresholds[j]){
			count_above_thresholds[j]++;
		}
		fprintf(log_fp, ",%d", count_above_thresholds[j]);
	}
	fprintf(log_fp, "\n");
	// Flush is needed since the process is ended with a SIGTERM
	fflush(log_fp);
  }

  fclose(file);
  return 0;
}


