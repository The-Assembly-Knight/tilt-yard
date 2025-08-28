#include "../include/tiltyard_Error.h"

static char *error_code_strings[TILTYARD_ERROR_CODE_AMOUNT] = {
	""
};

static char *func_strings[TILTYARD_FUNC_AMOUNT] = {
	""
};

static char *get_error_code_string(const enum tiltyard_error_code code)
{
	if (code > TILTYARD_ERROR_CODE_AMOUNT)
		return 0;

	return error_code_strings[code];
}

static char *get_func_string(const enum tiltyard_func func)
{
	if (func > TILTYARD_FUNC_AMOUNT)
		return 0;

	return func_strings[func];
}


void handle_error(const enum tiltyard_error_code error_code,
		  const enum tiltyard_func in_func,
		  const bool fatal
)
{
}
