/*
 * Test for limpet
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <limpet.h>

static const int sleep_time = 2;

int main(int argc, char *argv[]) {
    fprintf(stderr, "Should never get to main()\n");
    exit(EXIT_FAILURE);
}

#ifdef LIMPET
static void x(const char *name) {
    printf("This is printed by test %s\n", name);
    printf("Two messages should print after a %d second delay, the third\n"
        "after an additional %d second delay\n", sleep_time, sleep_time);
    sleep(sleep_time);
}

LIMPET_TEST(max_jobs1) {
    x(__func__);
}

LIMPET_TEST(max_jobs2) {
    x(__func__);
}

LIMPET_TEST(max_jobs3) {
    x(__func__);
}
#endif /* LIMPET */
