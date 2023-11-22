/*
 * Test for two-files, main()
 */

#include <stdio.h>
#include <stdlib.h>

#include <limpet.h>

int main(int argc, char *argv[]) {
    fprintf(stderr, "Should never get to main()");
    exit(EXIT_FAILURE);
}

#ifdef LIMPET
LIMPET_TEST(two_files1) {
    limpet_assert_eq(0, 0);
}
#endif /* LIMPET */
