#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main() {
	printf("hello\n");
	write(1, "X", 1);
	usleep(1000 * 100);
}