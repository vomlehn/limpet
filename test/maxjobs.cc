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
LEAVEMEIN_TEST(max_jobs1) {
    printf("This is printed by test max_jobs1\n");
    printf("You should see this message immediately\n");
    sleep(2);
}

LEAVEMEIN_TEST(max_jobs2) {
    printf("This is printed by test max_jobs2\n");
    printf("You should see this message immediately\n");
    sleep(2);
}

LEAVEMEIN_TEST(max_jobs3) {
    printf("This is printed by test max_jobs3\n");
    printf("You should see this message 2 seconds after max_jobs1 and max_jobs2"
        "finish\n");
}
#endif /* LEAVEMEIN */
