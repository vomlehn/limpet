CPPFLAGS :=
CPPFLAGS += $(CPPFLAGS) -g -ggdb
CPPFLAGS += -Wall -Wextra -Werror
CPPFLAGS += -Wno-unused-parameter
CPPFLAGS += -Iinclude
CPPFLAGS += -DLEAVEMEIN=LEAVEMEIN_LINUX

TESTS :=
TESTS += ./simple
TESTS += ./two-files

SIMPLE_TEST= for test in $(TESTS); do \
		printf "$$sep"; \
		echo "$$test"; \
		if $$test; then \
			echo "No failures found"; \
		else \
			echo "Failures were found"; \
		fi; \
		sep="\n"; \
	done
TIMEOUT_TEST= echo "./timeout"; \
	if LEAVEMEIN_TIMEOUT=0.5 ./timeout; then \
		echo "No failures found"; \
	else \
		echo "Failures were found"; \
	fi
SKIP2_TEST = echo "./skip"; \
	if LEAVEMEIN_RUNLIST="skip1 skip3" ./skip; then \
		echo "No failures found"; \
	else \
		echo "Failures were found"; \
	fi
SKIP_ALL_TEST= echo "./skip"; \
	if LEAVEMEIN_RUNLIST="" ./skip; then \
		echo "No failures found"; \
	else \
		echo "Failures were found"; \
	fi
MAXJOBS_TEST = echo "./maxjobs"; \
	if LEAVEMEIN_MAX_JOBS="2" ./maxjobs; then \
		echo "No failures found"; \
	else \
		echo "Failures were found"; \
	fi
SIGNAL_TEST = echo "./signal"; \
	if ./signal; then \
		echo "No failures found"; \
	else \
		echo "Failures were found"; \
	fi

.PHONY: test
test: simple two-files timeout skip maxjobs signal
	sep=""; \
    $(SIMPLE_TEST); \
	sep="\n"; \
    $(TIMEOUT_TEST); \
	sep="\n"; \
    $(SKIP2_TEST); \
	sep="\n"; \
    $(SKIP_ALL_TEST); \
	sep="\n"; \
    $(MAXJOBS_TEST); \
	sep="\n"; \
    $(SIGNAL_TEST); \
	sep="\n"; \
	echo "Done"

simple: simple.o include/leavemein.h include/leavemein-linux.h
	$(CC) $(CPPFLAGS) -o simple simple.o $(LDFLAGS)

simple.o: test/simple.cc \
	include/leavemein.h include/leavemein-linux.h
	$(CC) $(CPPFLAGS) -c -o simple.o test/simple.cc

two-files: two-files-main.o two-files-sub.o
	$(CC) $(CPPFLAGS) -o two-files two-files-main.o two-files-sub.o $(LDFLAGS)

two-files-main.o: test/two-files-main.cc \
	include/leavemein.h include/leavemein-linux.h
	$(CC) $(CPPFLAGS) -c -o two-files-main.o test/two-files-main.cc

two-files-sub.o: test/two-files-sub.cc \
	include/leavemein.h include/leavemein-linux.h
	$(CC) $(CPPFLAGS) -c -o two-files-sub.o test/two-files-sub.cc

timeout: timeout.o include/leavemein.h include/leavemein-linux.h
	$(CC) $(CPPFLAGS) -o timeout timeout.o $(LDFLAGS)

timeout.o: test/timeout.cc \
	include/leavemein.h include/leavemein-linux.h
	$(CC) $(CPPFLAGS) -c -o timeout.o test/timeout.cc

skip: skip.o include/leavemein.h include/leavemein-linux.h
	$(CC) $(CPPFLAGS) -o skip skip.o $(LDFLAGS)

skip.o: test/skip.cc \
	include/leavemein.h include/leavemein-linux.h
	$(CC) $(CPPFLAGS) -c -o skip.o test/skip.cc

maxjobs: maxjobs.o include/leavemein.h include/leavemein-linux.h
	$(CC) $(CPPFLAGS) -o maxjobs maxjobs.o $(LDFLAGS)

maxjobs.o: test/maxjobs.cc \
	include/leavemein.h include/leavemein-linux.h
	$(CC) $(CPPFLAGS) -c -o maxjobs.o test/maxjobs.cc

signal: signal.o include/leavemein.h include/leavemein-linux.h
	$(CC) $(CPPFLAGS) -o signal signal.o $(LDFLAGS)

signal.o: test/signal.cc \
	include/leavemein.h include/leavemein-linux.h
	$(CC) $(CPPFLAGS) -c -o signal.o test/signal.cc

.PHONY: clean
clean:
	rm -f simple simple.o \
        two-files two-files-main.o two-files-sub.o \
        timeout timeout.o \
        skip skip,o \
        maxjobs maxjobs.o \
        signal signal.o

    
