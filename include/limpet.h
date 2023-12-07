/*
 * Code allowing embedding of tests within C/C++ code
 */

#ifndef _LEAVEIN_H_
#define _LEAVEIN_H_

#ifdef LIMPET

#define LIMPET_LINUX                    2
#define LIMPET_SINGLE_THREADED_LINUX    3

#if LIMPET == LIMPET_LINUX
#include "limpet.d/limpet-linux.h"
#elif LIMPET == LIMPET_SINGLE_THREADED_LINUX
#include "limpet.d/limpet-single-threaded-linux.h"
#else
#error LIMPET is not supported
#endif

/*
 * These definitions are intended for use by the user code
 * =======================================================
 */

/*
 * The tests use two constructor priorities, which can be overridden
 *  __LIMPET_SETUP_PRI - prority to run the constructors that set up each
 *      test for execution. This priority must cause these constructors to
 *      run before those running at __LIMPET_RUN_PRI.
 *  __LIMPET_RUN_PRI - run all tests
 * These will generally be the highest and second highest constructor
 * priorities. If necessary, these can be overridden  by defining one or
 * both before #including this file:
 */
#ifndef __LIMPET_SETUP_PRI
#define __LIMPET_SETUP_PRI   101
#endif

#ifndef __LIMPET_RUN_PRI
#define __LIMPET_RUN_PRI     102
#endif

/*
 * Used to define a test. Usage:
 *  LEAVEIN(testname) {
 *      <test body>
 *  }
 *
 * This actually defines two functions. The first function has the given
 * testnane, prefixed by __limpet_, denoted by __limpet_<testname>.
 * This function is called as a highest priority constructor. The second
 * function is the user's test function, with the name given in testname.
 *
 * __limpet_<testname> is responsible for setting up anything required by
 * the user's test and then linking it into a list for later processing.
 */
#define LIMPET_TEST(testname) \
    static void __limpet_test_ ## testname(void)            \
        __attribute((constructor(__LIMPET_SETUP_PRI)));     \
    static void testname(void);                             \
    static void __limpet_test_ ## testname(void) {          \
        static struct __limpet_test common = {              \
            .next = NULL,                                   \
            .done = NULL,                                   \
            .skipped = false,                               \
            .name = #testname,                              \
            .func = testname,                               \
            .params = &__limpet_params,                     \
            .sysdep = __LIMPET_SYSDEP_INIT,                 \
        };                                                  \
        __limpet_enqueue_test(&common);                     \
    }                                                       \
    void testname(void)

#define limpet_assert_eq(a, b)    \
    do { if (!((a) == (b))) __limpet_exit(true); } while (0)
#define limpet_assert_ne(a, b)    \
    do { if (!((a) != (b))) __limpet_exit(true); } while (0)
#define limpet_assert_gt(a, b)    \
    do { if (!((a) > (b))) __limpet_exit(true); } while (0)
#define limpet_assert_ge(a, b)    \
    do { if (!((a) >= (b))) __limpet_exit(true); } while (0)
#define limpet_assert_lt(a, b)    \
    do { if (!((a) < (b))) __limpet_exit(true); } while (0)
#define limpet_assert_le(a, b)    \
    do { if (!((a) <= (b))) __limpet_exit(true); } while (0)
    
/*
 * These definitions are intended for internal use by the limpet code
 * ===================================================================
 */

/*
 * String used to separate reports
 */
#define __LIMPET_REPORT_SEP      "---\n"

/*
 * String used before printing limpet output.
 */
#define __LIMPET_MARKER "> "

/*
 * Add a test name to the run list
 * start - pointer to the beginning of the test name
 * end - pointer one past the end of the test name
 * params - pointer to the structure storing the configuration
 *
 * Returns: true if successful, false otherwise.
 */
static void __limpet_add_testname(struct __limpet_params *params,
    const char *start, const char *end) {
    size_t testname_size;
    char *testname;
    size_t new_runlist_size;

    testname_size = end - start;
    testname = (char *)malloc(testname_size);
    if (testname == NULL) {
        __limpet_fail("Out of memory allocating %zu bytes for test name\n",
            testname_size);
    }

    strncpy(testname, start, testname_size);

    new_runlist_size = sizeof(params->runlist[0]) * (params->n_runlist + 1);
    params->runlist = (char **)realloc(params->runlist, new_runlist_size);
    if (params->runlist == NULL) {
        __limpet_fail("Unable to realloc %zu bytes\n", new_runlist_size);
    }

    params->runlist[params->n_runlist] = testname;
    params->n_runlist += 1;
}

/*
 * Parse the runlist, if any
 * params - Pointer to the configuration structure
 *
 * Returns: true on success, false otherwise
 */
static bool __limpet_parse_runlist(struct __limpet_params *params) {
    const char *runlist_env;

    runlist_env = __limpet_get_runlist();

    if (runlist_env != NULL) {
        const char *start;
        const char *space;

        params->runlist = NULL;
        start = runlist_env;

        for (space = strchr(start, ' '); space != NULL;
            space = strchr(start, ' ')) {

            if (start == space) {
                __limpet_fail("Zero length test name invalid in %s\n",
                    runlist_env);
            }

            __limpet_add_testname(params, start, space);
            start = space + 1;
        }

        if (*start != '\0') {
            size_t name_len;

            name_len = strlen(start);
            __limpet_add_testname(params, start, start + name_len);
        }
    }

    return true;
}

/*
 * Parse environment variables to get the configuration
 * params - pointer to the structure storing the configuration
 *
 * Returns: true on success, false otherwise.
 */
static bool __limpet_parse_params(struct __limpet_params *params) {
    const char *max_jobs_env;
    const char *timeout;
    const char *verbose_env;

    memset(params, 0, sizeof(*params));

    max_jobs_env = __limpet_get_maxjobs();

    if (max_jobs_env != NULL) {
        char *endptr;
        params->max_jobs = strtoul(max_jobs_env, &endptr, 0);
        if (*max_jobs_env == '\0' || *endptr != '\0') {
            __limpet_fail("Invalid value specified for %s\n",
                max_jobs_env);
        }
    }

    if (!__limpet_parse_runlist(params)) {
        return false;
    }

    timeout = __limpet_get_timeout();

    if (timeout == NULL) {
        params->timeout = __LIMPET_DEFAULT_TIMEOUT;
    } else {
        char *endptr;

        params->timeout = strtof(timeout, &endptr);

        if (*timeout == '\0' || *endptr != '\0') {
            __limpet_fail("Bad timeout value\n", timeout);
        }
    }

    verbose_env = __limpet_get_verbose();
    if (verbose_env != NULL) {
        if (strcmp(verbose_env, "true") == 0) {
            params->verbose = true;
        } else if (strcmp(verbose_env, "false") == 0) {
            params->verbose = false;
        } else {
            __limpet_fail("VERBOSE must be true or false\n");
        }
    }

    /*
     * Clean up the environment
     */
    __limpet_parse_done();

    return true;
}

/*
 * __limpet_list - The linked list of all tests by the per-test
 *      constructors. Tests are added during the test discovery phase by
 *      single-threaded code and removed during the text execution phase.
 *      We're generally multi-threaded then, but only one single thread
 *      function removes tests. So, no mutual exclusion needed.
 */
struct __limpet_test *__limpet_list __attribute((common));
struct __limpet_test *__limpet_done __attribute((common));
struct __limpet_mutex __limpet_done_mutex __attribute((common));
struct __limpet_cond __limpet_done_cond __attribute((common));

/*
 * Add a test to the list of completed tests
 */
static void __limpet_enqueue_done(struct __limpet_test *test) {
    __limpet_mutex_lock(&__limpet_done_mutex);

    test->done = __limpet_done;
    __limpet_done = test;

    __limpet_cond_signal(&__limpet_done_cond);
    __limpet_mutex_unlock(&__limpet_done_mutex);
}

/*
 * Wait until a test completes, then remove it from the list and return it
 */
static struct __limpet_test *__limpet_dequeue_done(void) {
    struct __limpet_test *test;

    __limpet_mutex_lock(&__limpet_done_mutex);

    while (__limpet_done == NULL) {
        __limpet_cond_wait(&__limpet_done_cond,
            &__limpet_done_mutex);
    }

    test = __limpet_done;
    __limpet_done = test->done;

    __limpet_mutex_unlock(&__limpet_done_mutex);

    return test;
}

/*
 * Statistics-related items
 */
unsigned __limpet_started __attribute((common));
unsigned __limpet_passed __attribute((common));
unsigned __limpet_failed __attribute((common));
unsigned __limpet_skipped __attribute((common));
struct __limpet_mutex __limpet_statistics_mutex __attribute((common));
struct __limpet_cond __limpet_statistics_cond __attribute((common));

/*
 * Prototypes for system-dependent common functions
 */
static void __limpet_statistics_inc(unsigned *item) {
    __limpet_mutex_lock(&__limpet_statistics_mutex);
    (*item)++;
    __limpet_cond_signal(&__limpet_statistics_cond);
    __limpet_mutex_unlock(&__limpet_statistics_mutex);
}

static void __limpet_inc_started(void) {
    __limpet_statistics_inc(&__limpet_started);
}

static unsigned __limpet_get_started(void) {
    unsigned started;

    __limpet_mutex_lock(&__limpet_statistics_mutex);
    started = __limpet_started;
    __limpet_mutex_unlock(&__limpet_statistics_mutex);

    return started;
}

static void __limpet_inc_passed(void) {
    __limpet_statistics_inc(&__limpet_passed);
}

static void __limpet_inc_failed(void) {
    __limpet_statistics_inc(&__limpet_failed);
}

static void __limpet_inc_skipped(void) {
    __limpet_statistics_inc(&__limpet_skipped);
}

/*
 * Wait until we can start another test
 */
static void __limpet_wait_pending(unsigned pending) {
    unsigned running;

    __limpet_mutex_lock(&__limpet_statistics_mutex);

    running = __limpet_started - (__limpet_passed + __limpet_failed);
    while (running >= pending) {
        __limpet_cond_wait(&__limpet_statistics_cond,
            &__limpet_statistics_mutex);
        running = __limpet_started -
            (__limpet_passed + __limpet_failed);
    }

    __limpet_mutex_unlock(&__limpet_statistics_mutex);
}

/*
 * Called by per-test constructor to add the test to the test list. This
 * is called in a single threaded context.
 */
static void __limpet_enqueue_test(struct __limpet_test *test) {
    test->next = __limpet_list;
    __limpet_list = test;
}

/*
 * Add a test to the list of those to report. Called in a single threaded
 * context.
 */
static struct __limpet_test *
__limpet_first_test(void) {
    return __limpet_list;
}

/*
 * Returns the next test in the list. Called in a single threaded context.
 */
static struct __limpet_test *
__limpet_next_test(struct __limpet_test * test) {
    return test->next;
}

static bool __limpet_must_run(const char *name) {
    size_t i;

    if (__limpet_params.runlist == NULL) {
        return true;
    }

    for (i = 0; i < __limpet_params.n_runlist; i++) {
        if (strcmp(name, __limpet_params.runlist[i]) == 0) {
            return true;
        }
    }

    return false;
}

/*
 * Call before printing each test's information
 */
static void __limpet_print_test_header(struct __limpet_test *test,
    const char *sep) {
    __limpet_printf("%s", sep);
    __limpet_printf("%sLog for %s\n", __LIMPET_MARKER, test->name);
}

/*
 * Call after printing each test
 */
static void __limpet_print_test_trailer(struct __limpet_test *test) {
    __limpet_printf("%sTest complete: %s ", __LIMPET_MARKER, test->name);
    __limpet_print_status(test);
    __limpet_printf("\n");
}

/*
 * Call after printing all test information
 */
static void __limpet_print_final_trailer(const char *sep) {
    
    __limpet_printf("%s", sep);
    printf("%sRan %u tests: %u passed %u failed %u skipped\n",
        __LIMPET_MARKER, __limpet_started, __limpet_passed, __limpet_failed,
        __limpet_skipped);
}

/*
 * Called to print a report on any tests pending in the queue of completed
 * tests.
 *
 * Returns: true if it printed something, false otherwise.
 */
static bool __limpet_report_on_done(size_t *reported, const char *sep) {
    bool printed_something = false;

    for (; *reported != __limpet_get_started(); (*reported)++) {
        struct __limpet_test *p;

        p = __limpet_dequeue_done();
        __limpet_cleanup_test(p);

        printed_something |= __limpet_pre_stored(p, sep);

        __limpet_dump_stored_log(p);

        __limpet_post_stored(p);
    }

    return printed_something;
}

/*
 * This is the function that runs all the tests in the file including this
 * header file. There will actually be one of these in each file #including
 * limpet.h, but since they are identical, any of them can be run. Only
 * one of them will be run because the first one exits the process.
 */
static void __limpet_run(void) \
    __attribute((constructor(__LIMPET_RUN_PRI)));
static void __limpet_run(void) {
    struct __limpet_test *p;
    const char *sep;
    size_t reported;

printf("Running limpet\n");
    __limpet_mutex_init(&__limpet_statistics_mutex);
    __limpet_cond_init(&__limpet_statistics_cond);
    __limpet_parse_params(&__limpet_params);

    sep = "";
    reported = 0;

    for (p = __limpet_first_test(); p != NULL;
        p = __limpet_next_test(p)) {
        if (!__limpet_must_run(p->name)) {
            p->skipped = true;
            __limpet_inc_skipped();
            continue;
        }

        /*
         * If we have a maximum number of concurrent jobs, wait until the
         * number of pending tests is less than or equal to that
         */
        if (__limpet_params.max_jobs != 0) {
            __limpet_wait_pending(__limpet_params.max_jobs);
        }

        __limpet_inc_started();

        if (__limpet_pre_start( p, sep)) {
            sep = __LIMPET_REPORT_SEP;
        }

        __limpet_start_one(p);

        __limpet_post_start(p);

        /*
         * Keep resource usage to a minimum by doing reporting here. It
         * doesn't need to be exact, so no fancy sychronization.
         */
        if (__limpet_report_on_done(&reported, sep)) {
            sep = __LIMPET_REPORT_SEP;
        }
    }

    /*
     * All tests have been started. Print reports for any that haven't
     * been reported.
     */
    if (__limpet_report_on_done(&reported, sep)) {
        sep = __LIMPET_REPORT_SEP;
    }

    __limpet_print_final_trailer(sep);

    __limpet_exit(__limpet_failed != 0);
}
#endif /* LIMPET */
#endif /* _LEAVEIN_H_ */
