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
/*
 * Set the time out value to 0.5 seconds
 */
LEAVEMEIN_TEST(timeout) {
    const int sleep_time = 2;
    printf("This is printed by test tiemout\n");
    printf("Sleeping for %d seconds\n", sleep_time);
    sleep(sleep_time); 
}
#endif /* LEAVEMEIN */
