# -- Cross-compilation libraries --
#
# We have pre-compiled versions of libseccomp for many architectures.
#
ifneq (,$(findstring arm-,$(CROSS_COMPILE)))
LIB_FILES     += $(realpath)/../../lib/arm-linux-gnueabihf/libseccomp.a
else ifneq (,$(findstring aarch64-,$(CROSS_COMPILE)))
LDFLAGS     += $(realpath)/../../lib/aarch64-linux-gnu/libseccomp.a
else ifneq (,$(findstring i386-,$(CROSS_COMPILE)))
LDFLAGS     += $(realpath)/../../lib/i386-linux-gnu/libseccomp.a
else ifneq (,$(findstring x86_64-,$(CROSS_COMPILE)))
LDFLAGS     += $(realpath)/../../lib/x86_64-linux-gnu/libseccomp.a
else ifneq (,$(findstring mips-,$(CROSS_COMPILE)))
LDFLAGS     += -EL $(realpath)/../../lib/mips-linux-gnu/libseccomp.a
CFLAGS      += -EL
else
LDFLAGS     += $(realpath)/../../lib/$(shell uname -m)*/libseccomp.a
endif

include Makefile.exe
