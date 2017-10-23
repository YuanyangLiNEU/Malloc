#include "func.h"

void free(void *ptr)
{
	pthread_mutex_lock(&lock);

	myFree(ptr);

  	pthread_mutex_unlock(&lock);
}