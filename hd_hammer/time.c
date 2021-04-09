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


//#pragma intrinsic(__rdtsc)

#define GB 1024 * 1024 * 1024l
#define MB 1024 * 1024
#define KB 1024

#define DISK_BUF_BYTES 4 * KB
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
#define CPU_CYCLE_TIME 1
#define STACK_ALLOCATED 1
#define RAW 1
#define REALTIME 1
#define LOGS_PER_SECOND_ALLOCATE 500

// TODO print out parameters at start to stderr?

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
	"/dev/sdb",
	"/dev/sdc",
	"/dev/sdd"
};
long fs[] = {
    100204886016l,
    1000204886016l,
    1000204886016l,
    10000831348736l
};



char *log_filename;

unsigned long long latency[LOGS_PER_SECOND_ALLOCATE*SECONDS];
double times[LOGS_PER_SECOND_ALLOCATE*SECONDS];
size_t log_index = 0;

int fd, log_fd;
FILE *file, *log_fp;
int bytes[DISK_BUF_BYTES] __attribute__ ((__aligned__ (4*KB)));
//int *bytes;
//
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
  struct my_args *args = arguments;

  srand(123);

  open_fd(args->allocate);

  long int file_end = 0;
  if (RANDOM_SEEK) {
    // Get the index of the last valid random position in the file
    file_end = file_max - DISK_BUF_BYTES;
    fprintf(stderr, "last position in file is %ld\n", file_end);
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

  unsigned long long ts, te, elapased_seconds = 0;

  if (CLEAR_INTERRUPT_FLAG) {
    iopl(3);
    asm("cli");
  }
  fprintf(stderr, "Starting main loop\n");
  log_fp = fopen(log_filename, "w+");
  while (1) {

    fillBytes();
    if (RANDOM_SEEK) {
      long int pos = rand() % file_end;
      fseek(file, pos, SEEK_SET);
      //fseek(file, 0, SEEK_SET);
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

    if(elapased_seconds > WARMUP_SECONDS){
	    if(CPU_CYCLE_TIME){
                    latency[log_index] = te-ts;
	    } else {
		    double i = 1e-6*(tpe.tv_nsec + tpe.tv_sec * 1e9 - tps.tv_nsec - tps.tv_sec * 1e9);
		    latency[log_index] = i;
	    }
	    times[log_index] = (tpe.tv_sec + 1e-9*tpe.tv_nsec)+real_ns_offset;
	    log_index++;
    }
    fprintf(stderr, "Elapsed: %lld       \r", elapased_seconds - WARMUP_SECONDS);

    if (elapased_seconds > TOTAL_SECONDS) {
      break;
    }
  }
  if (CLEAR_INTERRUPT_FLAG) {
    asm("sti");
  }
  for(int i = 0; i < log_index; i++){
    fprintf(log_fp, "%f,%lld\n", times[i], latency[i]);
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

  fprintf(stderr, "BUF_SIZE: %d\nFILE_SIZE %ld\nAVG %d\nWARMUP %d\nSECONDS %d\nRANDOM %d\nWRITE %d\nDIRECT %d\nCPU_TIMING %d\nFILENAME %s\nALLOCATE %d\nRUN %d\n", 
		  DISK_BUF_BYTES, FILE_SIZE, DEPTH, WARMUP_SECONDS, 
		  SECONDS, RANDOM_SEEK, WRITE, DIRECT, CPU_CYCLE_TIME,
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
