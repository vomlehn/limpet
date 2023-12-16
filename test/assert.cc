/*
 * Test for limpet: check out comparisons
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include <limpet.h>

int main(int argc, char *argv[]) {
    fprintf(stderr, "Should never get to main()\n");
    exit(EXIT_FAILURE);
}

#ifdef LIMPET
LIMPET_TEST(assert_success) {
    printf("This is printed by test assert_success\n");
    limpet_assert(true);
}

LIMPET_TEST(assert_failure) {
    printf("This is printed by test assert_failure\n");
    limpet_assert(0);
}

LIMPET_TEST(eq_success) {
    printf("This is printed by test eq_success\n");
    limpet_assert_eq(0, 0);
}

LIMPET_TEST(eq_failure) {
    printf("This is printed by test eq_failure\n");
    limpet_assert_eq(0, 1);
}

LIMPET_TEST(ne_success) {
    printf("This is printed by test ne_success\n");
    limpet_assert_ne(0, 1);
}

LIMPET_TEST(ne_failure) {
    printf("This is printed by test ne_failure\n");
    limpet_assert_ne(0, 0);
}

LIMPET_TEST(gt_success) {
    printf("This is printed by test gt_success\n");
    limpet_assert_gt(1, 0);
}

LIMPET_TEST(gt_failure) {
    printf("This is printed by test gt_failure\n");
    limpet_assert_gt(0, 1);
}

LIMPET_TEST(ge_success) {
    printf("This is printed by test ge_success\n");
    limpet_assert_ge(0, 0);
}

LIMPET_TEST(ge_failure) {
    printf("This is printed by test ge_failure\n");
    limpet_assert_ge(0, 1);
}

LIMPET_TEST(lt_success) {
    printf("This is printed by test lt_success\n");
    limpet_assert_lt(0, 1);
}

LIMPET_TEST(lt_failure) {
    printf("This is printed by test lt_failure\n");
    limpet_assert_lt(0, 0);
}

LIMPET_TEST(le_success) {
    printf("This is printed by test le_success\n");
    limpet_assert_le(0, 0);
}

LIMPET_TEST(le_failure) {
    printf("This is printed by test le_failure\n");
    limpet_assert_le(1, 0);
}

#endif /* LIMPET */
