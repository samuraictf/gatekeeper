CFLAGS += -O0 -ggdb \
	-fstack-protector-all -Wformat -Wformat=2 -Wformat-security -fPIE -D_FORTIFY_SOURCE=2 \
	-pedantic-errors \
	-Wall -Wextra \
	-Wcast-align -Wpointer-arith -Wmissing-noreturn -Wmissing-format-attribute -Wredundant-decls \
	-Wunreachable-code -Winline -Wfloat-equal \
	-D_INOTIFY
	
LDFLAGS += -lpcre

all:
	gcc $(CFLAGS) -o gatekeeper gatekeeper.c sockets.c ringbuffer.c pcre.c $(LDFLAGS)

clean:
	rm -f gatekeeper *.o
