#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

char* filename = "/dev/sdb";

int main(){
    FILE *f = fopen(filename, "w+");
    int fd = fileno(f);
    if(fd < 0){
	fprintf(stderr, "Error opening file (%d)\n", errno);
	exit(1);
    }
    if(write(fd, "test", 4) < 0){
	fprintf(stderr, "Error writing to file\n");
	exit(1);
    }
    fseek(f, 0, SEEK_SET);
    char bytes[10];
    for(int i = 0; i < 10 ; i++){
	bytes[i] = 0;
    }
    if(fread(bytes, sizeof(int), 10, f) <= 0){
	fprintf(stderr, "Error reading from file\n");
	exit(1);
    }
    printf("%s\n", bytes);
}
