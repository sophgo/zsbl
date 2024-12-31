#include <types.h>

void *_sbrk(unsigned long inc);

void *malloc (unsigned long nbytes)
{
	return _sbrk(nbytes);
}

void *realloc(void *ptr, unsigned long nbytes)
{
	void *dst;
	int i = 0;

	dst = malloc(nbytes);
	for (i = 0; i < nbytes; i++)
		*((char *)dst+i) = *((char *)ptr+i);

	return dst;
}

void *calloc(size_t nmemb, size_t size)
{
	return malloc(nmemb * size);
}

void free (void *addr)
{

}


