/*
 * Test for correct printing of signals
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <limpet.h>

int main(int argc, char *argv[]) {
    fprintf(stderr, "Should never get to main()");
    exit(EXIT_FAILURE);
}

#ifdef LIMPET
LIMPET_TEST(sigsegv) {
    printf("This is printed by test %s\n", __func__);
    printf("You should not see this %d\n", *(int *)NULL);
}

LIMPET_TEST(abrt) {
    printf("This is printed byt test %s\n", __func__);
    abort();
}

#endif /* LIMPET */
