#ifndef _LEAVEIN_
#define _LEAVEIN_

#ifdef LEAVEMEIN
#include <stdio.h>          // FIXME: debugging, remove
#include <unistd.h>

#include <leavemein-linux.h>

/*
 * Definitions common to all implementations
 *  name - Name of the test
 *  func - Function to execute the test
 *  next - Next item on the list of tests, or NULL at the end.
 *  sysdep - System-dependent information
 */
struct __leavemein_common {
    struct __leavemein_common   *next;
    const char *                name;
    void                        (*func)(void);
    struct __leavemein_sysdep   *sysdep;
};

/*
 * These definitions are intended for use by the user code
 * =======================================================
 */

/*
 * The tests use two constructor priorities, which can be overridden
 *  __LEAVEMEIN_SETUP_PRI - prority to run the constructors that set up each
 *      test for execution. This priority must cause these constructors to
 *      run before those running at __LEAVEMEIN_RUN_PRI.
 *  __LEAVEMEIN_RUN_PRI - run all tests
 * These will generally be the highest and second highest constructor
 * priorities. If necessary, these can be overridden  by defining one or
 * both before #including this file:
 */
// FIXME: verify these values
#ifndef __LEAVEMEIN_SETUP_PRI
#define __LEAVEMEIN_SETUP_PRI   148
#endif

#ifndef __LEAVEMEIN_RUN_PRI
#define __LEAVEMEIN_RUN_PRI     149
#endif

/*
 * Used to define a test. Usage:
 *  LEAVEIN(testname) {
 *      <test body>
 *  }
 *
 * This actually defines two functions. The first function has the given
 * testnane, prefixed by __leavemein_, denoted by __leavemein_<testname>. This
 * function is called as a highest priority constructor. The second function
 * is the user's test function, with the name given in testname.
 *
 * __leavemein_<testname> is responsible for setting up anything required by
 * the user's test and then linking it into a list for later processing.
 */
#define LEAVEMEIN_TEST(testname) \
    static void __leavemein_test_ ## testname(void)         \
        __attribute((constructor(__LEAVEMEIN_SETUP_PRI)));  \
    static void testname(void);                             \
    static void __leavemein_test_ ## testname(void) {       \
        static struct __leavemein_common common = {         \
            .next = NULL,                               \
            .name = #testname,                          \
            .func = testname,                           \
            .sysdep = {0},                              \
        };                                              \
        printf("Test %s\n", #testname);                 \
        __leavemein_add_test(&common);                      \
    }                                                   \
    void testname(void)

#define leavemein_assert_eq(a, b)    \
    do { if ((a) != (b)) __leavemein_test_exit(true); } while (0)
    
/*
 * These definitions are intended for internal use by the leavemein code
 * ===================================================================
 */

/*
 * This is the linked list of all tests by the per-test constructors
 */
struct __leavemein_common *__leavemein_list __attribute((common));

/*
 * Prototypes for system-dependent common functions
 */
static void __leavemein_update_status(bool is_error);

/*
 * Called by per-test constructor to add the test to the test list
 */
static void __leavemein_add_test(struct __leavemein_common *test) {
    test->next = __leavemein_list;
    __leavemein_list = test;
}

/*
 * This is the function that runs all the tests in the file including this
 * header file.
 */
static void __leavemein_run(void) \
    __attribute((constructor(__LEAVEMEIN_RUN_PRI)));
static void __leavemein_run(void) {
    struct __leavemein_common *p;

printf("start run\n");
    /* printf("start test execution"); */
    for (p = __leavemein_list; p != NULL; p = p->next) {
        printf("executing %s\n", p->name);
        __leavemein_run_one(p);
    }
    __leavemein_run_exit(false);
}
#endif /* LEAVEMEIN */
#endif /* _LEAVEIN_ */
