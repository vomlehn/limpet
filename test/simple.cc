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
    printf("Running simple_good\n");
    leavemein_assert_eq(0, 0);
}

LEAVEMEIN_TEST(simple_bad) {
    printf("Running simple_bad\n");
    leavemein_assert_eq(0, 1);
}

#endif /* LEAVEMEIN */
