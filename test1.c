#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include <malloc.h>

void *mallocTest(void*);

//static const pthread_mutex_t lock;

/*void myPrintf(const char *fmt, ...) {
	va_list args;
    va_start(args, fmt);
	char buf[1024];
  	vsnprintf(buf, 1024, fmt, args);
  	write(STDOUT_FILENO, buf, strlen(buf));
  	//fsync(STDOUT_FILENO);
    va_end(args);
}*/

void myPrintf(const char *fmt, ...) {
	va_list args;
    va_start(args, fmt);
	char buf[1024];
  	vsnprintf(buf, 1024, fmt, args);
  	write(STDOUT_FILENO, buf, strlen(buf));
    va_end(args);
}

int main(int argc, char *argv[])
{
	//mallocTest(NULL);
/*	pthread_t inc_x_thread[100];
	int cnt = 5;
	for (int i = 0; i < cnt; ++i) {
		if (pthread_create(&inc_x_thread[i], NULL, mallocTest, NULL)) {
			printf("Error creating thread\n");
			return 1;
		}
	}

	for (int i = 0; i < cnt; ++i) {
		if(pthread_join(inc_x_thread[i], NULL)) {
			printf("Error joining thread\n");
			return 2;
		}
	}*/
	//myPrintf("OK%d%s\n", 123, "why");
	pthread_t self_id = pthread_self();

	size_t size = 8;
	//mallocStat();
	void *mem[10];
	for (int i = 0; i < 10; ++i) {
		myPrintf("malloc size: %zu\n", size);
		mem[i] = malloc(size);
		size *= 2;
	}
	for (int i = 0; i < 10; ++i) {
		myPrintf("free addr: %p\n", mem[9 - i]);
		free(mem[9 - i]);
		malloc_stats();
	}
	//mallocStat();
	//printf("In thread %lu, successfully malloc'd %zu bytes at addr %p\n", self_id, size, mem);
/*	mem = malloc(size);
	printf("In thread %lu, successfully malloc'd %zu bytes at addr %p\n", self_id, size, mem);
	mem = malloc(size);
	printf("In thread %lu, successfully malloc'd %zu bytes at addr %p\n", self_id, size, mem);
	mem = malloc(size);
	printf("In thread %lu, successfully malloc'd %zu bytes at addr %p\n", self_id, size, mem);

	size = 1024;
	mem = malloc(size);
	printf("In thread %lu, successfully malloc'd %zu bytes at addr %p\n", self_id, size, mem);
	mem = malloc(size);
	printf("In thread %lu, successfully malloc'd %zu bytes at addr %p\n", self_id, size, mem);*/


  	return 0;
}

/*void* mallocTest(void *param) {
	pthread_t self_id = pthread_self();
	size_t size = 16;
	void *mem = malloc(size);
	printf("In thread %lu, successfully malloc'd %zu bytes at addr %p\n", self_id, size, mem);
	char *s = (char *)mem;
	for (int i = 0; i < size - 1; ++i) {
		s[i] = 'a' + i;
	}
	s[size - 1] = 0;
	printf("In thread %lu, the content of allocated memory is: %s\n", self_id, s);
	size = 1024;
	mem = realloc(mem, size);
	printf("In thread %lu, successfully malloc'd %zu bytes at addr %p\n", self_id, size, mem);
	printf("In thread %lu, the content of allocated memory is: %s\n", self_id, s);
	free(mem);
	printf("In thread %lu, successfully free'd %zu bytes at addr %p\n", self_id, size, mem);

	return NULL;
}*/
