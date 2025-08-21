#pragma once

#include <sys/types.h>
#include <stdint.h>

typedef struct {
	uint8_t *base;
	size_t capacity;
	size_t offset;

	size_t last_alloc;
	size_t high_water;
	size_t alloc_count;
} Yard;
