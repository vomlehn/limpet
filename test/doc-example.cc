    /*
     * Example use of Limpet included in README file
     *
     * To compile, use:
     *
     *      cc  -Iinclude -DLIMPET=LIMPET_LINUX -o simple test/simple.cc
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
    LIMPET_TEST(simple_good) {
        printf("doc-example: This is printed by test simple_good\n");
        limpet_assert_eq(0, 0);
    }

    LIMPET_TEST(simple_bad) {
        printf("doc-example: This is printed by test simple_bad\n");
        limpet_assert_ne(0, 0);
    }

    #endif /* LIMPET */
