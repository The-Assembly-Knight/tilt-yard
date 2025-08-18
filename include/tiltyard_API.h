#pragma once

#include "tiltyard_struct.h"

yard *tiltyard_create(size_t capacity);
void *tiltyard_alloc(yard *y, size_t size);
void tiltyard_reset(yard *y);
void tiltyard_destroy(yard *y);

