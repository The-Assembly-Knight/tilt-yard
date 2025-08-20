#pragma once

#include "tiltyard_struct.h"

Yard *tiltyard_create(size_t capacity);

void *tiltyard_alloc(Yard *yard, size_t size);
void *tiltyard_calloc(Yard *yard, size_t size);

void *tiltyard_alloc_aligned(Yard *yard, size_t size, size_t alignment);
void *tiltyard_calloc_aligned(Yard *yard, size_t size, size_t alignment);

void tiltyard_destroy(Yard *yard);
void tiltyard_wipe(Yard *yard);
void tiltyard_null(Yard **yard);

void tiltyard_destroy_and_null(Yard **yard);
void tiltyard_destroy_wipe_and_null(Yard **yard);


void tiltyard_reset(Yard *yard);

void tiltyard_obliterate(Yard **yard);
void tiltyard_obliterate_cleaning(Yard **yard);

size_t tiltyard_get_marker(Yard *yard);
void tiltyard_reset_to(Yard *yard, size_t marker);

void tiltyard_clean_until(Yard *yard, size_t marker);
void tiltyard_clean_from(Yard *yard, size_t marker);
void tiltyard_clean_from_until(Yard *yard, size_t marker_beg, size_t marker_end);
