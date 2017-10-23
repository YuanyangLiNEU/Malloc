#include "func.h"

#ifndef MAX_ORDER
#define MAX_ORDER 10
#endif

#ifndef MIN_ORDER
#define MIN_ORDER 6
#endif

#ifndef LARGE_BLOCK_SIZE
#define LARGE_BLOCK_SIZE 512
#endif

#ifndef NEW_PAGE_NUMBER
#define NEW_PAGE_NUMBER 4
#endif

static MallocBlock* freeHeader[MAX_ORDER + 1] = {NULL};
// no need to track allocated large block
//static MallocBlock* largeBlockHeader = NULL;


/*  Total size of arena allocated with sbrk/mmap
    Total number of bins
    For each bin:
        Total number of blocks
        Used blocks
        Free blocks
        Total allocation requests
        Total free requests
*/
unsigned long long TOTAL_ALLOC_REQUESTS = 0;
unsigned long long TOTAL_FREE_REQUESTS = 0;

unsigned long long MMAP_BINS_TOTAL_COUNT = 0;
unsigned long long MMAP_BINS_IN_USE_COUNT = 0;
unsigned long long MMAP_BINS_FREE_COUNT = 0;

unsigned long long SBRK_BINS_TOTAL_COUNT = 0;
unsigned long long SBRK_BLOCK_TOTAL_COUNT = 0;
unsigned long long SBRK_BLOCK_IN_USE_COUNT = 0;
unsigned long long SBRK_BLOCK_FREE_COUNT = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
