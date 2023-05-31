# Makefile, v1
# Sistemas Operativos, DEI/IST/ULisboa 2021-22
#
# This makefile should be run from the *root* of the project

CC ?= gcc 
LD ?= gcc 

# space separated list of directories with header files
INCLUDE_DIRS := fs .
# this creates a space separated list of -I<dir> where <dir> is each of the values in INCLUDE_DIRS
INCLUDES = $(addprefix -I, $(INCLUDE_DIRS))

SOURCES  := $(wildcard */*.c)
HEADERS  := $(wildcard */*.h)
OBJECTS  := $(SOURCES:.c=.o)
TARGET_EXECS := tests/tfs_built_in_functions tests/copy_to_external_simple tests/copy_to_external_errors tests/write_10_blocks_spill tests/write_10_blocks_simple tests/write_more_than_10_blocks_simple tests/threads1 tests/simple_threads1 tests/copy_to_external_threads2 

# VPATH is a variable used by Makefile which finds *sources* and makes them available throughout the codebase
# vpath %.h <DIR> tells make to look for header files in <DIR>
vpath # clears VPATH
vpath %.h $(INCLUDE_DIRS)

CFLAGS = -std=c11 -D_POSIX_C_SOURCE=200809L -fsanitize=thread
CFLAGS += $(INCLUDES)

# Warnings
CFLAGS += -fdiagnostics-color=always -Wall -Werror -Wextra -Wcast-align -Wconversion -Wfloat-equal -Wformat=2 -Wnull-dereference -Wshadow -Wsign-conversion -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunused -lpthread 
# Warning suppressions
CFLAGS += -Wno-sign-compare 
LDFLAGS = -pthread -fsanitize=thread

# optional debug symbols: run make DEBUG=no to deactivate them
ifneq ($(strip $(DEBUG)), no)
  CFLAGS += -g 
endif

# optional O3 optimization symbols: run make OPTIM=no to deactivate them
ifeq ($(strip $(OPTIM)), no)
  CFLAGS += -O0 
else
  CFLAGS += -O3 
endif

# A phony target is one that is not really the name of a file
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean depend fmt

all: clean $(TARGET_EXECS)

test : test1 test2 test3 test4 test5 test6 test7 test8 test9
	@echo "Ending tests :)"

test1:
	@echo ----- Test 1 ------
	./tests/tfs_built_in_functions

test2:
	@echo ----- Test 2 ------
	./tests/copy_to_external_simple

test3:
	@echo ----- Test 3 ------
	./tests/copy_to_external_errors

test4:
	@echo ----- Test 4 ------
	./tests/write_10_blocks_simple

test5:
	@echo ----- Test 5 ------
	./tests/write_more_than_10_blocks_simple

test6:
	@echo ----- Test 6 ------
	./tests/write_10_blocks_spill

test7:
	@echo ----- Test 7 ------
	./tests/threads1

test8:
	@echo ----- Test 8 ------
	./tests/simple_threads1

test9:
	@echo ----- Test 9 ------
	./tests/copy_to_external_threads2
# The following target can be used to invoke clang-format on all the source and header
# files. clang-format is a tool to format the source code based on the style specified 
# in the file '.clang-format'.
# More info available here: https://clang.llvm.org/docs/ClangFormat.html

# The $^ keyword is used in Makefile to refer to the right part of the ":" in the 
# enclosing rule. See https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

fmt: $(SOURCES) $(HEADERS)
	clang-format -i $^



# Note the lack of a rule.
# make uses a set of default rules, one of which compiles C binaries
# the CC, LD, CFLAGS and LDFLAGS are used in this rule
tests/tfs_built_in_functions: tests/tfs_built_in_functions.o fs/operations.o fs/state.o
tests/copy_to_external_errors: tests/copy_to_external_errors.o fs/operations.o fs/state.o
tests/copy_to_external_simple: tests/copy_to_external_simple.o fs/operations.o fs/state.o
tests/write_10_blocks_spill: tests/write_10_blocks_spill.o fs/operations.o fs/state.o
tests/write_10_blocks_simple: tests/write_10_blocks_simple.o fs/operations.o fs/state.o
tests/write_more_than_10_blocks_simple: tests/write_more_than_10_blocks_simple.o fs/operations.o fs/state.o
tests/threads1: tests/threads1.o fs/operations.o fs/state.o -lpthread
tests/simple_threads1: tests/simple_threads1.o fs/operations.o fs/state.o -lpthread 
tests/copy_to_external_threads2: tests/copy_to_external_threads2.o fs/operations.o fs/state.o -lpthread 
clean:
	rm -f $(OBJECTS) $(TARGET_EXECS)




# This generates a dependency file, with some default dependencies gathered from the include tree
# The dependencies are gathered in the file autodep. You can find an example illustrating this GCC feature, without Makefile, at this URL: https://renenyffenegger.ch/notes/development/languages/C-C-plus-plus/GCC/options/MM
# Run `make depend` whenever you add new includes in your files
depend : $(SOURCES)
	$(CC) $(INCLUDES) -MM $^ > autodep
