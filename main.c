#include "include/tiltyard_API.h"

int main(void)
{
	Arena *my_arena = tiltyard_create(0);
	tiltyard_destroy_and_null(&my_arena);
	return 0;
}
