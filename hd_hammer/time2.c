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
#define BLKFLSBUF _IO(0x12, 97) /* flush buffer cache */

#define DISK_BUF_BYTES (1028*8)
#define FILE_SIZE 1073742000 // 1 GB
#define DEPTH 1
#define SECONDS 30
#define WARMUP_SECONDS 10
#define TOTAL_SECONDS (SECONDS + WARMUP_SECONDS)

//char *file = "testfile";
char *file = "/media/mark/Backup/dummy_data/testfile";
unsigned char *buf;
FILE *fd;

void flush_buffer_cache(int fd) {
  sync();
  fsync(fd);     /* flush buffers */
  fdatasync(fd); /* flush buffers */
  sync();
  if (ioctl(fd, BLKFLSBUF, NULL)) /* do it again, big time */
    perror("BLKFLSBUF failed");
  sync();
}

void open_fd() {
  fd = fopen(file, "wb");
  // fd = open(file, O_TRUNC | O_APPEND | O_DIRECT | O_SYNC);
  if (fd == NULL) {
    fprintf(stderr, "Error opening file\n");
    exit(1);
  }
}

int main(int argc, char **argv, char **arge) {
  srand(123);
  struct timespec tps, tpe, tp_init;
  unsigned int i;
  int sum_index = 0;
  unsigned int sum = 0;
  clock_gettime(CLOCK_MONOTONIC_RAW, &tp_init);
  fprintf(stderr, "Starting main loop\n");
  int bytes[DISK_BUF_BYTES];
  open_fd();
  // asm("cli");
  int elapased_seconds;
  while (1) {
    for (int i = 0; i < DISK_BUF_BYTES; i++) {
      bytes[i] = rand();
    }
    fseek(fd, 0, SEEK_SET);
    // Start time
    clock_gettime(CLOCK_MONOTONIC_RAW, &tps);
    fwrite(bytes, DISK_BUF_BYTES, sizeof(int), fd);
    sync();
    // if ((r = write(fd, bytes, DISK_BUF_BYTES*sizeof(int))) != DISK_BUF_BYTES) {
    // End time
    clock_gettime(CLOCK_MONOTONIC_RAW, &tpe);
    i = tpe.tv_nsec + tpe.tv_sec*1e9 - tps.tv_nsec - tps.tv_sec*1e9;

    elapased_seconds = tpe.tv_sec - tp_init.tv_sec;
    if (elapased_seconds > WARMUP_SECONDS) { // Do not log outlier values
      // Average DEPTH many readings
      sum_index++;
      sum += i;
      if (sum_index == DEPTH) {
        double average = (1e-6 * sum) / DEPTH;
        printf("%f,%f\n", (tpe.tv_sec + 1e-9 * tpe.tv_nsec), average);
        sum_index = 0;
        sum = 0;
      }
	fprintf(stderr, "Elapsed: %d\r", elapased_seconds - WARMUP_SECONDS);
    }
    // Exit after SECONDS seconds
    if (elapased_seconds > TOTAL_SECONDS) {
      break;
    }
  }
  // asm("sti");
  fclose(fd);
  return 0;
}
