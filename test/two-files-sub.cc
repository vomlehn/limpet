/*
 * Test for two-files, main()
 */

#include <stdio.h>
#include <stdlib.h>

#include <leavemein.h>

#ifdef LEAVEMEIN
LEAVEMEIN_TEST(two_files2) {
    leavemein_assert_eq(0, 0);
}
#endif /* LEAVEMEIN */

