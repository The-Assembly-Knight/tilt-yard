#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../include/tiltyard_API.h"

/* Check if a+b overflows size_t
 *
 * Adds a and b and checks if it overflows
 * SIZE_MAX (which is the max size of size_t)
 *
 * Returns:
 * - 1 if a+b overflows size_t.
 * - 0 if a+b does not overflow size_t.
 *
 * Notes:
 * - Nothing.
 */
static inline int size_add_overflow(size_t a, size_t b)
{
	if (a > SIZE_MAX - b) return 1;
	return 0;
}

/* Create a new yard with size 'capacity'.
 *
 * Create space in the heap for the yard
 * through malloc
 *
 * Returns:
 * - A null pointer if the capacity is 0 or
 *   there is not enough memory in the heap for that capacity or the yard pointer.
 * - A pointer to a yard allocated in the heap if there is enough
 *   memory in the heap for the capacity given.
 *
 * Notes:
 * - Allocates memory in the heap through malloc 2 times, one for the yard
 *   and the other one for the base of the yard (capacity).
 * - The memory allocated in the heap for the yard and its base
 *   must be freed through tiltyard_destroy, tiltyard_destroy_and_null, or
 *   tiltyard_wipe_destroy_and_null functions.
 */
Yard *tiltyard_create(size_t capacity)
{
	if (capacity == 0)
		return NULL;

	Yard *yard = malloc(sizeof(Yard));
	if (!yard) return NULL;
	
	yard->base = malloc(capacity);
	if (!yard->base) {
		free(yard);
		return NULL;
	}

	yard->capacity = capacity;
	yard->offset = 0;
	yard->last_alloc_offset = 0;
	yard->high_water = 0;
	yard->alloc_count = 0;
	return yard;
}

/* Allocate 'size' bytes from the yard with the default alignment
 *
 * The default alignment is sizeof(void *).
 *
 * Returns:
 * - A pointer into the yard if there is enough capacity.
 * - NULL if there is not enough space or yard == NULL.
 *
 * Notes:
 * - Does NOT call malloc/free; the returned pointer comes
 *   from the already allocated memory for the yard.
 * - The memory is uninitialized (use tiltyard_calloc if you need
 *   zeroed memory)
 */
void *tiltyard_alloc(Yard *yard, size_t size)
{
	return tiltyard_alloc_aligned(yard, size, sizeof(void *));
}

/* Allocate size bytes from the yard with the default alignment,
 * and zero-initialize the memory.
 *
 * Same behavior as 'tiltyard_alloc' except:
 * - The returned memory is set to all zero bytes.
 *
 * Returns:
 * - A pointer into the yard if there is enough capacity.
 * - NULL if there is not enough space or yard == NULL.
 *
 * Notes:
 * - Does NOT call malloc/free; the returned pointer comes
 *   from the already allocated memory for the yard.
 */
void *tiltyard_calloc(Yard *yard, size_t size)
{
	void *ptr = tiltyard_alloc(yard, size);

	if  (!ptr) return NULL;

	memset(ptr, 0, size);
	return ptr;
}

/* Allocate size bytes from the yard with a custom alignment.
 *
 * Same behavior as 'tiltyard_alloc' except:
 * - The memory is aligned based on 'alignment' instead of a
 *   default one.
 *
 * This function also assign stats such as last_alloc, alloc_count,
 * and high_water to the yard for each allocation.
 *
 * Returns:
 * - A pointer into the yard if there is enough capacity.
 * - NULL if there is not enough space or yard == NULL.
 *
 * Notes:
 * - Does NOT call malloc/free; the returned pointer comes
 *   from the already allocated memory for the yard.
 * - The memory is uninitialized (use tiltyard_calloc_aligned if you need
 *   zeroed memory)
 */
void *tiltyard_alloc_aligned(Yard *yard, size_t size, size_t alignment)
{
	if (!yard || size == 0 || alignment == 0 || (alignment & (alignment - 1)) != 0)
		return NULL;

	size_t aligned_offset = (yard->offset + alignment - 1) & ~(alignment - 1);

	if (size_add_overflow(aligned_offset, size) || aligned_offset + size > yard->capacity)
		return NULL;

	void *ptr = (char *)yard->base + aligned_offset;
	yard->last_alloc_offset = yard->offset;
	yard->alloc_count++;
	yard->offset = aligned_offset + size;
	if (yard->offset > yard->high_water)
		yard->high_water = yard->offset;
	return ptr;
}

/* Allocate size bytes from the yard with a custom alignment,
 * and zero-initialize the memory.
 *
 * Same behavior as 'tiltyard_calloc' except:
 * - The memory is aligned based on 'alignment' instead of a
 *   default one.
 *
 * Returns:
 * - A pointer into the yard if there is enough capacity.
 * - NULL if there is not enough space or yard == NULL.
 *
 * Notes:
 * - Does NOT call malloc/free; the returned pointer comes
 *   from the already allocated memory for the yard.
 */
void *tiltyard_calloc_aligned(Yard *yard, size_t size, size_t alignment)
{
	void *ptr = tiltyard_alloc_aligned(yard, size, alignment);

	if (!ptr) return NULL;

	memset(ptr, 0, size);
	return ptr;
}

/* Frees yard and its based (which are allocated in the heap)
 *
 * the yard's base and the yard itself will be freed using free
 * if 'yard' is not NULL.
 *
 * Returns:
 * - Nothing
 *
 * Notes:
 * - The pointer to yard will still point to where it pointed, even
 *   though the memory was already freed, in case you want to null the pointer
 *   please consider using 'tiltyard_null', 'tiltyard_destroy_and_null',
 *   or 'tiltyard_wipe_destroy_and_null'.
 * - Until the OS process that the memory was freed, it will still contain
 *   what it already had before freeing, in case you want to zero what you
 *   wrote on that memory please consider using 'tiltyard_wipe'
 *   or 'tiltyard_wipe_destroy_and_null'
 */
void tiltyard_destroy(Yard *yard) 
{
	if (!yard) return;

	free(yard->base);
	free(yard);
}

/* Zeroes all the memory in the yard.
 *
 * Uses memset to zero the entire yard if the 'yard' is not NULL
 *
 * Returns:
 * - Nothing
 *
 * Notes:
 * - Take into account that this function will not free the memory allocated
 *   for the yard nor null the yard. In case you want to do any of those actions,
 *   please consider using 'tiltyard_wipe_destroy_and_null'.
 */
void tiltyard_wipe(Yard *yard)
{
	if (!yard) return;

	memset(yard->base, 0, yard->capacity);
}

/* Nulls the pointer to the yard given by the user.
 *
 * Nulls 'yard' if 'yard' is not already NULL.
 *
 * Returns:
 * - Nothing
 *
 * Notes:
 * - Take into account that if the pointer is null
 *   there will not be any way to recover the yard, which
 *   means there will not be any way to free it.
 * - If you want to free the yard, use this function after you used
 *   'tiltyard_destroy'.
 * - The yard will keep containing what was allocated before it was nulled,
 *   so please make sure you use 'tiltyard_wipe' before this function
 *   or instead use 'tiltyard_wipe_destroy_and_null'.
 */
void tiltyard_null(Yard **yard)
{	
	if (!yard) return;

	*yard = NULL;
}

/* Destroys and nulls the yard in that order
 *
 * If the given yard is not NULL,
 * combines the behaviors of 'tiltyard_destroy' and
 * 'tiltyard_null' in that order.
 *
 * Returns:
 * - Nothing.
 *
 * Notes:
 * - Take into account that the pointer to the yard
 *   will be null making it impossible to access the yard again.
 * - Take into account that this function will not prevent the memory
 *   where the yard was allocated to keep containing what it had before
 *   it was destroyed and null. In case you want to make sure the memory was
 *   zeroed use 'tiltyard_wipe_destroy_and_null'.
 */
void tiltyard_destroy_and_null(Yard **yard)
{
	if (!yard || !*yard) return;

	tiltyard_destroy(*yard);
	tiltyard_null(yard);
}

/* Wipes, destroys, and nulls the yard in that order.
 *
 * If 'yard' is not NULL, combines the behaviors of
 * 'tiltyard_wipe' and 'tiltyard_destroy_and_null' in
 * that order.
 *
 * Returns:
 * - Nothing
 *
 * Notes:
 * - Take into account that the pointer to the yard
 *   will be null making it impossible to access the yard again.
 */
void tiltyard_wipe_destroy_and_null(Yard **yard)
{
	if (!yard || !*yard) return;

	tiltyard_wipe(*yard);
	tiltyard_destroy_and_null(yard);
}

/* Resets the current offset to 0
 *
 * Allows the user to reuse the arena from the beginning
 * by resetting the offset to 0 if the 'yard' is not NULL.
 *
 * Returns:
 * - Nothing.
 *
 * Notes:
 * - Even though the user resets the yard, the yard will still
 *   hold the data that was allocated into it.
 * - In case you want to reset the yard and reset all the data within it,
 *   you should use 'tiltyard_wipe' followed by this function.
 */
void tiltyard_reset(Yard *yard)
{
	if (yard) yard->offset = 0;
}

/* Gets current offset as a marker.
 *
 * Returns:
 * - 0 if 'yard' is NULL.
 * - Current offset if 'yard' is not NULL.
 *
 * Notes:
 * - The user should take into account that even if 'yard'
 *   is not NULL, marker can still be 0.
 */
size_t tiltyard_get_marker(Yard *yard)
{
	if (!yard) return 0;

	return yard->offset;
}

/* Changes the current offset to 'marker'
 *
 * Changes the current offset to 'marker' as long
 * as 'yard' is not NULL, marker < offset, and
 * marker is <= the yard's capacity.
 *
 * Returns:
 * - Nothing
 *
 * Notes:
 * - Even if the offset offset is reset to marker, 
 *   the yard will conserve the data it had before.
 */
void tiltyard_reset_to(Yard *yard, size_t marker)
{
	if (!yard || marker < yard->offset || marker > yard->offset)
		return;
	
	yard->offset = marker;
}

/* Zeroes all bytes from the beginning of the yard
 * to the marker.
 *
 * Zeroes all bytes from the beginning of the yard
 * to the marker using memset if yard is not NULL, marker is not 0,
 * and marker is not > yard's capacity.
 *
 * Returns:
 * - Nothing.
 *
 * Notes:
 * - The data zeroed will not be recoverable through tiltyard by
 *   any means, which means the user should think twice before using
 *   this function.
 */
void tiltyard_clean_until(Yard *yard, size_t marker)
{
	if (!yard || marker == 0 || marker > yard->capacity)
		return;
	
	memset(yard->base, 0, marker);
}

/* Zeroes all bytes from the maker to the end of the yard
 *
 * Zeroes all bytes from the maker
 * to the end of the yard  using memset
 * if yard is not NULL, and the marker is <= yard's capacity.
 *
 * Returns:
 * - Nothing.
 *
 * Notes:
 * - The data zeroed will not be recoverable through tiltyard by
 *   any means, which means the user should think twice before using
 *   this function.
 */
void tiltyard_clean_from(Yard *yard, size_t marker)
{
	if (!yard || marker >= yard->capacity)
		return;
	
	memset((char *)yard->base + marker, 0, yard->capacity - marker);
}

/* Zeroes all bytes from 'maker_beg' to 'marker_end'
 *
 * Zeroes all bytes from 'maker_beg'
 * to 'marker_end'  using memset
 * if yard is not NULL, 'maker_beg' < 'maker_end',
 * 'marker_beg' < yard's capacity, and 'marker_end' <= yard's capacity.
 *
 * Returns:
 * - Nothing.
 *
 * Notes:
 * - The data zeroed will not be recoverable through tiltyard by
 *   any means, which means the user should think twice before using
 *   this function.
 */
void tiltyard_clean_from_until(Yard *yard, size_t marker_beg, size_t marker_end)
{
	if (!yard || marker_beg >= marker_end || marker_beg >= yard->capacity || marker_end > yard->capacity)
		return;

	memset((char *)yard->base + marker_beg, 0, marker_end - marker_beg);
}

/* Returns the capacity of the yard
 *
 * Returns yard's capacity if yard is not NULL.
 *
 * Returns:
 * - 0 if 'yard' is NULL.
 * - yard's capacity if 'yard' is not NULL.
 *
 * Notes:
 *  - This function is meant to be used as a way to debug
 *  the yards already created and it will not affect nor change
 *  any aspect of the yard.
 */
size_t tiltyard_get_capacity(Yard *yard)
{
	if (!yard) return 0;
	return yard->capacity;
}

/* Returns the amount of bytes used already
 *
 * Returns yard's offset if yard is not NULL.
 *
 * Returns:
 * - 0 if 'yard' is NULL.
 * - yard's offset if 'yard' is not NULL.
 *
 * Notes:
 *  - This function is meant to be used as a way to debug
 *  the yards already created and it will not affect nor change
 *  any aspect of the yard.
 *  - Take into account that yard's used capacity may be 0, which means
 *  that even if 'yard' is not NULL you still can get 0.
 */
size_t tiltyard_get_used_capacity(Yard *yard)
{
	if (!yard) return 0;
	return yard->offset;
}

/* Return the amount of bytes available until the end of the yard
 *
 * Returns yard's available capacity if 'yard' is not NULL.
 *
 * Returns:
 * - 0 if 'yard' is NULL.
 * - yard's capacity - yard's offset if 'yard' is not NULL.
 *
 * Notes:
 *  - This function is meant to be used as a way to debug
 *  the yards already created and it will not affect nor change
 *  any aspect of the yard.
 *  - Take into account that yard's available capacity  may be 0, which means
 *  that even if 'yard' is not NULL you still can get 0.
 */
size_t tiltyard_get_available_capacity(Yard *yard)
{
	if (!yard) return 0;
	return yard->capacity - yard->offset;
}

/* Return the closest allocation to the end of the yard.
 *
 * Returns yard's high_water if 'yard' is not NULL.
 *
 * Returns:
 * - 0 if 'yard' is NULL.
 * - yard's high_water if 'yard' is not NULL.
 *
 * Notes:
 *  - This function is meant to be used as a way to debug
 *  the yards already created and it will not affect nor change
 *  any aspect of the yard.
 *  - Take into account that yard's high_water  may be 0, which means
 *  that even if 'yard' is not NULL you still can get 0.
 */
size_t tiltyard_get_high_water(Yard *yard)
{
	if (!yard) return 0;
	return yard->high_water;
}

/* Return the amount of allocations in the yard.
 *
 * Returns yard's alloc_count if yard is not NULL.
 *
 * Returns:
 * - 0 if 'yard' is NULL.
 * - yard's alloc_count if 'yard' is not NULL.
 *
 * Notes:
 *  - This function is meant to be used as a way to debug
 *  the yards already created and it will not affect nor change
 *  any aspect of the yard.
 *  - Take into account that yard's alloc_count  may be 0, which means
 *  that even if 'yard' is not NULL you still can get 0.
 */
size_t tiltyard_get_alloc_count(Yard *yard)
{
	if (!yard) return 0;
	return yard->alloc_count;
}

/* Return the last allocation's offset in the yard.
 *
 * Returns yard's last_alloc_offset if yard is not NULL.
 *
 * Returns:
 * - 0 if 'yard' is NULL.
 * - yard's last_alloc_offset if 'yard' is not NULL.
 *
 * Notes:
 *  - This function is meant to be used as a way to debug
 *  the yards already created and it will not affect nor change
 *  any aspect of the yard.
 *  - Take into account that yard's last_alloc may be 0, which means
 *  that even if 'yard' is not NULL you still can get 0.
 */
size_t tiltyard_get_last_alloc_offset(Yard *yard)
{
	if (!yard) return 0;
	return yard->last_alloc_offset;
}

/* Return all the stats of the yard.
 *
 * Returns TiltyardStats struct with the yard's stats.
 *
 * Returns:
 * - TiltyardStats with all values zeroed if 'yard' is null.
 * - TiltyardStats with all yard's stats if 'yard' is not null.
 *
 * Notes:
 *  - This function is meant to be used as a way to debug
 *  the yards already created and it will not affect nor change
 *  any aspect of the yard.
 */
TiltyardStats tiltyard_get_stats(Yard *yard)
{
	TiltyardStats stats = {
		.capacity = tiltyard_get_capacity(yard),
		.used = tiltyard_get_used_capacity(yard),
		.available = tiltyard_get_available_capacity(yard),
		.high_water = tiltyard_get_high_water(yard),
		.alloc_count = tiltyard_get_alloc_count(yard),
		.last_alloc_offset = tiltyard_get_last_alloc_offset(yard),
	};
	return stats;
}
