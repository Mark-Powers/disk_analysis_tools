#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

int task(FILE* file);
void fillBytes();
FILE *open_fd(int allocate, char* filename);
int is_eof(FILE* file);

struct my_args {
    int allocate;
    long int file_max;
    char* filename;
    char* log_filename;
} ;

#endif /* UTIL_H */
