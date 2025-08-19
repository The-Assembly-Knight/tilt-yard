#include <mm_malloc.h>
#include <string.h>

#include "../include/tiltyard_API.h"

Yard *tiltyard_create(size_t capacity)
{
	Yard *yard = malloc(sizeof(Yard));
	if (!yard) return NULL;
	
	yard->base = malloc(capacity);
	if (!yard->base) {
		free(yard);
		return NULL;
	}

	yard->capacity = capacity;
	yard->offset = 0;
	return yard;
}

void *tiltyard_alloc(Yard *yard, size_t size)
{
	if (!yard || yard->offset + size > yard->capacity || size == 0)
		return NULL;

	void *ptr = yard->base + yard->offset;
	yard->offset += size;
	return ptr;
}

void *tiltyard_calloc(Yard *yard, size_t size)
{
	void *ptr = tiltyard_alloc(yard, size);

	if (!ptr)
		return NULL;

	memset(ptr, 0, size);
	return ptr;
}

void tiltyard_reset(Yard *yard)
{
	yard->offset = 0;
}

void tiltyard_destroy(Yard *yard) 
{
	free(yard->base);
	free(yard);
}
