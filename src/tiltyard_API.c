#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../include/tiltyard_API.h"
#include "../include/tiltyard_Error.h"

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

/* Create a new arena with size 'capacity'.
 *
 * Create space in the heap for the arena
 * through malloc
 *
 * Returns:
 * - A null pointer if the capacity is 0 or
 *   there is not enough memory in the heap for that capacity or the arena pointer.
 * - A pointer to a arena allocated in the heap if there is enough
 *   memory in the heap for the capacity given.
 *
 * Notes:
 * - Allocates memory in the heap through malloc 2 times, one for the arena
 *   and the other one for the base of the arena (capacity).
 * - The memory allocated in the heap for the arena and its base
 *   must be freed through tiltyard_destroy, tiltyard_destroy_and_null, or
 *   tiltyard_wipe_destroy_and_null functions.
 */
Arena *tiltyard_create(size_t capacity)
{
	if (capacity == 0)
		tiltyard_handle_error(SIZE_EQUALS_ZERO, TILTYARD_CREATE, true);

	Arena *arena = malloc(sizeof(Arena));
	if (!arena) tiltyard_handle_error(NOT_ENOUGH_SPACE_FOR_ARENA, TILTYARD_CREATE, true);
	
	arena->base = malloc(capacity);
	if (!arena->base) {
		free(arena);
		tiltyard_handle_error(NOT_ENOUGH_SPACE_FOR_SIZE_OF_ARENA, TILTYARD_CREATE, true);
	}

	arena->capacity = capacity;
	arena->offset = 0;
	arena->last_alloc_offset = 0;
	arena->high_water = 0;
	arena->alloc_count = 0;
	return arena;
}

/* Allocate 'size' bytes from the arena with the default alignment
 *
 * The default alignment is sizeof(void *).
 *
 * Returns:
 * - A pointer into the arena if there is enough capacity.
 * - NULL if there is not enough space or arena == NULL.
 *
 * Notes:
 * - Does NOT call malloc/free; the returned pointer comes
 *   from the already allocated memory for the arena.
 * - The memory is uninitialized (use tiltyard_calloc if you need
 *   zeroed memory)
 */
void *tiltyard_alloc(Arena *arena, size_t size)
{
	return tiltyard_alloc_aligned(arena, size, sizeof(void *));
}

/* Allocate size bytes from the arena with the default alignment,
 * and zero-initialize the memory.
 *
 * Same behavior as 'tiltyard_alloc' except:
 * - The returned memory is set to all zero bytes.
 *
 * Returns:
 * - A pointer into the arena if there is enough capacity.
 * - NULL if there is not enough space or arena == NULL.
 *
 * Notes:
 * - Does NOT call malloc/free; the returned pointer comes
 *   from the already allocated memory for the arena.
 */
void *tiltyard_calloc(Arena *arena, size_t size)
{
	void *ptr = tiltyard_alloc(arena, size);

	if  (!ptr) tiltyard_handle_error(EXCEEDED_ARENA_CAPACITY, TILTYARD_CALLOC, true);

	memset(ptr, 0, size);
	return ptr;
}

/* Allocate size bytes from the arena with a custom alignment.
 *
 * Same behavior as 'tiltyard_alloc' except:
 * - The memory is aligned based on 'alignment' instead of a
 *   default one.
 *
 * This function also assign stats such as last_alloc, alloc_count,
 * and high_water to the arena for each allocation.
 *
 * Returns:
 * - A pointer into the arena if there is enough capacity.
 * - NULL if there is not enough space or arena == NULL.
 *
 * Notes:
 * - Does NOT call malloc/free; the returned pointer comes
 *   from the already allocated memory for the arena.
 * - The memory is uninitialized (use tiltyard_calloc_aligned if you need
 *   zeroed memory)
 */
void *tiltyard_alloc_aligned(Arena *arena, size_t size, size_t alignment)
{
	if (!arena)
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_ALLOC_ALIGNED, true);

	if (alignment == 0 || (alignment & (alignment - 1)) != 0)
		tiltyard_handle_error(INVALID_ALIGNMENT, TILTYARD_ALLOC_ALIGNED, true);

	size_t aligned_offset = (arena->offset + alignment - 1) & ~(alignment - 1);

	if (size_add_overflow(aligned_offset, size) || aligned_offset + size > arena->capacity)
		tiltyard_handle_error(ALIGNMENT_TOO_BIG, TILTYARD_ALLOC_ALIGNED, true);

	void *ptr = (char *)arena->base + aligned_offset;
	arena->last_alloc_offset = arena->offset;
	arena->alloc_count++;
	arena->offset = aligned_offset + size;
	if (arena->offset > arena->high_water)
		arena->high_water = arena->offset;
	return ptr;
}

/* Allocate size bytes from the arena with a custom alignment,
 * and zero-initialize the memory.
 *
 * Same behavior as 'tiltyard_calloc' except:
 * - The memory is aligned based on 'alignment' instead of a
 *   default one.
 *
 * Returns:
 * - A pointer into the arena if there is enough capacity.
 * - NULL if there is not enough space or arena == NULL.
 *
 * Notes:
 * - Does NOT call malloc/free; the returned pointer comes
 *   from the already allocated memory for the arena.
 */
void *tiltyard_calloc_aligned(Arena *arena, size_t size, size_t alignment)
{
	void *ptr = tiltyard_alloc_aligned(arena, size, alignment);

	if (!ptr)
		tiltyard_handle_error(EXCEEDED_ARENA_CAPACITY, TILTYARD_CALLOC_ALIGNED, true);

	memset(ptr, 0, size);
	return ptr;
}

/* Frees arena and its based (which are allocated in the heap)
 *
 * the arena's base and the arena itself will be freed using free
 * if 'arena' is not NULL.
 *
 * Returns:
 * - Nothing
 *
 * Notes:
 * - The pointer to arena will still point to where it pointed, even
 *   though the memory was already freed, in case you want to null the pointer
 *   please consider using 'tiltyard_null', 'tiltyard_destroy_and_null',
 *   or 'tiltyard_wipe_destroy_and_null'.
 * - Until the OS process that the memory was freed, it will still contain
 *   what it already had before freeing, in case you want to zero what you
 *   wrote on that memory please consider using 'tiltyard_wipe'
 *   or 'tiltyard_wipe_destroy_and_null'
 */
void tiltyard_destroy(Arena *arena) 
{
	if (!arena)
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_DESTROY, false);
	else {

		free(arena->base);
		free(arena);
	}
}

/* Zeroes all the memory in the arena.
 *
 * Uses memset to zero the entire arena if the 'arena' is not NULL
 *
 * Returns:
 * - Nothing
 *
 * Notes:
 * - Take into account that this function will not free the memory allocated
 *   for the arena nor null the arena. In case you want to do any of those actions,
 *   please consider using 'tiltyard_wipe_destroy_and_null'.
 */
void tiltyard_wipe(Arena *arena)
{
	if (!arena)
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_WIPE, false);

	memset(arena->base, 0, arena->capacity);
}

/* Nulls the pointer to the arena given by the user.
 *
 * Nulls 'arena' if 'arena' is not already NULL.
 *
 * Returns:
 * - Nothing
 *
 * Notes:
 * - Take into account that if the pointer is null
 *   there will not be any way to recover the arena, which
 *   means there will not be any way to free it.
 * - If you want to free the arena, use this function after you used
 *   'tiltyard_destroy'.
 * - The arena will keep containing what was allocated before it was nulled,
 *   so please make sure you use 'tiltyard_wipe' before this function
 *   or instead use 'tiltyard_wipe_destroy_and_null'.
 */
void tiltyard_null(Arena **arena)
{	
	if (!arena)
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_NULL, false);

	*arena = NULL;
}

/* Destroys and nulls the arena in that order
 *
 * If the given arena is not NULL,
 * combines the behaviors of 'tiltyard_destroy' and
 * 'tiltyard_null' in that order.
 *
 * Returns:
 * - Nothing.
 *
 * Notes:
 * - Take into account that the pointer to the arena
 *   will be null making it impossible to access the arena again.
 * - Take into account that this function will not prevent the memory
 *   where the arena was allocated to keep containing what it had before
 *   it was destroyed and null. In case you want to make sure the memory was
 *   zeroed use 'tiltyard_wipe_destroy_and_null'.
 */
void tiltyard_destroy_and_null(Arena **arena)
{
	if (!arena || !*arena)
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_DESTROY_AND_NULL, false);

	tiltyard_destroy(*arena);
	tiltyard_null(arena);
}

/* Wipes, destroys, and nulls the arena in that order.
 *
 * If 'arena' is not NULL, combines the behaviors of
 * 'tiltyard_wipe' and 'tiltyard_destroy_and_null' in
 * that order.
 *
 * Returns:
 * - Nothing
 *
 * Notes:
 * - Take into account that the pointer to the arena
 *   will be null making it impossible to access the arena again.
 */
void tiltyard_wipe_destroy_and_null(Arena **arena)
{
	if (!arena || !*arena)
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_WIPE_DESTROY_AND_NULL, false);

	tiltyard_wipe(*arena);
	tiltyard_destroy_and_null(arena);
}

/* Resets the current offset to 0
 *
 * Allows the user to reuse the arena from the beginning
 * by resetting the offset to 0 if the 'arena' is not NULL.
 *
 * Returns:
 * - Nothing.
 *
 * Notes:
 * - Even though the user resets the arena, the arena will still
 *   hold the data that was allocated into it.
 * - In case you want to reset the arena and reset all the data within it,
 *   you should use 'tiltyard_wipe' followed by this function.
 */
void tiltyard_reset(Arena *arena)
{
	if (!arena)
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_RESET, true);
	
	arena->offset = 0;
}

/* Gets current offset as a marker.
 *
 * Returns:
 * - 0 if 'arena' is NULL.
 * - Current offset if 'arena' is not NULL.
 *
 * Notes:
 * - The user should take into account that even if 'arena'
 *   is not NULL, marker can still be 0.
 */
size_t tiltyard_get_marker(Arena *arena)
{
	if (!arena)
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_GET_MARKER, true);

	return arena->offset;
}

/* Changes the current offset to 'marker'
 *
 * Changes the current offset to 'marker' as long
 * as 'arena' is not NULL, marker < offset, and
 * marker is <= the arena's capacity.
 *
 * Returns:
 * - Nothing
 *
 * Notes:
 * - Even if the offset offset is reset to marker, 
 *   the arena will conserve the data it had before.
 */
void tiltyard_reset_to(Arena *arena, size_t marker)
{
	if (!arena)
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_RESET_TO, true);
	
	if (marker > arena->capacity || marker > arena->offset)
		tiltyard_handle_error(OUT_OF_BOUNDS_MARKER, TILTYARD_RESET_TO, true);
	
	arena->offset = marker;
}

/* Zeroes all bytes from the beginning of the arena
 * to the marker.
 *
 * Zeroes all bytes from the beginning of the arena
 * to the marker using memset if arena is not NULL, marker is not 0,
 * and marker is not > arena's capacity.
 *
 * Returns:
 * - Nothing.
 *
 * Notes:
 * - The data zeroed will not be recoverable through tiltyard by
 *   any means, which means the user should think twice before using
 *   this function.
 */
void tiltyard_clean_until(Arena *arena, size_t marker)
{
	if (!arena)
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_CLEAN_UNTIL, true);

	if (marker == 0)
		return;

	if (marker > arena->capacity)
		tiltyard_handle_error(OUT_OF_BOUNDS_MARKER, TILTYARD_CLEAN_UNTIL, true);
	
	memset(arena->base, 0, marker);
}

/* Zeroes all bytes from the maker to the end of the arena
 *
 * Zeroes all bytes from the maker
 * to the end of the arena  using memset
 * if arena is not NULL, and the marker is <= arena's capacity.
 *
 * Returns:
 * - Nothing.
 *
 * Notes:
 * - The data zeroed will not be recoverable through tiltyard by
 *   any means, which means the user should think twice before using
 *   this function.
 */
void tiltyard_clean_from(Arena *arena, size_t marker)
{
	if (!arena)
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_CLEAN_FROM, true);

	if (marker >= arena->capacity)
		tiltyard_handle_error(OUT_OF_BOUNDS_MARKER, TILTYARD_CLEAN_FROM, true);
	
	memset((char *)arena->base + marker, 0, arena->capacity - marker);
}

/* Zeroes all bytes from 'maker_beg' to 'marker_end'
 *
 * Zeroes all bytes from 'maker_beg'
 * to 'marker_end'  using memset
 * if arena is not NULL, 'maker_beg' < 'maker_end',
 * 'marker_beg' < arena's capacity, and 'marker_end' <= arena's capacity.
 *
 * Returns:
 * - Nothing.
 *
 * Notes:
 * - The data zeroed will not be recoverable through tiltyard by
 *   any means, which means the user should think twice before using
 *   this function.
 */
void tiltyard_clean_from_until(Arena *arena, size_t marker_beg, size_t marker_end)
{
	if (!arena)
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_CLEAN_FROM_UNTIL, true);

	if (marker_beg >= marker_end || marker_beg >= arena->capacity || marker_end > arena->capacity)
		tiltyard_handle_error(OUT_OF_BOUNDS_MARKER, TILTYARD_CLEAN_FROM_UNTIL, true);

	memset((char *)arena->base + marker_beg, 0, marker_end - marker_beg);
}

/* Returns the capacity of the arena
 *
 * Returns arena's capacity if arena is not NULL.
 *
 * Returns:
 * - 0 if 'arena' is NULL.
 * - arena's capacity if 'arena' is not NULL.
 *
 * Notes:
 *  - This function is meant to be used as a way to debug
 *  the yards already created and it will not affect nor change
 *  any aspect of the arena.
 */
size_t tiltyard_get_capacity(Arena *arena)
{
	if (!arena) {
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_GET_CAPACITY, true);
		return 0;
	}

	return arena->capacity;
}

/* Returns the amount of bytes used already
 *
 * Returns arena's offset if arena is not NULL.
 *
 * Returns:
 * - 0 if 'arena' is NULL.
 * - arena's offset if 'arena' is not NULL.
 *
 * Notes:
 *  - This function is meant to be used as a way to debug
 *  the yards already created and it will not affect nor change
 *  any aspect of the arena.
 *  - Take into account that arena's used capacity may be 0, which means
 *  that even if 'arena' is not NULL you still can get 0.
 */
size_t tiltyard_get_used_capacity(Arena *arena)
{
	if (!arena) {
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_GET_USED_CAPACITY, true);
		return 0;
	}

	return arena->offset;
}

/* Return the amount of bytes available until the end of the arena
 *
 * Returns arena's available capacity if 'arena' is not NULL.
 *
 * Returns:
 * - 0 if 'arena' is NULL.
 * - arena's capacity - arena's offset if 'arena' is not NULL.
 *
 * Notes:
 *  - This function is meant to be used as a way to debug
 *  the yards already created and it will not affect nor change
 *  any aspect of the arena.
 *  - Take into account that arena's available capacity  may be 0, which means
 *  that even if 'arena' is not NULL you still can get 0.
 */
size_t tiltyard_get_available_capacity(Arena *arena)
{
	if (!arena) {
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_GET_AVAILABLE_CAPACITY, true);
		return 0;
	}

	return arena->capacity - arena->offset;
}

/* Return the closest allocation to the end of the arena.
 *
 * Returns arena's high_water if 'arena' is not NULL.
 *
 * Returns:
 * - 0 if 'arena' is NULL.
 * - arena's high_water if 'arena' is not NULL.
 *
 * Notes:
 *  - This function is meant to be used as a way to debug
 *  the yards already created and it will not affect nor change
 *  any aspect of the arena.
 *  - Take into account that arena's high_water  may be 0, which means
 *  that even if 'arena' is not NULL you still can get 0.
 */
size_t tiltyard_get_high_water(Arena *arena)
{
	if (!arena) {
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_GET_HIGH_WATER, true);
		return 0;
	}

	return arena->high_water;
}

/* Return the amount of allocations in the arena.
 *
 * Returns arena's alloc_count if arena is not NULL.
 *
 * Returns:
 * - 0 if 'arena' is NULL.
 * - arena's alloc_count if 'arena' is not NULL.
 *
 * Notes:
 *  - This function is meant to be used as a way to debug
 *  the yards already created and it will not affect nor change
 *  any aspect of the arena.
 *  - Take into account that arena's alloc_count  may be 0, which means
 *  that even if 'arena' is not NULL you still can get 0.
 */
size_t tiltyard_get_alloc_count(Arena *arena)
{
	if (!arena) {
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_GET_ALLOC_COUNT, true);
		return 0;
	}

	return arena->alloc_count;
}

/* Return the last allocation's offset in the arena.
 *
 * Returns arena's last_alloc_offset if arena is not NULL.
 *
 * Returns:
 * - 0 if 'arena' is NULL.
 * - arena's last_alloc_offset if 'arena' is not NULL.
 *
 * Notes:
 *  - This function is meant to be used as a way to debug
 *  the yards already created and it will not affect nor change
 *  any aspect of the arena.
 *  - Take into account that arena's last_alloc may be 0, which means
 *  that even if 'arena' is not NULL you still can get 0.
 */
size_t tiltyard_get_last_alloc_offset(Arena *arena)
{
	if (!arena) {
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_GET_LAST_ALLOC, true);
		return 0;
	}

	return arena->last_alloc_offset;
}

/* Return all the stats of the arena.
 *
 * Returns TiltyardStats struct with the arena's stats.
 *
 * Returns:
 * - TiltyardStats with all values zeroed if 'arena' is null.
 * - TiltyardStats with all arena's stats if 'arena' is not null.
 *
 * Notes:
 *  - This function is meant to be used as a way to debug
 *  the yards already created and it will not affect nor change
 *  any aspect of the arena.
 */
TiltyardStats tiltyard_get_stats(Arena *arena)
{
	if (!arena)
		tiltyard_handle_error(NULL_POINTER_TO_ARENA, TILTYARD_GET_STATS, true);

	TiltyardStats stats = {
		.capacity = tiltyard_get_capacity(arena),
		.used = tiltyard_get_used_capacity(arena),
		.available = tiltyard_get_available_capacity(arena),
		.high_water = tiltyard_get_high_water(arena),
		.alloc_count = tiltyard_get_alloc_count(arena),
		.last_alloc_offset = tiltyard_get_last_alloc_offset(arena),
	};
	return stats;
}
