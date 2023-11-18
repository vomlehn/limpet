/*
 * Test for leavemein
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <leavemein.h>

static const int sleep_time = 2;

int main(int argc, char *argv[]) {
    fprintf(stderr, "Should never get to main()");
    exit(EXIT_FAILURE);
}

#ifdef LEAVEMEIN
static void x(const char *name) {
    printf("This is printed by test %s\n", name);
    printf("Two messages should print after a %d second delay, the third\n"
        "after an additional %d second delay\n", sleep_time, sleep_time);
    sleep(sleep_time);
}

LEAVEMEIN_TEST(max_jobs1) {
    x(__func__);
}

LEAVEMEIN_TEST(max_jobs2) {
    x(__func__);
}

LEAVEMEIN_TEST(max_jobs3) {
    x(__func__);
}
#endif /* LEAVEMEIN */
