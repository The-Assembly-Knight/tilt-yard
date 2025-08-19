#pragma once

#include "tiltyard_struct.h"

Yard *tiltyard_create(size_t capacity);
void *tiltyard_alloc(Yard *yard, size_t size);
void *tiltyard_calloc(Yard *yard, size_t size);

void tiltyard_reset(Yard *yard);
void tiltyard_destroy(Yard *yard);
void tiltyard_obliterate(Yard **yard);
void tiltyard_obliterate_cleaning(Yard **yard);

ssize_t tiltyard_marker(Yard *yard);
void tiltyard_reset_to(Yard *yard, ssize_t marker);

void tiltyard_clean_until(Yard *yard, ssize_t marker);
void tiltyard_clean_from(Yard *yard, ssize_t marker);
void tiltyard_clean_from_until(Yard *yard, ssize_t marker_beg, ssize_t marker_end);
