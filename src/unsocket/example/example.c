#include <sys/socket.h>

int main() {
	send(1, "hello", 5, 0);
}
