#pragma once

#include "tiltyard_struct.h"

typedef struct {
	size_t capacity;
	size_t used;
	size_t available;
	size_t high_water;
	size_t alloc_count;
	size_t last_alloc;
} TiltyardStats;

Yard *tiltyard_create(size_t capacity);

void *tiltyard_alloc(Yard *yard, size_t size);
void *tiltyard_calloc(Yard *yard, size_t size);

void *tiltyard_alloc_aligned(Yard *yard, size_t size, size_t alignment);
void *tiltyard_calloc_aligned(Yard *yard, size_t size, size_t alignment);

void tiltyard_destroy(Yard *yard);
void tiltyard_wipe(Yard *yard);
void tiltyard_null(Yard **yard);

void tiltyard_destroy_and_null(Yard **yard);
void tiltyard_wipe_destroy_and_null(Yard **yard);

void tiltyard_reset(Yard *yard);

size_t tiltyard_get_marker(Yard *yard);
void tiltyard_reset_to(Yard *yard, size_t marker);

void tiltyard_clean_until(Yard *yard, size_t marker);
void tiltyard_clean_from(Yard *yard, size_t marker);
void tiltyard_clean_from_until(Yard *yard, size_t marker_beg, size_t marker_end);

size_t tiltyard_get_capacity(Yard *yard);
size_t tiltyard_get_used_capacity(Yard *yard);
size_t tiltyard_get_available_capacity(Yard *yard);
size_t tiltyard_get_high_water(Yard *yard);
size_t tiltyard_get_alloc_count(Yard *yard);
size_t tiltyard_get_last_alloc(Yard *yard);
TiltyardStats tiltyard_get_stats(Yard *yard);
