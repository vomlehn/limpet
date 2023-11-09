/*
 * Test for two-files, main()
 */

#include <stdio.h>
#include <stdlib.h>

#include <leavemein.h>

int main(int argc, char *argv[]) {
    fprintf(stderr, "Should never get to main()");
    exit(EXIT_FAILURE);
}

#ifdef LEAVEMEIN
LEAVEMEIN_TEST(two_files1) {
    leavemein_assert_eq(0, 0);
}
#endif /* LEAVEMEIN */
