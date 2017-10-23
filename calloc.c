#include "func.h"

void *calloc(size_t nmemb, size_t size)
{
	pthread_mutex_lock(&lock);

	if (nmemb == 0 || size == 0) {
		pthread_mutex_unlock(&lock);
		return NULL;
	}
	
	size_t totalSize = nmemb * size;
	void* ret = myMalloc(totalSize);
	if (ret != NULL) {
		memset(ret, 0, totalSize);
	}

	pthread_mutex_unlock(&lock);
  	return ret;
}