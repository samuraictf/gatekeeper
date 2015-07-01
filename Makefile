TARGETS=$(wildcard */Makefile)

all:
	for target in $(TARGETS); do \
		echo ======= $$(dirname $$target) ======= ; \
		pushd $$(dirname $$target) >/dev/null; \
		make all; \
		popd >/dev/null; \
	done

clean:
	for target in $(TARGETS); do \
		echo ======= $$(dirname $$target) ======= ; \
		pushd $$(dirname $$target) >/dev/null; \
		make clean; \
		popd >/dev/null; \
	done

.PHONY: clean all
