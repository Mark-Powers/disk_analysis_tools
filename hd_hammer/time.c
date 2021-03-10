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

#define DISK_BUF_BYTES (1028*2)
#define FILE_SIZE 1073742000 // 1 GB
#define DEPTH 1
#define SECONDS 1

// char *file = "testfile";
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
  srand(time(NULL));
  struct timespec tps, tpe, tp_init;
  // buf = prepare_timing_buf(DISK_BUF_BYTES);
  unsigned int i;
  int sum_index = 0;
  unsigned int sum = 0;
  clock_gettime(CLOCK_REALTIME, &tp_init);
  fprintf(stderr, "Starting main loop\n");
  int bytes[DISK_BUF_BYTES];
  open_fd();
  // asm("cli");
  while (1) {
    // Start time
    clock_gettime(CLOCK_REALTIME, &tps);
    // Do write task
    for (int i = 0; i < DISK_BUF_BYTES; i++) {
      bytes[i] = rand();
    }
    fwrite(bytes, DISK_BUF_BYTES, sizeof(int), fd);
    sync();
    // if ((r = write(fd, bytes, DISK_BUF_BYTES*sizeof(int))) != DISK_BUF_BYTES) {
    // End time
    clock_gettime(CLOCK_REALTIME, &tpe);
    i = tpe.tv_nsec - tps.tv_nsec;
    if (1 || i < 1000000000) { // Do not log outlier values
      // Average DEPTH many readings
      sum_index++;
      sum += i;
      if (sum_index == DEPTH) {
        double average = (1e-6 * sum) / DEPTH;
        printf("%f,%f\n", (tpe.tv_sec + 1e-9 * tpe.tv_nsec), average);
        sum_index = 0;
        sum = 0;
      }
    }
    // Exit after SECONDS seconds
    if (tpe.tv_sec - tp_init.tv_sec > 15) {
      break;
    }
  }
  // asm("sti");
  fclose(fd);
  return 0;
}
