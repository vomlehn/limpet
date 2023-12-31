# Make file for building and testing Limpet

# Supported versions are:
# LINUX
# SINGLE_THREADED_LINUX

VERSION=LINUX

SHELL = /bin/bash

BIN = bin
SRC = src
ACTUAL = actual
SCRIPTS = test/scripts

PATH := $(SCRIPTS):$(PATH)
export PATH

SETPATH = export PATH=$(PATH)

# Be sure bin and src directories are created
$(shell mkdir -p $(BIN) $(SRC) $(ACTUAL))

# Sent to "C" to compiler for C, "C++" to compiler for C++
LANG = C++

ifeq "$(LANG)" "C++"
CC = g++
SFX = cc
else ifeq "$(LANG)" "C"
CC = gcc
SFX = c
else
$(error LANG must be set to "cc" or "c")
endif

# Flags for various versions
# Linux configuration
INCS_LINUX = include/limpet-linux.h
CPPFLAGS_LINUX = -DLIMPET=LIMPET_LINUX

# Linux single-threaded configuration, a good starting point for embedded
# use
INCS_SINGLE_THREADED_LINUX = include/limpet-single-threaded-linux.h
CPPFLAGS_SINGLE_THREADED_LINUX = -DLIMPET=LIMPET_SINGLE_THREADED_LINUX

# Set up configuration info
CPPFLAGS := $(CPPFLAGS_$(VERSION))
CPPFLAGS += -g -ggdb
CPPFLAGS += -Wall -Wextra -Werror
CPPFLAGS += -Wno-unused-parameter
CPPFLAGS += -Iinclude

INCS := include/limpet.h include/limpet-sysdep.h
INCS += $(INCS_$(VERSION))

# Produce a list of file names for test executables
TEST_BINS = \
    $(sort $(shell $(SETPATH); print-testnames $(VERSION) $(TESTS) | \
	sed -e 's/^[^:]*$$/$(BIN)\/&/' -e 's/^.*:/$(BIN)\//'))

# Come up with a list of just the test file names, without any preceeding
# directory name
TEST_NAME_LIST = $(shell $(SETPATH); print-testnames $(VERSION) $(TESTS))

# Define test-specific flags
define print_cppflags
$(SETPATH); print-cppflags $(VERSION) $(1)
endef

.PHONY: test
test: $(TEST_BINS)
	run-tests $(VERSION) $(ACTUAL) $(BIN) "$(TEST_NAME_LIST)"
	check-tests $(ACTUAL) "$(TEST_NAME_LIST)"

$(BIN)/assert: $(BIN)/assert.o $(LIMPET_HDRS)
	$(CC) -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/assert.o: $(SRC)/assert.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) $(shell $(call print_cppflags,assert)) -c \
	    -o $@ $(filter-out %.h,$^)

$(BIN)/default-verbose: $(BIN)/default-verbose.o $(LIMPET_HDRS)
	$(CC) -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/default-verbose.o: $(SRC)/simple.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) $(shell $(call print_cppflags,default-verbose)) -c \
	    -o $@ $(filter-out %.h,$^)

$(BIN)/doc-example: $(BIN)/doc-example.o $(LIMPET_HDRS)
	$(CC) -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/doc-example.o: $(SRC)/simple.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) $(shell $(call print_cppflags,doc-example)) -c \
	    -o $@ $(filter-out %.h,$^)

$(BIN)/maxjobs: $(BIN)/maxjobs.o $(LIMPET_HDRS)
	$(CC) -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/maxjobs.o: $(SRC)/maxjobs.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) $(shell $(call print_cppflags,maxjobs)) -c \
	    -o $@ $(filter-out %.h,$^)

$(BIN)/not-verbose: $(BIN)/not-verbose.o $(LIMPET_HDRS)
	$(CC) -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/not-verbose.o: $(SRC)/simple.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) $(shell $(call print_cppflags,not-verbose)) -c \
	    -o $@ $(filter-out %.h,$^)

$(BIN)/signal: $(BIN)/signal.o $(LIMPET_HDRS)
	$(CC) -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/signal.o: $(SRC)/signal.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) $(shell $(call print_cppflags,signal)) -c \
	    -o $@ $(filter-out %.h,$^)

$(BIN)/simple: $(BIN)/simple.o $(LIMPET_HDRS)
	$(CC) -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/simple.o: $(SRC)/simple.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) $(shell $(call print_cppflags,simple)) -c \
	    -o $@ $(filter-out %.h,$^)

$(BIN)/skip1: $(BIN)/skip1.o $(LIMPET_HDRS)
	$(CC) -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/skip1.o: $(SRC)/skip.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) $(shell $(call print_cppflags,skip1)) \
	    -c -o $@ $(filter-out %.h,$^)

$(BIN)/skip2: $(BIN)/skip2.o $(LIMPET_HDRS)
	$(CC) -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/skip2.o: $(SRC)/skip.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) $(shell $(call print_cppflags,skip2)) -c \
	    -o $@ $(filter-out %.h,$^)

$(BIN)/timeout: $(BIN)/timeout.o $(LIMPET_HDRS)
	$(CC) -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/timeout.o: $(SRC)/timeout.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) $(shell $(call print_cppflags,timeout)) -c \
	    -o $@ $(filter-out %.h,$^)

$(BIN)/two-files: $(BIN)/two-files-main.o $(BIN)/two-files-sub.o
	$(CC) -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/two-files-main.o: $(SRC)/two-files-main.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) $(shell $(call print_cppflags,two-files)) -c \
	    -o $@ $(filter-out %.h,$^)

$(BIN)/two-files-sub.o: $(SRC)/two-files-sub.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) $(shell $(call print_cppflags,two-files)) -c \
	    -o $@ $(filter-out %.h,$^)

$(SRC)/%.$(SFX): test/%.cc
	cp $^ $@

.PHONY: clean
clean:
	rm -rf $(BIN) $(SRC) $(ACTUAL)

clobber: clean
	rmdir $(BIN) $(SRC)
