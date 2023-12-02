# Make file for building and testing Limpet

SHELL = /bin/bash

# Supported versions are:
# LINUX
# SINGLE_THREADED

VERSION=LINUX

BIN=bin
SRC=src

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
INCS_SINGLE_THREADED = include/limpet-single-threaded.h
CPPFLAGS_SINGLE_THREADED = -DLIMPET=LIMPET_SINGLE_THREADED
TESTS_SINGLE_THREADED = LIMPET_MAX_JOBS="1":maxjobs

# Set up configuration info
CPPFLAGS := $(CPPFLAGS_$(VERSION))
CPPFLAGS += -g -ggdb
CPPFLAGS += -Wall -Wextra -Werror
CPPFLAGS += -Wno-unused-parameter
CPPFLAGS += -Iinclude

ACTUAL= actual
CANONICAL = test/canonical

INCS := include/limpet.h include/limpet-sysdep.h
INCS += $(INCS_$(VERSION))

# Tests are separated by spaces. Test parameters are separated by colons,
# meaning we can substituted spaces for colons to get the shell command
# line. We can get the test name by removing everything up to and including
# the last colon.
COMMON_TESTS := \
	signal \
	simple \
	"LIMPET_RUNLIST=\"skip1 skip3\"":skip \
	"LIMPET_RUNLIST=\"\"":skip \
	LIMPET_TIMEOUT=0.5:timeout \
	two-files
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

.PHONY: test
test: $(TEST_LIST)
	@SEP=""; \
	declare -a tests; \
	tests=( $(TESTS) ); \
	for test in "$${tests[@]}"; do \
		printf "$$SEP"; \
		TEST_NAME=$$(echo "$$test" | sed 's/^.*://'); \
		TEST_CMD=$$(echo "$$test" | \
			sed \
				-e 's/^[^:]*$$/$(BIN)\/&/' \
				-e 's/^\(.*\):/\1 $(BIN)\//g' \
			); \
		running_string="Running test $$TEST_NAME"; \
		echo "$$running_string"; \
        echo "$$running_string" | sed 's/./=/g'; \
		if eval "$$TEST_CMD | \
			test/parse-test.sh $$TEST_NAME $(ACTUAL)"; then \
			echo "No failures found"; \
		else \
			echo "Failures were found"; \
		fi; \
		SEP='\n'; \
	done; \
	echo "Done testing"
	@echo "Starting comparison"; \
	canonicals=$$(ls $(CANONICAL) | xargs -n1 basename); \
	actuals=$$(ls $(ACTUAL) | xargs -n1 basename); \
	n_canonicals="$$(echo "$$canonicals" | wc -w)"; \
    n_actuals="$$(echo "$$actuals" | wc -w)"; \
	if [ $$n_canonicals -ne $$n_actuals ]; then \
	    echo "Different number of canonical and actual files" 1>&2; \
	    exit 1; \
	fi; \
	errors=0; \
	for file in $$canonicals; do \
		actual=$(ACTUAL)/$$file; \
		canonical=$(CANONICAL)/$$file; \
		echo "Compare expected output with actual in $$actual"; \
		diff $$canonical $$actual; \
		if [ $$? -ne 0 ]; then \
			errors=$$((errors + 1)); \
		fi; \
	done; \
	if [ $$errors -eq 0 ]; then \
		echo "Tests PASSED"; \
	else \
		echo "Tests FAILED"; \
		exit 1; \
	fi

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

$(BIN)/skip: $(BIN)/skip.o $(LIMPET_HDRS)
	$(CC) $(CPPFLAGS) -o $@ $(filter-out %.h,$^) $(LDFLAGS)

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
