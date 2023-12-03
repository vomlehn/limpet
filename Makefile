# Make file for building and testing Limpet

SHELL = /bin/bash

# Supported versions are:
# LINUX
# SINGLE_THREADED_LINUX

VERSION=LINUX

BIN=bin
SRC=src
ACTUAL= actual
SCRIPTS=test/scripts

# Be sure bin and src directories are created
$(shell mkdir -p $(BIN) $(SRC))

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
TESTS_LINUX = LIMPET_MAX_JOBS="2":maxjobs

# Linux single-threaded configuration, a good starting point for embedded
# use
INCS_SINGLE_THREADED_LINUX = include/limpet-single-threaded-linux.h
CPPFLAGS_SINGLE_THREADED_LINUX = -DLIMPET=LIMPET_SINGLE_THREADED_LINUX
# We're skipping things that require passing parameters until it's
# implemented at run time.
TESTS_SINGLE_THREADED_LINUX = LIMPET_MAX_JOBS="1":maxjobs \

# Set up configuration info
CPPFLAGS := $(CPPFLAGS_$(VERSION))
CPPFLAGS += -g -ggdb
CPPFLAGS += -Wall -Wextra -Werror
CPPFLAGS += -Wno-unused-parameter
CPPFLAGS += -Iinclude

INCS := include/limpet.h include/limpet-sysdep.h
INCS += $(INCS_$(VERSION))

# Tests are separated by spaces. Test parameters are separated by colons,
# meaning we can substituted spaces for colons to get the shell command
# line. We can get the test name by removing everything up to and including
# the last colon.
COMMON_TESTS := \
	signal \
	simple \
	two-files \
	"LIMPET_RUNLIST=\"skip1 skip3\"":skip1 \
	"LIMPET_RUNLIST=\"\"":skip2 \
	LIMPET_TIMEOUT=0.5:timeout \
	LIMPET_MAX_JOBS="1":maxjobs
TESTS = $(COMMON_TESTS) $(TESTS_$(VERSION))

ECHO_TESTS = declare -a tests; \
	tests=($(TESTS)); \
    for test in "$${tests[@]}"; do \
        echo "$$test"; \
    done

# Use the shell to parse the list of tests so that we can use
# quote-delimited strings
TEST_LIST = $(sort $(shell ($(ECHO_TESTS)) | \
	sed -e 's/^[^:]*$$/$(BIN)\/&/' -e 's/^.*:/$(BIN)\//'))

# Come up with a list of just the test names
TEST_NAME_LIST = $(sort $(shell ($(ECHO_TESTS)) | sed -e 's/^.*://'))

PATH := $(SCRIPTS):$(PATH)
export PATH

.PHONY: test
test: $(TEST_LIST)
	run-tests.sh $(ACTUAL) $(BIN) "$(TEST_NAME_LIST)"
	check-tests.sh $(ACTUAL) "$(TEST_NAME_LIST)"

$(BIN)/maxjobs: $(BIN)/maxjobs.o $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/maxjobs.o: $(SRC)/maxjobs.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) -c -o $@ $(filter-out %.h,$^)

$(BIN)/signal: $(BIN)/signal.o $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/signal.o: $(SRC)/signal.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) -c -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/simple: $(BIN)/simple.o $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/simple.o: $(SRC)/simple.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) -c -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/skip1: $(BIN)/skip.o $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) -o $@ $(filter-out %.h,$^) $(LDFLAGS)

# Same binary, but with another name so output will be unique
$(BIN)/skip2: $(BIN)/skip1
	cp $^ $@

$(BIN)/skip.o: $(SRC)/skip.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) -c -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/timeout: $(BIN)/timeout.o $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/timeout.o: $(SRC)/timeout.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) -c -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/two-files: $(BIN)/two-files-main.o $(BIN)/two-files-sub.o
	$(CC) $(CPPFLAGS) -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/two-files-main.o: $(SRC)/two-files-main.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) -c -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(BIN)/two-files-sub.o: $(SRC)/two-files-sub.$(SFX) $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) -c -o $@ $(filter-out %.h,$^) $(LDFLAGS)

$(SRC)/%.$(SFX): test/%.cc
	cp $^ $@

.PHONY: clean
clean:
	rm -f $(BIN)/* $(SRC)/*

clobber: clean
	rmdir $(BIN) $(SRC)
