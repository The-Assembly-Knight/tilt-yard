#pragma once

#include "tiltyard_struct.h"

Yard *tiltyard_create(size_t capacity);
void *tiltyard_alloc(Yard *yard, size_t size);
void *tiltyard_calloc(Yard *yard, size_t size);

void tiltyard_reset(Yard *yard);
void tiltyard_destroy(Yard *yard);

void tiltyard_marker(Yard *yard);
void tiltyard_reset_to(Yard *yard, size_t marker);

void tiltyard_clean_to(Yard *yard, size_t marker);
void tiltyard_clean_from(Yard *yard, size_t marker);
