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

#define DISK_BUF_BYTES (1028 * 16)
#define FILE_SIZE 1073742000 / 32 // 1 GB
#define DEPTH 1
#define SECONDS 30
#define WARMUP_SECONDS 10
#define TOTAL_SECONDS (SECONDS + WARMUP_SECONDS)
#define CLEAR_INTERRUPT_FLAG 0
#define RANDOM_SEEK 1

// char *file = "testfile";
char *filename = "/media/mark/Backup/dummy_data/testfile";
unsigned char *buf;
int fd;
FILE *file;

void open_fd() {
  fprintf(stderr, "Opening file\n");
  fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0) {
    fprintf(stderr, "Error opening file: open\n");
    exit(1);
  }
  file = fdopen(fd, "wb");
  if (file == NULL) {
    fprintf(stderr, "Error opening file: fdopen\n");
    exit(1);
  }
  fprintf(stderr, "Allocating...\n");
  // fallocate(fd, 0, 0, FILE_SIZE);
  posix_fallocate(fd, 0, FILE_SIZE);
}

int main(int argc, char **argv, char **arge) {
  srand(123);

  open_fd();
  // Get the index of the last valid random position in the file
  fseek(file, 0, SEEK_END);
  long int file_end = ftell(file) - DISK_BUF_BYTES;
  fprintf(stderr, "Last index of file is %ld\n", file_end);
  fseek(file, 0, SEEK_SET);

  struct timespec tps, tpe, tp_init, tp_init_real;
  clock_gettime(CLOCK_REALTIME, &tp_init_real);
  clock_gettime(CLOCK_MONOTONIC_RAW, &tp_init);
  // We use clock_monotonic_raw to ensure our data is consistent while gathering
  // but when we write the log, we write the unix timestamp from when the
  // program started
  double real_ns_offset = (tp_init_real.tv_sec + 1e-9 * tp_init_real.tv_nsec) -
                          (tp_init.tv_sec + 1e-9 * tp_init.tv_nsec);

  unsigned int i;
  int sum_index = 0;
  unsigned int sum = 0;
  int bytes[DISK_BUF_BYTES];
  int elapased_seconds;
  if (CLEAR_INTERRUPT_FLAG) {
    asm("cli");
  }
  fprintf(stderr, "Starting main loop\n");
  while (1) {
    for (int i = 0; i < DISK_BUF_BYTES; i++) {
      bytes[i] = rand();
    }
    if (RANDOM_SEEK) {
      long int pos = rand() % file_end;
      fseek(file, pos, SEEK_SET);
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &tps);
    fwrite(bytes, DISK_BUF_BYTES, sizeof(int), file);
    sync();
    clock_gettime(CLOCK_MONOTONIC_RAW, &tpe);
    i = tpe.tv_nsec + tpe.tv_sec * 1e9 - tps.tv_nsec - tps.tv_sec * 1e9;

    elapased_seconds = tpe.tv_sec - tp_init.tv_sec;
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
