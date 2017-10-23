#include "func.h"

void *malloc(size_t size)
{
	pthread_mutex_lock(&lock);

	void *ret = myMalloc(size);

	pthread_mutex_unlock(&lock);
	return ret;
}
