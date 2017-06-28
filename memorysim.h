#include <stdio.h>

//Initial counts
int traceEvents = 0;
int readCount = 0;
int writeCount = 0;
int hitCount = 0;

struct tableEntry{
	int vpn;	//Virtual page number
   	int dirty;	//Dirty bit
    	int valid;	//Valid bit
    	int reference;  //Used in clk. If 0, it gets replaced as it already has gone through one run
    	int present;	//Is it present in physical memory?
	int added;	//Used in lru. Sees when it was added to physical memory
	int distance; //Used in opt. Used to find to know how far a page is from being referenced
};

//Initializes the page table
void PTInit(struct tableEntry PT[], int PTSIZE)
{
	int i;
	for(i = 0; i < PTSIZE; i++)
	{
		PT[i].vpn = -1;
		PT[i].dirty = 0;
		PT[i].valid = 0;
		PT[i].present = -1;
	}
}

//Initializes the cache
void cacheInit(struct tableEntry cache[], int frames)
{
	int i;
	for(i = 0; i < frames; i++)
	{
		cache[i].vpn = -1;
		cache[i].reference = 1;
		cache[i].added = -1;
		cache[i].distance = -1;
	}
}

//Checks to see if frame has already been loaded into cache. Returns the location in cache unless not found
int foundInCache(struct tableEntry cache[], int frames, unsigned pageNumber, int debug)
{
	int i;
	for(i = 0; i < frames; i++)
	{
		if(cache[i].vpn == pageNumber)
		{
			hitCount++;
			if(debug)
			{
				printf("Found in cache.\n");
			}
			return i;	//Page was found in cache
		}
	}
	return -1;	//Page was not found in cache
}

//Displays the cache
void cacheDisplay(struct tableEntry cache[], int frames)
{
	int i;
	printf("Cache: ");
	for(i = 0; i < frames; i++)
	{
		printf("%d, ", cache[i].vpn);
	}
	printf("\n");
}
