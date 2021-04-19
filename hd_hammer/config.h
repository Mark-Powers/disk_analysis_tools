// Constants
#define GB 1024 * 1024 * 1024l
#define MB 1024 * 1024
#define KB 1024
#define ZERO_SEEK 0
#define SEQ_SEEK 1
#define RANDOM_SEEK 2

// # Configuration vars
// How many bytes to write each operation
#define DISK_BUF_BYTES 4 * KB

#define FILE_SIZE 128l * GB
#define MEMORY_SIZE 1 * GB

#define SECONDS 30
#define WARMUP_SECONDS 10

#define SEEK_TYPE RANDOM_SEEK

// Write or read
#define WRITE 1
// Use direct IO
#define DIRECT 0
// Use raw disk (rather than file system)
#define RAW 1
// Use linux RT scheduling
#define REALTIME 1

// Log real times for each read (i.e. the epoch time)
#define LOG_TIME 1

// Estimate of how many log entries we need to allocate 
// (to avoid mallocs during runtime)
#define LOGS_PER_SECOND_ALLOCATE 30000

// Values computed based on other variables
#define TOTAL_SECONDS (SECONDS + WARMUP_SECONDS)
#define LOG_SIZE LOGS_PER_SECOND_ALLOCATE*TOTAL_SECONDS

#define LOG_ACCEL 1
