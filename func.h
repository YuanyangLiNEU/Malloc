#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include <pthread.h>

typedef struct MallocBlock {
	size_t size;  				/* size of memory */
	struct MallocBlock* base;   /* point to base block */
	struct MallocBlock* pre;    /* point to pre block */
    struct MallocBlock* next;   /* point to next block */
    pthread_t tid;				/* thread id */
	int order; 					/* used for buddy allocation */
    int isFree;					/* if the block is used */
    int isLarge;				/* if the block is large block */
} MallocBlock;

void myPrintf(const char *fmt, ...);

void *myMalloc(size_t size);
void myFree(void *ptr);

void *largeMalloc(size_t size);
void *buddyMalloc(size_t size);
MallocBlock *getOrderedBlock(int order);
void mallocOrderedBlock(int order);
void mallocNewBlock();

void buddyFree(MallocBlock *mb);
void largeFree(MallocBlock *mb);

int getOrder(size_t size);
void deleteNode(MallocBlock **header, MallocBlock *mb);
void insertHead(MallocBlock **header, MallocBlock *mb);

void mallocStat();
void orderStat(int order);
void blockStat(MallocBlock *mb);

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void malloc_stats();

extern pthread_mutex_t lock;