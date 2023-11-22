/*
 * Test for correct printing of signals
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
LEAVEMEIN_TEST(sigsegv) {
    printf("This is printed by test %s\n", __func__);
    printf("You should not see this %d\n", *(int *)NULL);
}

LEAVEMEIN_TEST(abrt) {
    printf("This is printed byt test %s\n", __func__);
    abort();
}

#endif /* LEAVEMEIN */
