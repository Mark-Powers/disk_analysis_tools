#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <x86intrin.h>

#pragma intrinsic(__rdtsc)

#define GB 1024 * 1024 * 1024
#define MB 1024 * 1024
#define KB 1024

#define DISK_BUF_BYTES 1 * KB
#define FILE_SIZE 128l * GB
#define MEMORY_SIZE 1 * GB
#define DEPTH 1
#define SECONDS 30
#define WARMUP_SECONDS 10
#define TOTAL_SECONDS (SECONDS + WARMUP_SECONDS)
#define CLEAR_INTERRUPT_FLAG 0
#define RANDOM_SEEK 1
#define WRITE 1
#define DIRECT 0
#define CPU_CYCLE_TIME 0
#define STACK_ALLOCATED 1

char *filename;
char* filenames[] = {
	"testfile",
	"/dev/sdb",
	"/dev/sdc",
	"/dev/sdd"
};

int fd;
FILE *file;
int bytes[DISK_BUF_BYTES] __attribute__ ((__aligned__ (4*KB)));;
//int *bytes;

unsigned int sum_index = 0;

void fillBytes() {
  for (int i = 0; i < DISK_BUF_BYTES; i++) {
    bytes[i] = rand();
  }
}

void open_fd(int allocate) {
  fprintf(stderr, "Opening file\n");
  if (WRITE) {
    if(DIRECT){
      fd = open(filename, O_WRONLY | O_DIRECT | O_SYNC);
    } else {
      fd = open(filename, O_WRONLY | O_SYNC);
    }
    if (fd < 0) {
      fprintf(stderr, "Error opening file: open (are you root?)\n");
      exit(1);
    }
    file = fdopen(fd, "wb");
    if (file == NULL) {
      fprintf(stderr, "Error opening file: fdopen\n");
      exit(1);
    }
    if(allocate){
      fprintf(stderr, "Allocating...\n");
      posix_fallocate(fd, 0, FILE_SIZE);
    }
  } else {
    if(allocate){
      file = fopen(filename, "w");
      fprintf(stderr, "Writing random data to file...\n");
      for (long index = 0; index < MEMORY_SIZE; index += DISK_BUF_BYTES) {
        fillBytes();
        fwrite(bytes, sizeof(int), DISK_BUF_BYTES, file);
      }
      fclose(file);
    }
    file = fopen(filename, "r");
    fd = fileno(file);
    if (fd < 0) {
      fprintf(stderr, "Error opening file: open\n");
      exit(1);
    }
    file = fdopen(fd, "rb");
    if (file == NULL) {
      fprintf(stderr, "Error opening file: fdopen\n");
      exit(1);
    }
  }
}

void measure_time(unsigned int sum,
                  unsigned int elapased_seconds, double real_ns_offset,
                  struct timespec tpe, struct timespec tps,
                  struct timespec tp_init, unsigned long long x) {
  if(CPU_CYCLE_TIME){
    if (elapased_seconds > WARMUP_SECONDS) {
      sum_index++;
      sum += x;
      if (sum_index == DEPTH) {
        printf("%f,%lld\n", (tpe.tv_sec + 1e-9 * tpe.tv_nsec) + real_ns_offset, x/DEPTH);
        sum_index = 0;
        sum = 0;
      }
    }
  } else {
    double i = 1e-6*(tpe.tv_nsec + tpe.tv_sec * 1e9 - tps.tv_nsec - tps.tv_sec * 1e9);
    if (elapased_seconds > WARMUP_SECONDS) {
      sum_index++;
      sum += i;
      if (sum_index == DEPTH) {
        double average = sum/DEPTH;
        printf("%f,%f\n", (tpe.tv_sec + 1e-9 * tpe.tv_nsec) + real_ns_offset, average);
        sum_index = 0;
        sum = 0;
      }
    }
  }
  fprintf(stderr, "Elapsed: %d       \r", elapased_seconds - WARMUP_SECONDS);
}

int is_eof() {
  if (feof(file)) {
    return 1;
  } else {
    //ferror(file);
    fprintf(stderr, "??? %d - %d\n", errno, EINVAL);
    fprintf(stderr, "Error writing to file\n");
    exit(1);
  }
  return 0;
}

int task() {
  if (WRITE) {
    int ret = fwrite(bytes, DISK_BUF_BYTES, sizeof(int), file) == 0 && is_eof();
    sync();
    fsync(fd);
    fdatasync(fd);
    sync();
    //if (ioctl(fd, DISK_BUF_BYTES, NULL)){
    //	perror("BLKFLSBUF failed");
    //}
    sync();
    return ret;
  } else {
    return fread(bytes, DISK_BUF_BYTES, sizeof(int), file) == 0 && is_eof();
  }
}

int run(int allocate) {
  srand(123);

  open_fd(allocate);

  /*
  if(!STACK_ALLOCATED){
    posix_memalign((void**)&bytes, 4*KB, DISK_BUF_BYTES);
  }
  */

  long int file_end = 0;
  if (RANDOM_SEEK) {
    // Get the index of the last valid random position in the file
    fseek(file, 0, SEEK_END);
    file_end = ftell(file) - DISK_BUF_BYTES;
    fprintf(stderr, "Last pos: %ld\n", file_end);
    fseek(file, 0, SEEK_SET);
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
  
  /*double real_ns_offset = (tp_init_real.tv_sec + 1e-9 * tp_init_real.tv_nsec) -
                          (tp_init.tv_sec + 1e-9 * tp_init.tv_nsec);*/
  double real_ns_offset = -(tp_init.tv_sec + 1e-9 * tp_init.tv_nsec);

  unsigned long long ts, te, elapased_seconds = 0, sum = 0;

  if (CLEAR_INTERRUPT_FLAG) {
    iopl(3);
    asm("cli");
  }
  fprintf(stderr, "Starting main loop\n");
  while (1) {

    fillBytes();
    if (RANDOM_SEEK) {
      long int pos = rand() % file_end;
      fseek(file, pos, SEEK_SET);
    }

    if(CPU_CYCLE_TIME){
      ts = __rdtsc();
    } else {
      clock_gettime(CLOCK_MONOTONIC_RAW, &tps);
    }
    task();
    if(CPU_CYCLE_TIME){
      te = __rdtsc();
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &tpe);
    elapased_seconds = tpe.tv_sec - tp_init.tv_sec;
    measure_time(sum, elapased_seconds, real_ns_offset, tpe, tps,
                 tp_init, te-ts);
    if (elapased_seconds > TOTAL_SECONDS) {
      break;
    }
  }
  if (CLEAR_INTERRUPT_FLAG) {
    asm("sti");
  }
  fclose(file);
  close(fd);
  return 0;
}

int main(int argc, char **argv, char **arge) {
  int allocate = 0;
  int run_flag = 0;
  char c;
  filename = NULL;
  while ((c = getopt (argc, argv, "arf:")) != -1){
    switch (c)
      {
      case 'a':
        allocate = 1;
        break;
      case 'r':
        run_flag = 1;
        break;
      case 'f':
        filename = filenames[atoi(optarg)];
	break;
      case '?':
	if (optopt == 'f'){
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
	}
      }
  }
  if(filename == NULL){
    fprintf (stderr, "No filename specified.\n");
    exit(1);
  } else {
    fprintf (stderr, "Using file '%s'\n", filename);
  }

  fprintf(stderr, "BUF_SIZE: %d\nFILE_SIZE %ld\nAVG %d\nWARMUP %d\nSECONDS %d\nRANDOM %d\nWRITE %d\nDIRECT %d\nCPU_TIMING %d\nFILENAME %s\nALLOCATE %d\nRUN %d\n", 
		  DISK_BUF_BYTES, FILE_SIZE, DEPTH, WARMUP_SECONDS, 
		  SECONDS, RANDOM_SEEK, WRITE, DIRECT, CPU_CYCLE_TIME,
		  filename, allocate, run_flag);

  if(allocate && run_flag) {
    run(1);
  } else if(run_flag) {
    run(0);
  } else if(allocate){
    open_fd(1);
  } else {
    return 1;
  }
  //fclose(file);
  //close(fd);
}
