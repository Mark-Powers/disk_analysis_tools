#include "sequence_algorithm.h"

long int file_end_index;

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
	if(lastPos + size > file_end_index){
	    order = 0;
	    size = 0;
	    toRet = lastPos;
	    lastPos++;
	} 
    }
    if(lastPos >= file_end_index){
	return -1;
    }
    return toRet;
}

void init_sequence(long int file_end){
	file_end_index = file_end;
}
