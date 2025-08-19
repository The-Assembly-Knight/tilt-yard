#include <stdio.h>

#include "include/tiltyard_API.h"

#define YARD_SIZE 8

int *test_func(Yard **main_yard)
{
	Yard *my_yard =  tiltyard_create(sizeof(int) * YARD_SIZE);
	ssize_t mark = tiltyard_marker(my_yard);
	if (mark < 0) return NULL;

	printf("This is the current offset: %zu\n", my_yard->offset);
	int *int_array = tiltyard_calloc(my_yard, sizeof(int) * YARD_SIZE);
	
	printf("This is the current offset: %zu\n", my_yard->offset);
	*main_yard = my_yard;

	if (!int_array) return NULL;

	size_t i = 0;
	for (i = 0; i < YARD_SIZE; i++) {
		int_array[i] = (int)i + 10;
	}

	tiltyard_reset_to(my_yard, mark + 24);
	
	printf("This is the current offset: %zu\n", my_yard->offset);
	tiltyard_calloc(my_yard, sizeof(int) * 2);

	return int_array;
}

int main(void)
{
	Yard *main_yard = NULL;
	int *my_array = test_func(&main_yard);

	if (!main_yard || !my_array) return -1;

	size_t i = 0;
	for (i = 0; i < YARD_SIZE; i++) {
		printf("Value: %i\n", my_array[i]);
	}


	return 0;
}
