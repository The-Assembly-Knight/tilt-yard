#pragma once

#include <sys/types.h>
#include <stdint.h>

typedef struct {
	uint8_t *base;
	size_t capacity;
	size_t offset;

	size_t last_alloc_offset;
	size_t high_water;
	size_t alloc_count;
} Arena;

typedef struct {
	size_t capacity;
	size_t used;
	size_t available;
	size_t high_water;
	size_t alloc_count;
	size_t last_alloc_offset;
} TiltyardStats;

Arena *tiltyard_create(size_t capacity);

void *tiltyard_alloc(Arena *arena, size_t size);
void *tiltyard_calloc(Arena *arena, size_t size);

void *tiltyard_alloc_aligned(Arena *arena, size_t size, size_t alignment);
void *tiltyard_calloc_aligned(Arena *arena, size_t size, size_t alignment);

void tiltyard_destroy(Arena *arena);
void tiltyard_wipe(Arena *arena);
void tiltyard_null(Arena **arena);

void tiltyard_destroy_and_null(Arena **arena);
void tiltyard_wipe_destroy_and_null(Arena **arena);

void tiltyard_reset(Arena *arena);

size_t tiltyard_get_marker(Arena *arena);
void tiltyard_reset_to(Arena *arena, size_t marker);

void tiltyard_clean_until(Arena *arena, size_t marker);
void tiltyard_clean_from(Arena *arena, size_t marker);
void tiltyard_clean_from_until(Arena *arena, size_t marker_beg, size_t marker_end);

size_t tiltyard_get_capacity(Arena *arena);
size_t tiltyard_get_used_capacity(Arena *arena);
size_t tiltyard_get_available_capacity(Arena *arena);
size_t tiltyard_get_high_water(Arena *arena);
size_t tiltyard_get_alloc_count(Arena *arena);
size_t tiltyard_get_last_alloc(Arena *arena);
TiltyardStats tiltyard_get_stats(Arena *arena);
