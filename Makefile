CPPFLAGS :=
CPPFLAGS += $(CPPFLAGS) -g -ggdb
CPPFLAGS += -Wall -Wextra -Werror
CPPFLAGS += -Wno-unused-parameter
CPPFLAGS += -I.
CPPFLAGS += -DLEAVEMEIN

test: leavemein
	./leavemein

leavemein: leavemein.cc leavemein.h leavemein-linux.h
	clear
	c++ $(CPPFLAGS) -o leavemein leavemein.cc $(LDFLAGS)

.PHONY: clean
clean:
	rm -f leavemein
