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
LIMPET_TEST(skip1) {
    printf("This is printed by test skip1\n");
    printf("You should see this message\n");
}

LIMPET_TEST(skip2) {
    printf("This is printed by test skip2\n");
    printf("You should not see this message\n");
}

LIMPET_TEST(skip3) {
    printf("This is printed by test skip3\n");
    printf("You should see this message\n");
}

LIMPET_TEST(skip4) {
    printf("This is printed by test skip4\n");
    printf("You should not see this message\n");
}
#endif /* LIMPET */
