# Make file for building and testing Limpet

SHELL = /bin/bash

# Supported versions are:
# LINUX
# SINGLE_THREADED

VERSION=LINUX

BIN=bin
SRC=src

# Sent to "c" to compiler for C, "cc" to compiler for C++
LANG = c

ifeq "$(LANG)" "cc"
SFX = cc
else ifeq "$(LANG)" "c"
SFX = c
else
$(error LANG must be set to "cc" or "c")
endif

# Flags for various versions
INCS_LINUX = include/limpet-linux.h
CPPFLAGS_LINUX = -DLIMPET=LIMPET_LINUX
TESTS_LINUX = LIMPET_MAX_JOBS="2":maxjobs

INCS_SINGLE_THREADED = include/limpet-single-threaded.h
CPPFLAGS_SINGLE_THREADED = -DLIMPET=LIMPET_SINGLE_THREADED
TESTS_LINUX = LIMPET_MAX_JOBS="1":maxjobs

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
	"LIMPET_RUNLIST=\"skip1 skip3\"":skip \
	"LIMPET_RUNLIST=\"\"":skip \
	LIMPET_TIMEOUT=0.5:timeout \
	two-files
TESTS = $(COMMON_TESTS) $(TESTS_$(VERSION))

ECHO_TESTS = declare -a tests; \
	tests=($(TESTS)); \
    echo "$${tests[@]}"; \
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
		if eval $$TEST_CMD; then \
			echo "No failures found"; \
		else \
			echo "Failures were found"; \
		fi; \
		SEP='\n'; \
	done

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
	rm -f bin/*
