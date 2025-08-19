#include <stdio.h>

#include "include/tiltyard_API.h"

int *test_func(Yard **main_yard)
{
	Yard *my_yard =  tiltyard_create(sizeof(int) * 10);
	int *int_array = tiltyard_alloc(my_yard, sizeof(int) * 0);
	
	*main_yard = my_yard;

	if (!int_array) return NULL;

	size_t i = 0;
	for (i = 0; i < 10; i++) {
		int_array[i] = (int)i + 10;
	}

	return int_array;
}

int main(void)
{
	Yard *main_yard = NULL;
	int *my_array = test_func(&main_yard);
	
	if (!my_array) return 1;

	if (!main_yard) return 1;

	size_t i = 0;
	for (i = 0; i < 10; i++) {
		printf("The content of the current pointer is: %i\n", my_array[i]);
	}

	tiltyard_destroy(main_yard);

	return 0;
}
