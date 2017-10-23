#include "util.h"

void malloc_stats() {
	myPrintf("Malloc Statistics\n\n");

	myPrintf("Total Allocated Requests: %llu\n", TOTAL_ALLOC_REQUESTS);
	myPrintf("Total Free Requests: %llu\n", TOTAL_FREE_REQUESTS);

	myPrintf("Mmap Bins Total Count: %llu\n", MMAP_BINS_TOTAL_COUNT);
	myPrintf("Mmap Bins In Use Count: %llu\n", MMAP_BINS_IN_USE_COUNT);
	myPrintf("Mmap Bins Free Count: %llu\n", MMAP_BINS_FREE_COUNT);

	myPrintf("Sbrk Bins Total Count: %llu\n", SBRK_BINS_TOTAL_COUNT);
	myPrintf("Sbrk Block Total Count: %llu\n", SBRK_BLOCK_TOTAL_COUNT);
	myPrintf("Sbrk Block In Use Count: %llu\n", SBRK_BLOCK_IN_USE_COUNT);
	myPrintf("Sbrk Block Free Count: %llu\n", SBRK_BLOCK_FREE_COUNT);
}

void *myMalloc(size_t size) {
	++TOTAL_ALLOC_REQUESTS;

	void *ret = NULL;
	if (size > LARGE_BLOCK_SIZE) {
		ret = largeMalloc(size);
	} else {
		ret = buddyMalloc(size);
	}

	if (ret == NULL) {
		myPrintf("malloc failed. size: %zu\n", size);
	}

	return ret;
}

void myFree(void *ptr) {
	++TOTAL_FREE_REQUESTS;

	if (ptr == NULL) {
		//myPrintf("attempt to free NULL: %p\n", ptr);
		return ;
	}

  	MallocBlock *mb = ptr - sizeof(MallocBlock);
	pthread_t self_id = pthread_self();
  	if (mb->tid != self_id) {
  		myPrintf("attempt to free other threads' memory\n");
  		return ;
  	}

  	if (mb->isLarge) {
  		largeFree(mb);
  	} else {
  		buddyFree(mb);
  	}
}

void myPrintf(const char *fmt, ...) {
	va_list args;
    va_start(args, fmt);
	char buf[1024];
  	vsnprintf(buf, 1024, fmt, args);
  	write(STDOUT_FILENO, buf, strlen(buf));
    va_end(args);
}

void blockStat(MallocBlock* mb) {
	if (mb == NULL) {
		myPrintf("The block is NULL\n");
		return ;
	}

	myPrintf("start addr: %p, order: %d, size: %zu, isFree: %d, isLarge: %d, thread id: %lu\n", 
		mb, mb->order, mb->size, mb->isFree, mb->isLarge, mb->tid);
}

void orderStat(int order) {
	//myPrintf("In order %d\n", order);
	MallocBlock *header = freeHeader[order];
	while (header != NULL) {
		blockStat(header);
		header = header->next;
	}	
}

void mallocStat() {
	myPrintf("---------------------buddyMalloc--------------------\n");
	for (int i = MAX_ORDER; i >= MIN_ORDER; --i) {
		myPrintf("In order %d\n", i);
		MallocBlock *header = freeHeader[i];
		while (header != NULL) {
			blockStat(header);
			header = header->next;
		}	
	}
	myPrintf("---------------------buddyMalloc--------------------\n");
/*	myPrintf("---------------------largeMalloc--------------------\n");
	MallocBlock *header = largeBlockHeader;
	while (header != NULL) {
		//blockStat(header);
		header = header->next;
	}	
	myPrintf("---------------------largeMalloc--------------------\n");*/
}

void *largeMalloc(size_t size) {
	//myPrintf("In large block malloc, size: %zu\n", size);
	//mallocStat();
	size_t requiredSize = size + sizeof(MallocBlock);
	void *ret = mmap(0, requiredSize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (ret < 0 || ret == MAP_FAILED) {
		myPrintf("mmap error in malloc\n");
		return NULL;
	}

	++MMAP_BINS_TOTAL_COUNT;
	++MMAP_BINS_IN_USE_COUNT;

	MallocBlock *mb = (MallocBlock *)ret;
	mb->size = requiredSize;
	mb->tid = pthread_self();
	mb->order = 0;
	mb->isFree = 0;
	mb->isLarge = 1;

	/*if (largeBlockHeader != NULL) {
		largeBlockHeader->pre = mb;
	}
	mb->next = largeBlockHeader;
	largeBlockHeader = mb;*/

	return (void *)(++mb);
}

int getOrder(size_t size) {
	int order = MIN_ORDER;
	for (int i = MIN_ORDER; i <= MAX_ORDER; ++i) {
		order = i;
		if ((1 << i) >= size) {
			break;
		}
	}

	return order;
}

MallocBlock *getOrderedBlock(int order) {
	MallocBlock *header = freeHeader[order];
	pthread_t tid = pthread_self();
	while (header != NULL && header->tid != tid) {
		header = header->next;
	}

	return header;
}

// use sbrk to malloc new memory
void mallocNewBlock() {
	size_t memorySize = NEW_PAGE_NUMBER * sysconf(_SC_PAGESIZE);
	void *ret = sbrk(memorySize);
	if (ret == (void *)-1) {
		myPrintf("sbrk error in malloc\n");
		return ;
	}

	++SBRK_BINS_TOTAL_COUNT;

	int cnt = memorySize / (1 << MAX_ORDER);
	MallocBlock *pmb = (MallocBlock *)(ret + (cnt - 1) * (1 << MAX_ORDER));
	MallocBlock *last = pmb;
	pmb->size = 1 << MAX_ORDER;
	pmb->tid = pthread_self();
	pmb->order = MAX_ORDER;
	pmb->isFree = 1;
	pmb->isLarge = 0;
	pmb->base = (MallocBlock *)ret;
	pmb->next = freeHeader[MAX_ORDER];
	for (int i = 0; i < cnt - 1; ++i) {
		MallocBlock *nextBlock = pmb;
		pmb = (MallocBlock *)((void *)pmb - (1 << MAX_ORDER));
		memcpy(pmb, nextBlock, sizeof(MallocBlock));
		pmb->next = nextBlock;
		nextBlock->pre = pmb;
	}
	pmb->pre = NULL;
	if (freeHeader[MAX_ORDER] != NULL) {
		freeHeader[MAX_ORDER]->pre = last;
	}
	freeHeader[MAX_ORDER] = pmb;

	SBRK_BLOCK_TOTAL_COUNT += cnt;
	SBRK_BLOCK_FREE_COUNT += cnt;
}

// malloc block in given order
void mallocOrderedBlock(int order) {
	MallocBlock* mb = NULL;
	int oi = order;
	mb = getOrderedBlock(oi);
	while (oi <= MAX_ORDER && mb == NULL) {
		mb = getOrderedBlock(oi++);
	}
	
	if (mb == NULL) {
		mallocNewBlock();
		mb = getOrderedBlock(MAX_ORDER);
	}

	for (int oi = mb->order; oi > order; --oi) {
		mb = getOrderedBlock(oi);

		//remove cur block from list
		deleteNode(&freeHeader[oi], mb);

		// split block into two parts and insert them into lower order
		MallocBlock* secPart = (MallocBlock *)((void *)mb + (1 << (oi - 1)));
		mb->size = 1 << (oi - 1);
		mb->order = oi - 1;
		mb->pre = NULL;
		mb->next = secPart;

		memcpy(secPart, mb, sizeof(MallocBlock));
		secPart->pre = mb;
		secPart->next = freeHeader[oi - 1];
		if (freeHeader[oi - 1] != NULL) {
			freeHeader[oi - 1]->pre = secPart;
		}
		freeHeader[oi - 1] = mb;

		++SBRK_BLOCK_TOTAL_COUNT;
		++SBRK_BLOCK_FREE_COUNT;
	}
}

void *buddyMalloc(size_t size) {
	size_t requiredSize = size + sizeof(MallocBlock);
	int order = getOrder(requiredSize);
	MallocBlock *mb = getOrderedBlock(order);
	if (mb == NULL) {
		mallocOrderedBlock(order);
		mb = getOrderedBlock(order);
	}

	//remove mb from free list
	deleteNode(&freeHeader[order], mb);
	mb->isFree = 0;

	++SBRK_BLOCK_IN_USE_COUNT;
	--SBRK_BLOCK_FREE_COUNT;

	return (void *)(++mb);
}

void largeFree(MallocBlock *mb) {
	// delete mb from list
	/*MallocBlock *pre = mb->pre;
	MallocBlock *next = mb->next;
	if (pre == NULL) {
		largeBlockHeader = next;
	} else {
		pre->next = next;
	}

	if (next != NULL) {
		next->pre = pre;
	}

	mb->isFree = 1;
	mb->pre = NULL;
	mb->next = NULL;*/

	int ret = 0;
	if ((ret = munmap(mb, mb->size)) < 0) {
		myPrintf("munmap failed: %d\n", ret);
	}

	--MMAP_BINS_IN_USE_COUNT;
	++MMAP_BINS_FREE_COUNT;
}

void buddyFree(MallocBlock *mb) {
	if (mb->isFree == 1) {
		myPrintf("Attempt to free a free block\n");
		return ;
	}

	mb->isFree = 1;
	int order = mb->order;

	for (int i = order; i < MAX_ORDER; ++i) {
		// try to find a neighbor free node
		MallocBlock *neighbor = NULL;

		// try to find pre one pre neighbor
		if (mb > mb->base) {
			neighbor = (void *)mb - (1 << i);
			if (neighbor->tid == mb->tid && neighbor->order == i && !neighbor->isLarge && neighbor->isFree) {
				deleteNode(&freeHeader[i], neighbor);

				neighbor->order = i + 1;
				neighbor->size = 1 << (i + 1);

				// set to invalid
				mb->order = 0;
				mb->isFree = 0;

				mb = neighbor;

				--SBRK_BLOCK_TOTAL_COUNT;
				--SBRK_BLOCK_FREE_COUNT;
				continue;
			}
		}

		//failed to find pre one, try to find next neighbor
		if ((void *)mb < (void *)mb->base + (1 << MAX_ORDER)) {
			neighbor = (void *)mb + (1 << i);
			if (neighbor->tid == mb->tid && neighbor->order == i && !neighbor->isLarge && neighbor->isFree) {
				deleteNode(&freeHeader[i], neighbor);

				//set to invalid
				neighbor->order = 0;
				neighbor->isFree = 0;

				mb->order = i + 1;
				mb->size = 1 << (i + 1);

				--SBRK_BLOCK_TOTAL_COUNT;
				--SBRK_BLOCK_FREE_COUNT;
				continue;
			}
		}
		
		break;
	}

	order = mb->order;
	insertHead(&freeHeader[order], mb);

	++SBRK_BLOCK_FREE_COUNT;
	--SBRK_BLOCK_IN_USE_COUNT;
}

void deleteNode(MallocBlock **header, MallocBlock *mb) {
	MallocBlock* pre = mb->pre;
	MallocBlock* next = mb->next;
	if (pre == NULL) {
		*header = next;
	} else {
		pre->next = next;
	}

	if (next != NULL) {
		next->pre = pre;
	}
}

void insertHead(MallocBlock **header, MallocBlock *mb) {
	mb->next = *header;
	mb->pre = NULL;
	if (*header != NULL) {
		(*header)->pre = mb;
	}
	*header = mb;
}