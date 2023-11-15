CPPFLAGS :=
CPPFLAGS += $(CPPFLAGS) -g -ggdb
CPPFLAGS += -Wall -Wextra -Werror
CPPFLAGS += -Wno-unused-parameter
CPPFLAGS += -Iinclude
CPPFLAGS += -DLEAVEMEIN

.PHONY: test
test: simple two-files
	@sep=""; for test in ./simple ./two-files; do \
		printf "$$sep"; \
        echo "$$test"; \
		$$test; \
		sep="\n"; \
	done

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

.PHONY: clean
clean:
	rm -f simple two-files two-files-main.o two-files-sub.o
