CC:=clang

PROGGIES=test_echo

%: %.c
	$(CC) -o $@ $< $(CFLAGS)


all: $(PROGGIES)

clean:
	rm -f $(PROGGIES)