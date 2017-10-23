#include "func.h"

void *realloc(void *ptr, size_t size)
{
	pthread_mutex_lock(&lock);
	void *ret = ptr;

	if (ptr == NULL && size == 0) {
		myPrintf("ptr is NULL and size is 0\n");
		pthread_mutex_unlock(&lock);
		return ret;
	}

  	if (ptr == NULL) {
  		ret = myMalloc(size);
  		pthread_mutex_unlock(&lock);
  		return ret;
  	}

  	if (size == 0) {
  		myFree(ptr);
  		pthread_mutex_unlock(&lock);
  		return ptr;
  	}

  	MallocBlock *mb = ptr - sizeof(MallocBlock);
  	pthread_t self_id = pthread_self();
  	if (mb->tid != self_id) {
  		myPrintf("attempt to realloc other threads' memory\n");
  		pthread_mutex_unlock(&lock);
  		return ret;
  	}

  	size_t curSize = mb->size - sizeof(MallocBlock);
  	if (curSize != size) {
  		ret = myMalloc(size);
  		size_t copySize = size > curSize ? curSize : size;
  		memcpy(ret, ptr, copySize);
  		myFree(ptr);
  	}

	pthread_mutex_unlock(&lock);
  	return ret;
}