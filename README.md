# Page-Replacement-Memory-Simulator
This program simulates page replacement algorithms. The page replacement algorithms included are clock, FIFO, LRU, random, and optimal. This program outputs the number of reads, the number of writes, and the percentage of hits so that the algorithms can all be compared. 

To use this program, compile it by using the makefile. Type make to compile it. Make sure that the traces files are in the same location as the memory simulator program. 

Once all of that is done, input ./memorysim "tracefile" "number of frames" "clk | lru | fifo | rndm | opt" "debug | quiet"

Below is an example of an input.

./memorysim test.trace 3 opt debug

The trace file is a recording of a running program. test.trace, test2.trace, and test3.trace are not recordings from a program. These were created for easy testing. Each line in the trace file is a hexadecimal memory address followed by R (read) or W (write).

The number of frames is the number of page frames.

Debug mode will output information to the screen of what is happening in the algorithm. It will tell you if a read or write is happening, the entries in cache that are being filled, if the cache is full, and if a page was found in cache. It will also show you how the cache looks like after each read from the trace file. If there is a -1 in the cache display, that means that spot has not been filled yet. I only recommend using debug mode on one of the test traces as they are not long like the other trace files.

clk = clock

lru = least recently used

fifo = first in first out

rndm = random

opt = optimal

To clean the files created by the makefile, type in "make clean".
