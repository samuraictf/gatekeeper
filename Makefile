# Enable secondary expansion
.SECONDEXPANSION:

# For each directory $dir which has a makefile, the executable name
# should be $dir/$dir
TARGETS:=$(wildcard */Makefile)
TARGETS:=$(foreach makefile, $(TARGETS), $(shell dirname $(makefile)))
TARGETS:=$(foreach dir, $(TARGETS), $(dir)/$(dir))

all: $(TARGETS)
clean: $(TARGETS:=.clean)
test: $(TARGETS) $(TARGETS:=.test)

$(TARGETS): % : $$(wildcard $$(@D)/*.c) $$(wildcard $$(@D)/*.h)
	@echo ========== $(@D) ==========
	make -C $(@D)

%.clean:
	make -C $(@D) clean

%.test:
	bats */*.bats preload/*/*.bats

.PHONY: clean all test

