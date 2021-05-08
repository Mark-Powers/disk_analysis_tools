#include "sequence_algorithm.h"

long int file_end_index;

long int pos = 1000000000l;
long int jump = 407453;
//long int pos = 0l;
//long int jump = 589824;
long int nextPos(){
    pos += jump;
    pos %= file_end_index;
    return pos;
}

void init_sequence(long int file_end){
	file_end_index = file_end;
}


char* sequence_name(){
	return "big_jump_sequence";
}
