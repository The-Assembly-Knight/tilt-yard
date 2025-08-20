#include <stdio.h>

#include "include/tiltyard_API.h"

#define YARD_SIZE 8

int* create_int_array(Yard **m, size_t size)
{
	const size_t main_yard_capacity = sizeof(int) * 10;
	Yard *main_yard = tiltyard_create(main_yard_capacity);
	*m = main_yard;
	int *int_array = tiltyard_calloc(main_yard, size);

	return int_array;
}

int main(void)
{
	Yard *main_yard = NULL;
	int *int_array = create_int_array(&main_yard, sizeof(int) * 2);
	
	if (!main_yard)
		return -1;

	size_t i = 0;
	for (i = 0; i < 2; i++) {
		int_array[i] = 1;

		int_array[i + 2] = 1;
	}

	size_t mark = tiltyard_get_marker(main_yard);
	printf("This is the value of the mark: %zu\n", mark);
	tiltyard_clean_from_until(main_yard, mark, mark);

	for (i = 0; i < 10; i++) {
		printf("This is the value of the thing: %i\n", int_array[i]);
	}

	tiltyard_destroy_wipe_and_null(&main_yard);

	return 0;
}
