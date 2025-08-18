#pragma once

#include "tiltyard_struct.h"

Yard *tiltyard_create(size_t capacity);
void *tiltyard_alloc(Yard *yard, size_t size);
void tiltyard_reset(Yard *yard);
void tiltyard_destroy(Yard *yard);

