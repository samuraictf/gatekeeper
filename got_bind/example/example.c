#include <unistd.h>
#include <stdint.h>

int main() {
	uintptr_t* pointer;
	uintptr_t value;

	// Featch a pointer to read.  This should be write@got
	read(0, &pointer, sizeof(pointer));

	// Send it twice, once before and once after calling write.
	value = *pointer;
	write(1, &value, sizeof(value));

	value = *pointer;
	write(1, &value, sizeof(value));
}