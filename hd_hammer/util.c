#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "util.h"
#include "config.h"

int bytes[DISK_BUF_BYTES] __attribute__ ((__aligned__ (4*KB)));

// The actual task of reading or writing to the disk
int task(FILE* file) {
  if (WRITE) {
    int ret = fwrite(bytes, DISK_BUF_BYTES, sizeof(int), file) == 0 && is_eof(file);
    int fd = fileno(file);
    sync();
    fsync(fd);
    fdatasync(fd);
    sync();
    sync();
    return ret;
  } else {
    return fread(bytes, DISK_BUF_BYTES, sizeof(int), file) == 0 && is_eof(file);
  }
}

void fillBytes() {
  for (int i = 0; i < DISK_BUF_BYTES; i++) {
    bytes[i] = rand();
  }
}

FILE* open_fd(int allocate, char* filename) {
  int fd;
  FILE *file;
  if (WRITE) {
    if(DIRECT){
      if(RAW){
	fd = open(filename, O_WRONLY | O_DIRECT | O_SYNC);
      } else {
        fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT | O_DIRECT | O_SYNC, 644);
      }
    } else {
      if(RAW){
	fd = open(filename, O_WRONLY | O_SYNC);
      } else {
        fd = open(filename, O_WRONLY | O_TRUNC | O_SYNC, 644);
      }
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
  return file;
}

int is_eof(FILE* file) {
  if (feof(file)) {
    return 1;
  } else {
    fprintf(stderr, "Error writing to file\n");
    exit(1);
  }
  return 0;
}
