CC:=clang

PROGGIES=test_echo test_binary

%: %.c
	$(CC) -o $@ $< $(CFLAGS)


all: $(PROGGIES)

clean:
	rm -f $(PROGGIES)