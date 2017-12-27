# MOESI-simulator


usage: ./problem1 < input.txt > output.txt

This program can be compiled using the provided Makefile by running the command "Make" inside of the provided directory. The executable file takes in an input file as stdin and writes its results to the output file provided as stdout. 

This program simulates the MOESI protocol given an input problem and outputs the state changes in a prespecified format. The output will contain whether the memory access was a hit, whether the memory line was dirty, and which state the target cache and cache line is progressing to and from. 

An important implementation detail is that the dirtiness of a memory line is maintained by an array of dirty bits, with each index corresponding to a seperate cache line. According to the MOESI diagram provided as specification, writeback occurs during a transition from the invalid state to modified. Therefore, in this implementation, such a transition clears the dirty bit for the specified cache line while all subsequent writes to that line will set its value to 1. This field is specified during a probe read, resulting in either a "Hit" or "Hit Dirty" as output.     
