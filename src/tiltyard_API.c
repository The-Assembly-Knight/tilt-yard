#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../include/tiltyard_API.h"

Yard *tiltyard_create(size_t capacity)
{
	if (capacity == 0)
		return NULL;

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

	void *ptr = (char *)yard->base + yard->offset;
	yard->offset += size;
	return ptr;
}

void *tiltyard_calloc(Yard *yard, size_t size)
{
	if (!yard) return NULL;

	void *ptr = tiltyard_alloc(yard, size);

	if (!ptr)
		return NULL;

	memset(ptr, 0, size);
	return ptr;
}

void tiltyard_reset(Yard *yard)
{
	if (yard) yard->offset = 0;
}

void tiltyard_destroy(Yard *yard) 
{
	if (!yard) return;

	free(yard->base);
	free(yard);
}

void tiltyard_wipe(Yard *yard)
{
	if (!yard) return;

	memset(yard->base, 0, yard->capacity);
}

void tiltyard_null(Yard **yard)
{	
	if (!yard) return;

	*yard = NULL;
}

void tiltyard_destroy_and_null(Yard **yard)
{
	if (!yard || !*yard) return;

	tiltyard_destroy(*yard);
	tiltyard_null(yard);
}

void tiltyard_destroy_wipe_and_null(Yard **yard)
{
	if (!yard || !*yard) return;

	tiltyard_wipe(*yard);
	tiltyard_destroy_and_null(yard);
}

size_t tiltyard_get_marker(Yard *yard)
{
	if (!yard) return SIZE_MAX;

	return yard->offset;
}

void tiltyard_reset_to(Yard *yard, size_t marker)
{
	if (!yard || marker > yard->capacity)
		return;
	
	yard->offset = marker;
}

void tiltyard_clean_until(Yard *yard, size_t marker)
{
	if (!yard || marker <= 0 || marker > yard->capacity)
		return;
	
	memset(yard->base, 0, marker);
}

void tiltyard_clean_from(Yard *yard, size_t marker)
{
	if (!yard || marker >= yard->capacity)
		return;
	
	memset((char *)yard->base + marker, 0, yard->capacity - marker);
}
