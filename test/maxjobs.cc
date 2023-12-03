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
    printf("One message will print immediately. As many more as allowed\n");
    printf("by MAX_JOBS will also print immediately. Then other messages\n");
    printf("will print after a delay of %d seconds\n", sleep_time);
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
