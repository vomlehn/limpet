/*
 * Test for limpet
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <limpet.h>

int main(int argc, char *argv[]) {
    fprintf(stderr, "Should never get to main()\n");
    exit(EXIT_FAILURE);
}

#ifdef LIMPET
LIMPET_TEST(simple_good) {
    printf("This is printed by test simple_good\n");
    limpet_assert_eq(0, 0);
}

LIMPET_TEST(simple_bad) {
    printf("This is printed by test simple_bad\n");
    limpet_assert_eq(0, 1);
}

#endif /* LIMPET */
