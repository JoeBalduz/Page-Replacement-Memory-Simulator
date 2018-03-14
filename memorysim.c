#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "memorysim.h"

#define PTSIZE 1048567 //2^32 logical address / 2^12 offset = 2^20 = 1048576 page table entries

/*Assuming that all pages and page frames are 4096 bytes*/

//Clock replacement algorithm
void clk(int frames, char* traceFile, int debug)
{
	unsigned addr;	//Holds the address from the file
	char rw;	//Holds whether it is a read or write from the file

	int i, j;	//Iterators

	unsigned pageNumber;	

	struct tableEntry PT[PTSIZE];		//Creates the page table
	struct tableEntry cache[frames];	//Creates the cache

	int foundCache = -1;
	int changed = 0;	//Used to break out of loops

	//The front and next available slots in the arrays
	//Front of cache is initially -1 to indicate an empty array
	int myFrontCache = -1;	
	int nextAvailCache = 0;

	//Initializations
	PTInit(PT, PTSIZE);
	cacheInit(cache, frames);

	FILE* file;
	file = fopen(traceFile, "r");
	if(file)
	{
		//Will scan in everything in the file
		while(fscanf(file, "%x %c", &addr, &rw) != EOF)
		{
			pageNumber = addr / (16 * 16 * 16);	//Gets the page number. Offset is 12 bits, hence the 3 16s

			//Sets the valid bit
			if(PT[pageNumber].valid == 0)
			{
				PT[pageNumber].vpn = pageNumber;
				PT[pageNumber].valid = 1;
			}
			//Checks to see if the frame has already been loaded in cache
			foundCache = foundInCache(cache, frames, pageNumber, debug);
			//Page is already in cache
			if(foundCache >= 0)
			{
				cache[foundCache].reference = 1;	//Reference bit is set 1 to since it has been referenced
				if(rw == 'W')
				{
					cache[foundCache].dirty = 1;
				}
			}
			//Cache is empty
			else if(myFrontCache == -1)
			{
				if(debug)
				{
					printf("Filled in first entry of cache.\n");
				}
				myFrontCache = 0;
				nextAvailCache = 1;
				cache[myFrontCache].vpn = pageNumber;
				PT[pageNumber].present = 1;
				readCount++;
				if(rw == 'W')
				{
					cache[myFrontCache].dirty = 1;
				}			
			}
			//Cache is full
			else if(myFrontCache == nextAvailCache)
			{
				if(debug)
				{
					printf("Cache is full.\n");
				}
				//Goes through the cache a maximum of two times
				//and sets the reference bit to 0 if a page
				//is passed by the "clock hand"
				for(j = 0; j < 2; j++)
				{
					//Looks for reference bit of 0
					for(i = 0; i < frames; i++)
					{	
						//Reference bit is 0 so it gets replaced
						if(cache[(myFrontCache + i) % frames].reference == 0)
						{
							//Moved to swap space since dirty bit is 1
							if(cache[(myFrontCache + i) % frames].dirty == 1)
							{
								PT[cache[(myFrontCache + i) % frames].vpn].present = 0;
								if(debug)
								{
									printf("Write to disk.\n");
								}
								writeCount++;
							}
							if(debug)
							{
									printf("Read from disk.\n");
							}
							readCount++;
							PT[pageNumber].present = 1;
							cache[(myFrontCache + i) % frames] = PT[pageNumber];
							if(rw == 'W')
							{
								cache[(myFrontCache + i)].dirty = 1;
							}
							else
							{
								cache[(myFrontCache + i) % frames].dirty = 0;
							}
							cache[(myFrontCache + i) % frames].reference = 1;
							myFrontCache = (myFrontCache + i) % frames;
							nextAvailCache = myFrontCache;
							changed = 1;	//Used to break out of the for loops
						}
						else
						{
							cache[(myFrontCache + i) % frames].reference = 0;
						}
						if(changed)
						{
							break;
						}
					}
					if(changed)
					{
						break;
					}
				}
			}
			//The cache has not yet been filled up
			else
			{
				if(debug)
				{
					printf("Slot %d in array is filled.\n", nextAvailCache);
				}
				cache[nextAvailCache].vpn = pageNumber;
				if(debug)
				{
					printf("Read from disk.\n");
				}
				readCount++;
				PT[pageNumber].present = 1;
				if(rw == 'W')
				{
					cache[nextAvailCache].dirty = 1;
				}
				nextAvailCache = (nextAvailCache + 1) % frames;
			}
			traceEvents++;
			changed = 0;
			foundCache = -1;
			if(debug)
			{
				cacheDisplay(cache, frames);
			}
		}
		fclose(file);
	}
	printf("Total memory frames: %d\n", frames);
	printf("Events in trace: %d\n", traceEvents);
	printf("Hits percentage: %f%%\n", ((double)hitCount / traceEvents) * 100);
	printf("Total disk reads: %d\n", readCount);
	printf("Total disk writes: %d\n", writeCount);
}

//First in, first out replacement algorithm
void fifo(int frames, char* traceFile, int debug)
{
	unsigned addr;	//Holds the address from the file
	char rw;		//Holds whether it is a read or write from the file

	unsigned pageNumber;	

	struct tableEntry PT[PTSIZE];		//Creates the page table
	struct tableEntry cache[frames];	//Creates the cache

	int foundCache = -1;

	//The front and next available slots in the arrays
	//Front of cache is initially -1 to indicate an empty array
	int myFrontCache = -1;	
	int nextAvailCache = 0;

	//Initializations
	PTInit(PT, PTSIZE);
	cacheInit(cache, frames);
	
	FILE* file;
	file = fopen(traceFile, "r");
	if(file)
	{
		//Will scan in everything in the file
		while(fscanf(file, "%x %c", &addr, &rw) != EOF)
		{
			pageNumber = addr / (16 * 16 * 16);	//Gets the page number. Offset is 12 bits, hence the 3 16s

			//Sets the valid bit
			if(PT[pageNumber].valid == 0)
			{
				PT[pageNumber].vpn = pageNumber;
				PT[pageNumber].valid = 1;
			}
			//Checks to see if the frame has already been loaded in cache
			foundCache = foundInCache(cache, frames, pageNumber, debug);
			//Page is already in cache
			if(foundCache >= 0)
			{
				if(rw == 'W')
				{
					cache[foundCache].dirty = 1;
				}
			}
			//Cache is empty
			else if(myFrontCache == -1)
			{
				if(debug)
				{
					printf("Filled in first entry of cache.\n");
				}
				myFrontCache = 0;
				nextAvailCache = 1;
				cache[myFrontCache].vpn = pageNumber;
				PT[pageNumber].present = 1;
				readCount++;
				if(rw == 'W')
				{
					cache[myFrontCache].dirty = 1;
				}			
			}
			//Cache is full
			else if(myFrontCache == nextAvailCache)
			{
				if(debug)
				{
					printf("Cache is full.\n");
				}
				if(cache[myFrontCache].dirty == 1)
				{
					PT[cache[myFrontCache].vpn].present = 0;
					if(debug)
					{
						printf("Write to disk.\n");
					}
					writeCount++;
				}
				if(debug)
				{
					printf("Read from disk.\n");
				}
				readCount++;
				PT[pageNumber].present = 1;
				//Front of the cache is taken out since it was the first one
				//to be put into cache. Front of cache is then moved to the right
				//once
				cache[myFrontCache] = PT[pageNumber];
				if(rw == 'W')
				{
					cache[myFrontCache].dirty = 1;
				}
				else
				{
					cache[myFrontCache].dirty = 0;
				}
				myFrontCache = (myFrontCache + 1) % frames;
				nextAvailCache = myFrontCache;
			}
			//The cache has not yet been filled up
			else
			{
				if(debug)
				{
					printf("Slot %d in array is filled.\n", nextAvailCache);
				}
				cache[nextAvailCache].vpn = pageNumber;
				if(debug)
				{
					printf("Read from disk.\n");
				}
				readCount++;
				PT[pageNumber].present = 1;
				if(rw == 'W')
				{
					cache[nextAvailCache].dirty = 1;
				}
				nextAvailCache = (nextAvailCache + 1) % frames;
			}
			traceEvents++;
			foundCache = -1;
			if(debug)
			{
				cacheDisplay(cache, frames);
			}
		}
		fclose(file);
	}
	printf("Total memory frames: %d\n", frames);
	printf("Events in trace: %d\n", traceEvents);
	printf("Hits percentage: %f%%\n", ((double)hitCount / traceEvents) * 100);
	printf("Total disk reads: %d\n", readCount);
	printf("Total disk writes: %d\n", writeCount);
	
}

//Least recently used replacement algorithm
void lru(int frames, char* traceFile, int debug)
{
	unsigned addr;	//Holds the address from the file
	char rw;	//Holds whether it is a read or write from the file

	int i;	//Iterator

	unsigned pageNumber;	

	struct tableEntry PT[PTSIZE];		//Creates the page table
	struct tableEntry cache[frames];	//Creates the cache

	int foundCache = -1;
	int now = 0;	//Used to tell when a page was added
	int leastRecent;	//Least recently used in cache
	int leastRecentCache;	//Number that holds which spot in cache of the least recently used page

	//The front and next available slots in the arrays
	//Front of cache is initially -1 to indicate an empty array
	int myFrontCache = -1;	
	int nextAvailCache = 0;

	//Initializations
	PTInit(PT, PTSIZE);
	cacheInit(cache, frames);
	
	FILE* file;
	file = fopen(traceFile, "r");
	if(file)
	{
		//Will scan in everything in the file
		while(fscanf(file, "%x %c", &addr, &rw) != EOF)
		{
			pageNumber = addr / (16 * 16 * 16);	//Gets the page number. Offset is 12 bits, hence the 3 16s

			//Sets the valid bit
			if(PT[pageNumber].valid == 0)
			{
				PT[pageNumber].vpn = pageNumber;
				PT[pageNumber].valid = 1;
			}
			//Checks to see if the frame has already been loaded in cache
			foundCache = foundInCache(cache, frames, pageNumber, debug);
			//Page is already in cache
			if(foundCache >= 0)
			{
				cache[foundCache].added = now;
				now++;
				if(rw == 'W')
				{
					cache[foundCache].dirty = 1;
				}
			}
			//Cache is empty
			else if(myFrontCache == -1)
			{
				if(debug)
				{
					printf("Filled in first entry of cache.\n");
				}
				myFrontCache = 0;
				nextAvailCache = 1;
				cache[myFrontCache].vpn = pageNumber;
				cache[myFrontCache].added = now;
				now++;
				PT[pageNumber].present = 1;
				readCount++;
				if(rw == 'W')
				{
					cache[myFrontCache].dirty = 1;
				}			
			}
			//Cache is full
			else if(myFrontCache == nextAvailCache)
			{
				if(debug)
				{
					printf("Cache is full.\n");
				}
				leastRecent = cache[0].added;
				leastRecentCache = 0;
				//Goes through the cache and finds the page that was least recently used
				//That page then gets replaced
				for(i = 1; i < frames; i++)
				{
					if(cache[i].added < leastRecent)
					{
						leastRecent = cache[i].added;
						leastRecentCache = i;
					}
				}
				if(cache[leastRecentCache].dirty == 1)
				{
					PT[cache[leastRecentCache].vpn].present = 0;
					if(debug)
					{
						printf("Write to disk.\n");
					}
					writeCount++;
				}
				if(debug)
				{
					printf("Read from disk.\n");
				}
				readCount++;
				PT[pageNumber].present = 1;
				cache[leastRecentCache] = PT[pageNumber];
				cache[leastRecentCache].added = now;
				now++;
				if(rw == 'W')
				{
					cache[leastRecentCache].dirty = 1;
				}
				else
				{
					cache[leastRecentCache].dirty = 0;
				}
			}
			//The cache has not yet been filled up
			else
			{
				if(debug)
				{
					printf("Slot %d in array is filled.\n", nextAvailCache);
				}
				cache[nextAvailCache].vpn = pageNumber;
				cache[nextAvailCache].added = now;
				now++;
				if(debug)
				{
					printf("Read from disk.\n");
				}
				readCount++;
				PT[pageNumber].present = 1;
				if(rw == 'W')
				{
					cache[nextAvailCache].dirty = 1;
				}
				nextAvailCache = (nextAvailCache + 1) % frames;
			}
			traceEvents++;
			foundCache = -1;
			if(debug)
			{
				cacheDisplay(cache, frames);
			}
		}
		fclose(file);
	}
	printf("Total memory frames: %d\n", frames);
	printf("Events in trace: %d\n", traceEvents);
	printf("Hits percentage: %f%%\n", ((double)hitCount / traceEvents) * 100);
	printf("Total disk reads: %d\n", readCount);
	printf("Total disk writes: %d\n", writeCount);	
}

//Random replacement algorithm
void rndm(int frames, char* traceFile, int debug)
{
	unsigned addr;	//Holds the address from the file
	char rw;	//Holds whether it is a read or write from the file

	unsigned pageNumber;	

	struct tableEntry PT[PTSIZE];		//Creates the page table
	struct tableEntry cache[frames];	//Creates the cache

	int foundCache = -1;
	srand(time(NULL)); 	//Must be done to get random numbers
	int random;

	//The front and next available slots in the arrays
	//Front of cache is initially -1 to indicate an empty array
	int myFrontCache = -1;	
	int nextAvailCache = 0;

	//Initializations
	PTInit(PT, PTSIZE);
	cacheInit(cache, frames);
	
	FILE* file;
	file = fopen(traceFile, "r");
	if(file)
	{
		//Will scan in everything in the file
		while(fscanf(file, "%x %c", &addr, &rw) != EOF)
		{
			pageNumber = addr / (16 * 16 * 16);	//Gets the page number. Offset is 12 bits, hence the 3 16s

			//Sets the valid bit
			if(PT[pageNumber].valid == 0)
			{
				PT[pageNumber].vpn = pageNumber;
				PT[pageNumber].valid = 1;
			}
			//Checks to see if the frame has already been loaded in cache
			foundCache = foundInCache(cache, frames, pageNumber, debug);
			//Page is already in cache
			if(foundCache >= 0)
			{
				if(rw == 'W')
				{
					cache[foundCache].dirty = 1;
				}
			}
			//Cache is empty
			else if(myFrontCache == -1)
			{
				if(debug)
				{
					printf("Filled in first entry of cache.\n");
				}
				myFrontCache = 0;
				nextAvailCache = 1;
				cache[myFrontCache].vpn = pageNumber;
				PT[pageNumber].present = 1;
				readCount++;
				if(rw == 'W')
				{
					cache[myFrontCache].dirty = 1;
				}			
			}
			//Cache is full
			else if(myFrontCache == nextAvailCache)
			{
				if(debug)
				{
					printf("Cache is full.\n");
				}
				//Gets a random number between 0 and frames - 1. That page is then replaced
				random = rand() % frames;
				if(cache[random].dirty == 1)
				{
					PT[cache[random].vpn].present = 0;
					if(debug)
					{
						printf("Write to disk.\n");
					}
					writeCount++;
				}
				if(debug)
				{
					printf("Read from disk.\n");
				}
				PT[pageNumber].present = 1;
				readCount++;
				cache[random] = PT[pageNumber];
				if(rw == 'W')
				{
					cache[random].dirty = 1;
				}
				else
				{
					cache[random].dirty = 0;
				}
			}
			//The cache has not yet been filled up
			else
			{
				if(debug)
				{
					printf("Slot %d in array is filled.\n", nextAvailCache);
				}
				cache[nextAvailCache].vpn = pageNumber;
				if(debug)
				{
					printf("Read from disk.\n");
				}
				readCount++;
				PT[pageNumber].present = 1;
				if(rw == 'W')
				{
					cache[nextAvailCache].dirty = 1;
				}
				nextAvailCache = (nextAvailCache + 1) % frames;
			}
			traceEvents++;
			foundCache = -1;
			if(debug)
			{
				cacheDisplay(cache, frames);
			}
		}
		fclose(file);
	}
	printf("Total memory frames: %d\n", frames);
	printf("Events in trace: %d\n", traceEvents);
	printf("Hits percentage: %f%%\n", ((double)hitCount / traceEvents) * 100);
	printf("Total disk reads: %d\n", readCount);
	printf("Total disk writes: %d\n", writeCount);
}

void opt(int frames, char* traceFile, int debug)
{
	unsigned addr;	//Holds the address from the file
	char rw;		//Holds whether it is a read or write from the file

	unsigned pageNumber;	

	struct tableEntry PT[PTSIZE];		//Creates the page table
	struct tableEntry pageReference[PTSIZE];	//Used to find distance of next page reference
	struct tableEntry cache[frames];	//Creates the cache

	int foundCache = -1;
	int i, j;	//Iterators
	int curLocation = 0;	//Location is the number of trace events that have happened
	int furthestAway;	//Holds the location in cache of the page that is furthest away

	//The front and next available slots in the arrays
	//Front of cache is initially -1 to indicate an empty array
	int myFrontCache = -1;	
	int nextAvailCache = 0;

	//Initializations
	PTInit(PT, PTSIZE);
	cacheInit(cache, frames);
	for(i = 0; i < PTSIZE; i++)
		pageReference[i].vpn = -1;
	
	FILE* file;
	//Fills up the references array
	file = fopen(traceFile, "r");
	while(fscanf(file, "%x %c", &addr, &rw) != EOF)
	{
		pageNumber = addr / (16 * 16 * 16);	//Gets the page number. Offset is 12 bits, hence the 3 16s
		pageReference[traceEvents].vpn =  pageNumber;
		traceEvents++;
	}
	fclose(file);
	
	file = fopen(traceFile, "r");
	if(file)
	{
		//Will scan in everything in the file
		while(fscanf(file, "%x %c", &addr, &rw) != EOF)
		{
			pageNumber = addr / (16 * 16 * 16);	//Gets the page number. Offset is 12 bits, hence the 3 16s

			//Sets the valid bit
			if(PT[pageNumber].valid == 0)
			{
				PT[pageNumber].vpn = pageNumber;
				PT[pageNumber].valid = 1;
			}
			//Checks to see if the frame has already been loaded in cache
			foundCache = foundInCache(cache, frames, pageNumber, debug);
			//Page is already in cache
			if(foundCache >= 0)
			{
				if(rw == 'W')
				{
					cache[foundCache].dirty = 1;
				}
			}
			//Cache is empty
			else if(myFrontCache == -1)
			{
				if(debug)
				{
					printf("Filled in first entry of cache.\n");
				}
				myFrontCache = 0;
				nextAvailCache = 1;
				cache[myFrontCache].vpn = pageNumber;
				PT[pageNumber].present = 1;
				readCount++;
				if(rw == 'W')
				{
					cache[myFrontCache].dirty = 1;
				}			
			}
			//Cache is full
			else if(myFrontCache == nextAvailCache)
			{
				if(debug)
				{
					printf("Cache is full.\n");
				}
				//Finds how far the next page reference is to determine which page to replace
				for(i = 0; i < frames; i++)
				{
					for(j = curLocation; j < traceEvents; j++)
					{
						if(cache[i].vpn == pageReference[j].vpn)
						{
							cache[i].distance = j;
							break;
						}
						//If the page is not referenced again
						if(j == traceEvents - 1)
						{
							cache[i].distance = -2;
						}
					}
				}
				//Finds which page is furthest away
				furthestAway = 0;
				for(i = 0; i < frames; i++)
				{
					if(cache[i].distance == -2)
					{
						furthestAway = i;
						break;
					}
					if(cache[i].distance > cache[furthestAway].distance)
					{
						furthestAway = i;
					}
				}
				if(cache[furthestAway].dirty == 1)
				{
					PT[cache[furthestAway].vpn].present = 0;
					if(debug)
					{
						printf("Write to disk.\n");
					}
					writeCount++;
				}
				if(debug)
				{
					printf("Read from disk.\n");
				}
				PT[pageNumber].present = 1;
				readCount++;
				cache[furthestAway] = PT[pageNumber];
				if(rw == 'W')
				{
					cache[furthestAway].dirty = 1;
				}
				else
				{
					cache[furthestAway].dirty = 0;
				}
			}
			//The cache has not yet been filled up
			else
			{
				if(debug)
				{
					printf("Slot %d in array is filled.\n", nextAvailCache);
				}
				cache[nextAvailCache].vpn = pageNumber;
				if(debug)
				{
					printf("Read from disk.\n");
				}
				readCount++;
				PT[pageNumber].present = 1;
				if(rw == 'W')
				{
					cache[nextAvailCache].dirty = 1;
				}
				nextAvailCache = (nextAvailCache + 1) % frames;
			}
			foundCache = -1;
			if(debug)
			{
				cacheDisplay(cache, frames);
			}
			curLocation++;
		}
		fclose(file);
	}
	printf("Total memory frames: %d\n", frames);
	printf("Events in trace: %d\n", traceEvents);
	printf("Hits percentage: %f%%\n", ((double)hitCount / traceEvents) * 100);
	printf("Total disk reads: %d\n", readCount);
	printf("Total disk writes: %d\n", writeCount);
}

int main(int argc, char *argv[])
{
	char *traceFile;
	int frames;
	char *alg;
	char *mode;
	int debug = 0;

	traceFile = argv[1];
	frames = atoi(argv[2]);
	alg = argv[3];
	mode = argv[4];

	if(strcmp(mode, "debug") == 0)
		debug = 1;
	
	if(strcmp(alg, "clk") == 0)
		clk(frames, traceFile, debug);
	else if(strcmp(alg, "fifo") == 0)
		fifo(frames, traceFile, debug);
	else if(strcmp(alg, "lru") == 0)
		lru(frames, traceFile, debug);
	else if(strcmp(alg, "rndm") == 0)
		rndm(frames, traceFile, debug);
	else if(strcmp(alg, "opt") == 0)
		opt(frames, traceFile, debug);
	return 0;
}
