# Enable secondary expansion
.SECONDEXPANSION:

# For each directory $dir which has a makefile, the executable name
# should be $dir/$dir
TARGETS:=$(wildcard */Makefile)
TARGETS:=$(foreach makefile, $(TARGETS), $(shell dirname $(makefile)))
TARGETS:=$(foreach dir, $(TARGETS), $(dir)/$(dir))

all: $(TARGETS)

clean:
	@- for target in $(TARGETS); do \
		make -C $$(dirname $$target) clean; \
	done

$(TARGETS): % : $$(wildcard $$(@D)/*.c) $$(wildcard $$(@D)/*.h)
	make -C $(@D)

.PHONY: clean all
.SUFFIXES: Makefile

