# Project structure:
`time.c`: The main function, which parses arguments and calls the `run` function outlined in `run.h`. This spawns a pthread in order to set scheduling priority.

`run.h`: The function to run. There are two implementations: one that just logs after a specified amount of time, and one that runs continuously, having to be stopped manually by the user.

`util.h`: Defines functions used by multiple `run` implementations.

`sequence_algorithmh.h`: Defines the functions to specify how seeking should be done if the type set in the config is random seek. Implementations include `big_jump_sequence` which increments the position by a large number, `alternating_sequence` which goes back and forth as in 0,1,0,2,0,3,0..., and `random_sequence` which calls `rand()` to determine each position.

`config.h`: The various configuration options for the program.

# Building:
To build, there is a Makefile. All compiled files are placed within the `bin/` directory. 
Different runnable executables are created by linking the various implementations. Current runnale targets are:
- `time_standard` - runs the finite log version with random seek
- `time_alternating` - runs the finite log version with alternating seek
- `time_big_jump` - runs the finite log version with big jump seek
- `time_continuous` - runs the continuous log version with big jump seek

- 


