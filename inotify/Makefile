
# -- Compilers --
CC  = clang
CXX = clang++

# -- Output files --
OUT ?= inotify

# -- Directories --
SRC ?= .
INCLUDES = -I.
LDFLAGS = -L.

# -- Flags --
ifdef DEBUG
	CFLAGS		+= -g -O0 -DDEBUG=1
	LDFLAGS		+= -g -O0 -DDEBUG=1
endif
CFLAGS      += -fsanitize=address
LDFLAGS     += -fsanitize=address
CFLAGS 		+= $(INCLUDES)

# -- Input Files --
C_FILES   = $(wildcard $(SRC)/*.c)
CPP_FILES = $(wildcard $(SRC)/*.cpp) $(wildcard $(SRC)/*.cc)
OBJ_FILES = $(patsubst %.c, %.o, $(C_FILES)) $(patsubst %.cpp, %.o, $(CPP_FILES)) $(patsubst %.cc, %.o, $(CPP_FILES))

# -- Dependencies --
DEPFILE = .depfile

all: $(OUT)

# Output file
# =========================================================
$(OUT): $(OBJ_FILES)
	@echo Building binary $@ from [ $^ ]
	$(CXX) $(LDFLAGS) $^ -o $@

# Compile source files
# These rules are rewritten into the dependencies file
# =========================================================
.c.o:
	@echo Compiling $@ from [ $< ]
	$(CC) $(CFLAGS) -c $< -o $@

.cc.o:
	@echo Compiling $@ from [ $< ]
	$(CXX) $(CFLAGS) -c $< -o $@

.cpp.o:
	@echo Compiling $@ from [ $< ]
	$(CXX) $(CFLAGS) -c $< -o $@

# Generate dependencies.
# =========================================================
$(DEPFILE):
	rm -f $(DEPFILE)
	$(CC)  -E -MM $(CFLAGS) $(INCLUDES) $(CPP_FILES) $(C_FILES) -MF $(DEPFILE)

clean:
	rm -f $(OUT) $(SRC)/*.o $(DEPFILE)

# Generated dependency file.  Note the dependencies on
# $(DEPFILE)
# =========================================================
NODEPS:=clean tags svn
ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
	-include $(DEPFILE)
endif

.SUFFIXES: .c .cpp
.PHONY: all clean install