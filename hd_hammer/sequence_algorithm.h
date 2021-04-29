// Return the next position for a random sequence. Return less than 0
// to indicate the end of a sequence.
long int nextPos();

// initialize sequence. file_end is the last valid index to return by nextPos
void init_sequence(long int file_end);

// The name of the sequence algorithm
char* sequence_name();
