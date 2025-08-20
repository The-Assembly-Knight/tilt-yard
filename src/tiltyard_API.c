#include <stddef.h>
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
	return tiltyard_alloc_aligned(yard, size, sizeof(void *));
}

void *tiltyard_calloc(Yard *yard, size_t size)
{
	void *ptr = tiltyard_alloc(yard, size);

	if  (!ptr) return NULL;

	memset(ptr, 0, size);
	return ptr;
}

void *tiltyard_alloc_aligned(Yard *yard, size_t size, size_t alignment)
{
	if (!yard || size == 0 || alignment == 0 || (alignment & (alignment - 1)) != 0)
		return NULL;

	size_t aligned_offset = (yard->offset + alignment - 1) & ~(alignment - 1);

	if (aligned_offset + size > yard->capacity)
		return NULL;

	void *ptr = (char *)yard->base + aligned_offset;
	yard->offset = aligned_offset + size;
	return ptr;
}

void *tiltyard_calloc_aligned(Yard *yard, size_t size, size_t alignment)
{
	void *ptr = tiltyard_alloc_aligned(yard, size, alignment);

	if (!ptr) return NULL;

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
