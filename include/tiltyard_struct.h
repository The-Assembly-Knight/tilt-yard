#pragma once

#include <cstddef>
#include <cstdint>

typedef struct {
	uint8_t *base;
	size_t capacity;
	size_t offset;
} yard;
