/*
 * Test for leavemein
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <leavemein.h>

int main(int argc, char *argv[]) {
    fprintf(stderr, "Should never get to main()");
    exit(EXIT_FAILURE);
}

#ifdef LEAVEMEIN
LEAVEMEIN_TEST(simple_good) {
    write(1, "good\n", 5);
    leavemein_assert_eq(0, 0);
}

LEAVEMEIN_TEST(simple_bad) {
    write(1, "bad\n", 4);
    leavemein_assert_eq(0, 1);
}

#endif /* LEAVEMEIN */
