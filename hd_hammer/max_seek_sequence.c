#include <stdlib.h>

#include "sequence_algorithm.h"

long int file_end_index;
size_t size = 14;
long int sequence[] = {0, 427710, 0, 427711, 0, 427712, 0, 427713, 0, 427714, 0, 427715, 0, 427716};

size_t i = -1;
long int nextPos(){
	i++;
	i %= size;
	return sequence[i];
}

void init_sequence(long int file_end){
	file_end_index = file_end;
}

char* sequence_name(){
	return "max_seek";
}

