#include <stdio.h>
#include <stdlib.h>

#include "../include/tiltyard_Error.h"

static char *error_code_strings[TILTYARD_ERROR_CODE_AMOUNT + TILTYARD_ERROR_HANDLING_CODE_AMOUNT] = {
	"The maximum arena capacity established was surpassed",
	"An arena pointer with a null value was given to a function",
	"There is not enough space to allocate a new arena",
	"There is not enough space to allocate a new arena with the desired size",
	"The size of an arena can not be <= 0",
	"The alignment provided is not valid, alignments must be any natural power of two (1,2,4,8,...)",
	"The alignment provided was too big for the arena's capacity",
	"The marker provided is out of bounds (it is either greater than the current capacity or greater than the current offset)",

	"There was an error with tiltyard's error handling (ironical, right?). Please make sure to take an screenshot or copy the error code and send it to the Github issues section, and I will probably fix it. Thanks for using tiltyard!"
};

static char *func_strings[TILTYARD_FUNC_AMOUNT + TILTYARD_ERROR_HANDLING_FUNC_AMOUNT] = {
	"tiltyard_create",
	"tiltyard_alloc",
	"tiltyard_calloc",
	"tiltyard_alloc_aligned",
	"tiltyard_calloc_aligned",
	"tiltyard_destroy",
	"tiltyard_wipe",
	"tiltyard_null",
	"tiltyard_destroy_and_null",
	"tiltyard_wipe_destroy_and_null",
	"tiltyard_reset",
	"tiltyard_get_marker",
	"tiltyard_reset_to",
	"tiltyard_clean_until",
	"tiltyard_clean_from",
	"tiltyard_clean_from_until",
	"tiltyard_get_capacity",
	"tiltyard_get_used_capacity",
	"tiltyard_get_available_capacity",
	"tiltyard_get_high_water",
	"tiltyard_alloc_count",
	"tiltyard_last_alloc",
	"tiltyard_get_stats",

	"get_error_code_string",
	"get_func_string"
};

static char *get_error_code_string(const enum tiltyard_error_code code)
{
	if (code > TILTYARD_ERROR_CODE_AMOUNT)
		tiltyard_handle_error(TILTYARD_ERROR_HANDLING_ERROR, GET_ERROR_CODE_STRING, true);

	return error_code_strings[code];
}

static char *get_func_string(const enum tiltyard_func func)
{
	if (func > TILTYARD_FUNC_AMOUNT)
		tiltyard_handle_error(TILTYARD_ERROR_HANDLING_ERROR, GET_FUNC_STRING, true);

	return func_strings[func];
}

void tiltyard_handle_error(const enum tiltyard_error_code error_code,
		  const enum tiltyard_func in_func,
		  const bool fatal
)
{
	if (fatal) {
		fprintf(stderr, "ERROR: %s ", get_error_code_string(error_code));
		fprintf(stderr, "FOUND AT FUNCTION: %s\n", get_func_string(in_func));
		abort();
	}

	fprintf(stderr, "WARNING: %s ", get_error_code_string(error_code));
	fprintf(stderr, "FOUND AT FUNCTION: %s\n", get_func_string(in_func));
}
