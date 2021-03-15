#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define GB 1024 * 1024 * 1024
#define MB 1024 * 1024
#define KB 1024

#define DISK_BUF_BYTES 16 * KB
#define FILE_SIZE 64 * MB
#define MEMORY_SIZE 1 * GB
#define DEPTH 1
#define SECONDS 30
#define WARMUP_SECONDS 10
#define TOTAL_SECONDS (SECONDS + WARMUP_SECONDS)
#define CLEAR_INTERRUPT_FLAG 0
#define RANDOM_SEEK 1
#define WRITE 1
#define DIRECT 0

// TODO print out parameters at start to stderr?

char *filename = "/media/mark/Backup/dummy_data/testfile";
unsigned char *buf;
int fd;
FILE *file;
int bytes[DISK_BUF_BYTES];

void fillBytes() {
  for (int i = 0; i < DISK_BUF_BYTES; i++) {
    bytes[i] = rand();
  }
}

void open_fd(int allocate) {
  fprintf(stderr, "Opening file\n");
  if (WRITE) {
    if(DIRECT){
      fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT | O_SYNC, 0644);
    } else {
      fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
    if (fd < 0) {
      fprintf(stderr, "Error opening file: open\n");
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
    fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
      fprintf(stderr, "Error opening file: open\n");
      exit(1);
    }
    file = fdopen(fd, "wb+");
    if (file == NULL) {
      fprintf(stderr, "Error opening file: fdopen\n");
      exit(1);
    }
    if(allocate){
      fprintf(stderr, "Writing random data to file...\n");
      for (long index = 0; index < MEMORY_SIZE; index += DISK_BUF_BYTES) {
        fwrite(bytes, DISK_BUF_BYTES, sizeof(int), file);
      }
      fseek(file, 0, SEEK_SET);
    }
  }
}

void measure_time(unsigned int sum_index, unsigned int sum,
                  unsigned int elapased_seconds, double real_ns_offset,
                  struct timespec tpe, struct timespec tps,
                  struct timespec tp_init) {
  unsigned int i =
      tpe.tv_nsec + tpe.tv_sec * 1e9 - tps.tv_nsec - tps.tv_sec * 1e9;
  fprintf(stderr, "Elapsed: %d       \r", elapased_seconds - WARMUP_SECONDS);
  if (elapased_seconds > WARMUP_SECONDS) {
    sum_index++;
    sum += i;
    if (sum_index == DEPTH) {
      double average = (1e-6 * sum) / DEPTH;
      printf("%f,%f\n", (tpe.tv_sec + 1e-9 * tpe.tv_nsec) + real_ns_offset,
             average);
      sum_index = 0;
      sum = 0;
    }
  }
}

int is_eof() {
  if (feof(file)) {
    return 1;
  } else {
    fprintf(stderr, "Error writing or writing to file\n");
    exit(1);
  }
  return 0;
}

int task() {
  if (WRITE) {
    int ret = fwrite(bytes, DISK_BUF_BYTES, sizeof(int), file) == 0 && is_eof();
    sync();
    return ret;
  } else {
    return fread(bytes, DISK_BUF_BYTES, sizeof(int), file) == 0 && is_eof();
  }
}

int run(int allocate) {
  srand(123);

  open_fd(allocate);

  long int file_end = 0;
  if (RANDOM_SEEK) {
    // Get the index of the last valid random position in the file
    fseek(file, 0, SEEK_END);
    file_end = ftell(file) - DISK_BUF_BYTES;
    fseek(file, 0, SEEK_SET);
  }

  // We use clock_monotonic_raw to ensure our data is consistent while gathering
  // but when we write the log, we write the unix timestamp from when the
  // program started
  struct timespec tps, tpe, tp_init, tp_init_real;
  clock_gettime(CLOCK_REALTIME, &tp_init_real);
  clock_gettime(CLOCK_MONOTONIC_RAW, &tp_init);
  /*double real_ns_offset = (tp_init_real.tv_sec + 1e-9 * tp_init_real.tv_nsec) -
                          (tp_init.tv_sec + 1e-9 * tp_init.tv_nsec);*/
  double real_ns_offset = -(tp_init.tv_sec + 1e-9 * tp_init.tv_nsec);

  unsigned int sum = 0, sum_index = 0, elapased_seconds = 0;

  if (CLEAR_INTERRUPT_FLAG) {
    asm("cli");
  }
  fprintf(stderr, "Starting main loop\n");
  while (1) {

    fillBytes();
    if (RANDOM_SEEK) {
      long int pos = rand() % file_end;
      fseek(file, pos, SEEK_SET);
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &tps);
    task();
    clock_gettime(CLOCK_MONOTONIC_RAW, &tpe);

    elapased_seconds = tpe.tv_sec - tp_init.tv_sec;
    measure_time(sum_index, sum, elapased_seconds, real_ns_offset, tpe, tps,
                 tp_init);
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
  while ((c = getopt (argc, argv, "ar")) != -1){
    switch (c)
      {
      case 'a':
        allocate = 1;
        break;
      case 'r':
        run_flag = 1;
        break;
      }
  }

  if(allocate && run_flag) {
    run(1);
  } else if(run_flag) {
    run(0);
  } else if(allocate){
    open_fd(1);
  }
}
