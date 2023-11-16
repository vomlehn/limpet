CPPFLAGS :=
CPPFLAGS += $(CPPFLAGS) -g -ggdb
CPPFLAGS += -Wall -Wextra -Werror
CPPFLAGS += -Wno-unused-parameter
CPPFLAGS += -Iinclude
CPPFLAGS += -DLEAVEMEIN

TESTS :=
TESTS += ./simple
TESTS += ./two-files

.PHONY: test
test: simple two-files timeout
	sep=""; \
    for test in $(TESTS); do \
		printf "$$sep"; \
		echo "$$test"; \
		if $$test; then \
            echo "No failures found"; \
        else \
            echo "Failures were found"; \
        fi; \
		sep="\n"; \
	done; \
	if LEAVEMEIN_TIMEOUT=0.5 ./timeout; then \
        echo "No failures found"; \
    else \
        echo "Failures were found"; \
    fi; \
    sep="\n"; \
	unset LEAVEMEIN_TIMEOUT; \
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

.PHONY: clean
clean:
	rm -f simple two-files two-files-main.o two-files-sub.o
