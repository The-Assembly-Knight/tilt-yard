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
	if (!yard) return;

	free(yard->base);
	free(yard);
}

void tiltyard_obliterate(Yard **yard)
{
	if (!yard || !*yard) return;

	tiltyard_destroy(*yard);
	*yard = NULL;
}

void tiltyard_obliterate_cleaning(Yard **yard)
{
	if (!yard || !*yard) return;

	memset((*yard)->base, 0, (*yard)->offset);
	tiltyard_obliterate(yard);
}

size_t tiltyard_marker(Yard *yard)
{
	if (!yard) return 0;

	return yard->offset;
}


