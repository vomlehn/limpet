/*
 * Code allowing embedding of tests within C/C++ code
 */
#ifndef _LEAVEIN_H_
#define _LEAVEIN_H_

#ifdef LEAVEMEIN
#include <unistd.h>

#include <leavemein-linux.h>

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
        static struct __leavemein_test common = {           \
            .next = NULL,                                   \
            .name = #testname,                              \
            .func = testname,                               \
            .params = &__leavemein_params,                  \
            .sysdep = __LEAVEMEIN_SYSDEP_INIT,              \
        };                                                  \
        __leavemein_enqueue_test(&common);                  \
    }                                                       \
    void testname(void)

#define leavemein_assert_eq(a, b)    \
    do { if ((a) != (b)) __leavemein_exit(true); } while (0)
#define leavemein_assert_ne(a, b)    \
    do { if ((a) != (b)) __leavemein_exit(true); } while (0)
    
/*
 * These definitions are intended for internal use by the leavemein code
 * ===================================================================
 */

/*
 * Parsed parameter information
 */
struct __leavemein_params __leavemein_params __attribute((common));

/*
 * __leavemein_list - The linked list of all tests by the per-test
 *      constructors. Tests are added during the test discovery phase by
 *      single-threaded code and removed during the text execution phase. We're
 *      generally multi-threaded then, but only one single thread function
 *      removes tests. So, no mutual exclusion needed.
 */
struct __leavemein_test *__leavemein_list __attribute((common));

/*
 * Statistics-related items
 */
unsigned __leavemein_started __attribute((common));
unsigned __leavemein_total __attribute((common));
unsigned __leavemein_passed __attribute((common));
unsigned __leavemein_failed __attribute((common));
unsigned __leavemein_skipped __attribute((common));
struct __leavemein_mutex __leavemein_statistics_mutex __attribute((common));
struct __leavemein_cond __leavemein_statistics_cond __attribute((common));

/*
 * Prototypes for system-dependent common functions
 */
static void __leavemein_statistics_inc(unsigned *item) {
    __leavemein_mutex_lock(&__leavemein_statistics_mutex);
    (*item)++;
    __leavemein_total++;
    __leavemein_cond_signal(&__leavemein_statistics_cond);
    __leavemein_mutex_unlock(&__leavemein_statistics_mutex);
}

static void __leavemein_inc_started(void) {
    __leavemein_mutex_lock(&__leavemein_statistics_mutex);
    __leavemein_started++;
    __leavemein_cond_signal(&__leavemein_statistics_cond);
    __leavemein_mutex_unlock(&__leavemein_statistics_mutex);
}

static void __leavemein_inc_passed(void) {
    __leavemein_statistics_inc(&__leavemein_passed);
}

static void __leavemein_inc_failed(void) {
    __leavemein_statistics_inc(&__leavemein_failed);
}

static void __leavemein_inc_skipped(void) {
    __leavemein_statistics_inc(&__leavemein_skipped);
}

static void __leavemein_wait_pending(unsigned pending) {
    __leavemein_mutex_lock(&__leavemein_statistics_mutex);

    while (__leavemein_started - __leavemein_total > pending) {
        __leavemein_cond_wait(&__leavemein_statistics_cond,
            &__leavemein_statistics_mutex);
    }

    __leavemein_mutex_unlock(&__leavemein_statistics_mutex);
}

/*
 * Called by per-test constructor to add the test to the test list. This
 * is called in a single threaded context.
 */
static void __leavemein_enqueue_test(struct __leavemein_test *test) {
    test->next = __leavemein_list;
    __leavemein_list = test;
}

/*
 * Add a test to the list of those to report. Called in a single threaded
 * context.
 */
static struct __leavemein_test *
__leavemein_first_test(void) {
    return __leavemein_list;
}

/*
 * Returns the next test in the list. Called in a single threaded context.
 */
static struct __leavemein_test *
__leavemein_next_test(struct __leavemein_test * test) {
    return test->next;
}

static bool __leavemein_must_skip(const char *name) {
    size_t i;

    if (__leavemein_params.n_skiplist == 0) {
        if (__leavemein_params.skiplist == NULL) {
            return false;
        } else {
            return true;
        }
    }

    for (i = 0; i < __leavemein_params.n_skiplist; i++) {
        if (strcmp(name, __leavemein_params.skiplist[i]) == 0) {
            return true;
        }
    }

    return false;
}

static void __leavemein_report(struct __leavemein_test *test, const char *sep) {
    printf("%s", sep);
    __leavemein_printf("> Log for %s\n", test->name);

    if (__leavemein_dump_log(test) == 0) {
        __leavemein_printf("<empty log>\n");
    }

    __leavemein_printf("> Test complete: %s ", test->name);
    __leavemein_print_status(test);
    __leavemein_printf("\n");
}

/*
 * This is the function that runs all the tests in the file including this
 * header file. There will actually be one of these in each file #including
 * leavemein.h, but since they are identical, any of them can be run. Only
 * one of them will be run because the first one exits the process.
 */
static void __leavemein_run(void) \
    __attribute((constructor(__LEAVEMEIN_RUN_PRI)));
static void __leavemein_run(void) {
    struct __leavemein_test *p;
    const char *sep;

    __leavemein_mutex_init(&__leavemein_statistics_mutex);
    __leavemein_cond_init(&__leavemein_statistics_cond);
    __leavemein_parse_params(&__leavemein_params);

    for (p = __leavemein_first_test(); p != NULL;
        p = __leavemein_next_test(p)) {
        if (__leavemein_must_skip(p->name)) {
            __leavemein_inc_skipped();
            continue;
        }

        /*
         * If we have a maximum number of concurrent jobs, wait until the
         * number of pending tests is less than or equal to that
         */
        if (__leavemein_params.max_jobs != 0) {
            __leavemein_wait_pending(__leavemein_params.max_jobs);
        }
        
        __leavemein_start_one(p);
        __leavemein_inc_started();
    }

    /*
     * All tests have been started. We wait for everything to finish before
     * printing reports to make sure than any unexpected errors are printed
     * before the reports, and not lost in the clutter.
     */
    for (p = __leavemein_first_test(); p != NULL;
        p = __leavemein_next_test(p)) {
        __leavemein_cleanup_test(p);
    }

    /*
     * Report on the results
     */
    sep = "";
    for (p = __leavemein_first_test(); p != NULL;
        p = __leavemein_next_test(p)) {
        __leavemein_report(p, sep);
        sep = "\n";
    }

    printf("%s> Ran %u tests: %u passed %u failed %u skipped\n", sep,
        __leavemein_total, __leavemein_passed, __leavemein_failed,
        __leavemein_skipped);
    __leavemein_exit(false);
}
#endif /* LEAVEMEIN */
#endif /* _LEAVEIN_H_ */
