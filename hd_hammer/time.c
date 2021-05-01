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
#include <pthread.h>
#include <sched.h>

#include "util.h"
#include "run.h"
#include "config.h"

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


int main(int argc, char **argv, char **arge) {
  struct my_args args;
  args.filename = NULL;
  args.log_filename = NULL;

  int allocate = 0;
  int run_flag = 0;
  char c;
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
	    args.filename = raw_filenames[atoi(optarg)];
	    args.file_max = fs[atoi(optarg)];
	} else {
	    args.filename = filenames[atoi(optarg)];
	    args.file_max = FILE_SIZE;
	}
	break;
      case 'l':
	args.log_filename = optarg;
      case '?':
	if (optopt == 'f'){
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
	}
      }
  }
  if(args.filename == NULL){
    fprintf (stderr, "No filename specified.\n");
    exit(1);
  } else if(run_flag && args.log_filename == NULL){
    fprintf (stderr, "No log filename specified.\n");
    exit(1);
  } else {
    fprintf (stderr, "Using file '%s' with log '%s'\n", args.filename, args.log_filename);
  }

  struct sched_param param;
  pthread_attr_t attr;
  pthread_t thread;
  int ret;

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

 
  if(run_flag){
        if(geteuid() != 0)
        {
                fprintf(stderr, "you must be root to spawn a RT pthread\n");
                exit(1);
        }
  }

  if(run_flag) {
    args.allocate = allocate;
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
      open_fd(allocate, args.filename);
  } else {
    return 1;
  }

out:
  return 0;
}
