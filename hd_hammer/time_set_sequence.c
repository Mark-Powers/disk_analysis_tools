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
#include <pthread.h>
#include <sched.h>
#include <phidget22.h>

#include "config.h"

char *filename;
char* filenames[] = {
	"testfile",
	"/media/markp/sdb/testfile",
	"/media/markp/sdc/testfile",
	"/media/markp/sdd/testfile",
	"/media/markp/Backup/testfile"
};
char* raw_filenames[] = {
	"testfile",
	"/dev/sdb", // WD blue
	"/dev/sdc", // WD red (SMR)
	"/dev/sdd", // WD Gold 1 TB
	"/dev/sde", // WD Gold 10 TB
	"/dev/sdf"  // Seagate Barracuda
};
long fs[] = {
    100204886016l, //~100 GB
    1000204886016l, // 1 TB
    300614658048l,   // 3 TB
    1000204886016l, // 1 TB
    10000831348736l, // 10 TB
    300614658048l    // 3 TB
};



char *log_filename;

unsigned long int random_pos;
unsigned long long timer_start;
unsigned long long timer_end;
long times_nsec;
time_t times_sec;
size_t log_index = 0;
double values[3]; // Used for accelerometer/gyroscope
double accel_x;
double accel_y;
double accel_z;
double gyro_x;
double gyro_y;
double gyro_z;

int fd, log_fd;
FILE *file, *log_fp;
int bytes[DISK_BUF_BYTES] __attribute__ ((__aligned__ (4*KB)));
unsigned int sum_index = 0;
unsigned long file_max;

struct my_args {
    int allocate;
} ;


void fillBytes() {
  for (int i = 0; i < DISK_BUF_BYTES; i++) {
    bytes[i] = rand();
  }
}

long int file_end;

long int lastPos = 1000000;
long int size = 1;
int order = 0;
long int nextPos(){
    long int toRet;
    if(order == 0){
	order++;
	return lastPos;
    } else if(order == 1){
	order++;
	return lastPos+size;
    } else{
	order = 1;
	toRet = lastPos;
	size++;
	if(lastPos + size > file_end){
	    order = 0;
	    size = 0;
	    toRet = lastPos;
	    lastPos++;
	} 
    }
    if(lastPos >= file_end){
	return -1;
    }
    return toRet;
}

void open_fd(int allocate) {
  fprintf(stderr, "Opening file\n");
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
}

int is_eof() {
  if (feof(file)) {
    return 1;
  } else {
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
    sync();
    return ret;
  } else {
    return fread(bytes, DISK_BUF_BYTES, sizeof(int), file) == 0 && is_eof();
  }
}

void *run(void *arguments) {
  PhidgetReturnCode res;
  PhidgetAccelerometerHandle accelerometer;
  PhidgetAccelerometer_create(&accelerometer);
  res = Phidget_openWaitForAttachment((PhidgetHandle)accelerometer, PHIDGET_TIMEOUT_DEFAULT);
  if (res != EPHIDGET_OK){
	fprintf(stderr, "could not open accelerometer");
	exit(1);
  }
  res = PhidgetAccelerometer_setDataInterval(accelerometer, 4); // Minimum is 4 ms (hardware minimum)

  PhidgetGyroscopeHandle gyroscope;
  PhidgetGyroscope_create(&gyroscope);
  res = Phidget_openWaitForAttachment((PhidgetHandle)gyroscope, PHIDGET_TIMEOUT_DEFAULT);
  if (res != EPHIDGET_OK){
	fprintf(stderr, "could not open gyroscope");
	exit(1);
  }
  res = PhidgetGyroscope_setDataInterval(gyroscope, 4);

  struct my_args *args = arguments;

  srand(123);

  open_fd(args->allocate);

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

  fprintf(stderr, "Starting main loop\n");
  log_fp = fopen(log_filename, "w+");
  unsigned long long init_ts = __rdtsc();
  while (1) {
    
    fillBytes();
    long int pos = nextPos();
    if(pos < 0){
	    break;
    }
    random_pos = pos;
    fseek(file, pos, SEEK_SET);
    // else, sequential, and so leave file position as is

    timer_start = __rdtsc();
    task();
    timer_end = __rdtsc();

    PhidgetGyroscope_getAngularRate(gyroscope, &values);
    gyro_x = values[0];
    gyro_y = values[1];
    gyro_z = values[2];
    PhidgetAccelerometer_getAcceleration(accelerometer, &values);
    accel_x = values[0];
    accel_y = values[1];
    accel_z = values[2];

    if(LOG_TIME){
	clock_gettime(CLOCK_MONOTONIC_RAW, &tpe);
        elapased_seconds = tpe.tv_sec - tp_init.tv_sec;
        if(elapased_seconds > WARMUP_SECONDS){
	   times_nsec = tpe.tv_nsec;
	    times_sec = tpe.tv_sec;
	    log_index++;
	}
    }
    fprintf(log_fp, "%f,%lld,%ld,%f,%f,%f,%f,%f,%f\n", times_sec + 1e-9*times_nsec + real_ns_offset, timer_end-timer_start, random_pos, accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z);
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
  log_filename = NULL;
  while ((c = getopt (argc, argv, "arf:l:")) != -1){
    switch (c)
      {
      case 'a':
        allocate = 1;
        break;
      case 'r':
        run_flag = 1;
        break;
      case 'f':
	if(RAW){
	    filename = raw_filenames[atoi(optarg)];
	    file_max = fs[atoi(optarg)];
	    file_end = file_max - DISK_BUF_BYTES;
	} else {
	    filename = filenames[atoi(optarg)];
	    file_max = FILE_SIZE;
	}
	break;
      case 'l':
	log_filename = optarg;
      case '?':
	if (optopt == 'f'){
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
	}
      }
  }
  if(filename == NULL){
    fprintf (stderr, "No filename specified.\n");
    exit(1);
  } else if(run_flag && log_filename == NULL){
    fprintf (stderr, "No log filename specified.\n");
    exit(1);
  } else {
    fprintf (stderr, "Using file '%s' with log '%s'\n", filename, log_filename);
  }

  fprintf(stderr, "BUF_SIZE: %d\nFILE_SIZE %ld\nWARMUP %d\nSECONDS %d\nRANDOM %d\nWRITE %d\nDIRECT %d\nFILENAME %s\nALLOCATE %d\nRUN %d\n", 
		  DISK_BUF_BYTES, FILE_SIZE, WARMUP_SECONDS, 
		  SECONDS, RANDOM_SEEK, WRITE, DIRECT,
		  filename, allocate, run_flag);


  struct sched_param param;
  pthread_attr_t attr;
  pthread_t thread;
  int ret;
  struct my_args args;

  /* Lock memory */
  if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
  	fprintf(stderr, "mlockall failed: %m\n");
  	exit(-2);
  }

  ret = pthread_attr_init(&attr);
  if (ret) {
  	fprintf(stderr, "init pthread attributes failed\n");
  	goto out;
  }
  if(REALTIME){
	ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (ret) {
		fprintf(stderr, "pthread setschedpolicy failed\n");
		goto out;
	}  
	param.sched_priority = 99;
	ret = pthread_attr_setschedparam(&attr, &param);
	if (ret) {
		fprintf(stderr, "pthread setschedparam failed\n");
		goto out;
	}
	ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (ret) {
		fprintf(stderr, "pthread setinheritsched failed\n");
		goto out;
	}
  }

  
  if(allocate && run_flag) {
    args.allocate = 1;
    ret = pthread_create(&thread, &attr, &run, (void*)&args);
    if (ret) {
    	fprintf(stderr, "create pthread failed\n");
    	goto out;
    }
    /* Join the thread and wait until it is done */
    ret = pthread_join(thread, NULL);
    if (ret)
  	fprintf(stderr, "join pthread failed: %m\n");
  } else if(run_flag) {
    args.allocate = 0;
    ret = pthread_create(&thread, &attr, &run, (void*)&args);
    if (ret) {
    	fprintf(stderr, "create pthread failed\n");
    	goto out;
    }
    /* Join the thread and wait until it is done */
    ret = pthread_join(thread, NULL);
    if (ret)
  	fprintf(stderr, "join pthread failed: %m\n");
  } else if(allocate){
      open_fd(1);
  } else {
    return 1;
  }

out:
  return 0;
}
