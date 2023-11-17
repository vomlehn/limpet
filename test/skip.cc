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
LEAVEMEIN_TEST(skip1) {
    printf("This is printed by test skip1\n");
    printf("You should see this message\n");
}

LEAVEMEIN_TEST(skip2) {
    printf("This is printed by test skip2\n");
    printf("You should not see this message\n");
}

LEAVEMEIN_TEST(skip3) {
    printf("This is printed by test skip3\n");
    printf("You should see this message\n");
}

LEAVEMEIN_TEST(skip4) {
    printf("This is printed by test skip4\n");
    printf("You should not see this message\n");
}
#endif /* LEAVEMEIN */
