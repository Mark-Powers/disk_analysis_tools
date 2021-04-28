#include <stdlib.h>

#include "sequence_algorithm.h"

long int file_end_index;

long int nextPos(){
	return rand() & file_end_index;
}

void init_sequence(long int file_end){
	file_end_index = file_end;
	srand(123);
}

