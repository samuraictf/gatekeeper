# Enable secondary expansion
.SECONDEXPANSION:

# For each directory $dir which has a makefile, the executable name
# should be $dir/$dir
TARGETS:=$(sort $(wildcard */Makefile))
TARGETS:=$(foreach makefile, $(TARGETS), $(shell dirname $(makefile)))
TARGETS:=$(foreach dir, $(TARGETS), $(dir)/$(dir))

all: $(TARGETS)
clean: $(TARGETS:=.clean)
test: $(TARGETS:=.bats)
	bats $^

$(TARGETS): % : $$(wildcard $$(@D)/*.c) $$(wildcard $$(@D)/*.h)
	@echo ========== $(@D) ==========
	make -C $(@D)

%.clean:
	make -C $(@D) clean

%.bats: %;

.PHONY: clean all test $(TARGETS:=.bats)