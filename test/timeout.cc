/*
 * Test for limpet
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
/*
 * Set the time out value to 0.5 seconds
 */
LIMPET_TEST(timeout) {
    const int sleep_time = 2;
    printf("This is printed by test tiemout\n");
    printf("Sleeping for %d seconds\n", sleep_time);
    sleep(sleep_time); 
}
#endif /* LIMPET */
