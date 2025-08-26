#include <stdio.h>
#include <assert.h>

#include "../include/tiltyard_Error.h"

void assert_error(const int condition, const char *error_message)
{
	if (condition) {
		fprintf(stderr, "%s", error_message);
		assert(!condition);
	}
}
