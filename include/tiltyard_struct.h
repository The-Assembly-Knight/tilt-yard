#pragma once

#include <sys/types.h>
#include <stdint.h>

typedef struct {
	uint8_t *base;
	size_t capacity;
	size_t offset;
} Yard;
