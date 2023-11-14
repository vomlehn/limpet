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
            .done = NULL,                                   \
            .name = #testname,                              \
            .func = testname,                               \
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
 * __leavemein_list - The linked list of all tests by the per-test constructors. Tests
 *      are added during the test discovery phase by single-threaded code and removed
 *      during the text execution phase. We're generally multi-threaded then, but
 *      only one single thread function removes tests. So, no mutual exclusion needed.
 * __leavemein_donelist - Linked list of completed tests. This is called from
 *      multi-threaded code, so it has to use mutual exclusion.
 * __leavemein_donelist_cond - Conditional variable protecting __leavemein_donelist
 */
struct __leavemein_test *__leavemein_list __attribute((common));
struct __leavemein_test *__leavemein_donelist __attribute((common));
struct __leavemein_mutex __leavemein_donelist_mutex __attribute((common));
struct __leavemein_cond __leavemein_donelist_cond __attribute((common));

/*
 * Prototypes for system-dependent common functions
 */
static void __leavemein_update_status(bool is_error);

/*
 * Called by per-test constructor to add the test to the test list
 */
static void __leavemein_enqueue_test(struct __leavemein_test *test) {
    test->next = __leavemein_list;
    printf("Enqueuing test %s\n", test->name);
    __leavemein_list = test;
}

/*
decrement_count()
{
    pthread_mutex_lock(&count_lock);
    while (count == 0)
        pthread_cond_wait(&count_nonzero, &count_lock);
    count = count - 1;
    pthread_mutex_unlock(&count_lock);
}

increment_count()
{
    pthread_mutex_lock(&count_lock);
    if (count == 0)
        pthread_cond_signal(&count_nonzero);
    count = count + 1;
    pthread_mutex_unlock(&count_lock);
}
*/

/*
 * Pull one test off the done list. Only call this when there is at least one pending
 * test
 */
static void __leavemein_enqueue_done(struct __leavemein_test *test) {
    __leavemein_lock(&__leavemein_donelist_mutex);

    test->done = __leavemein_donelist;
    __leavemein_donelist = test;

    __leavemein_cond_signal(&__leavemein_donelist_cond);
    __leavemein_unlock(&__leavemein_donelist_mutex);
}


/*
 * Pull one test off the done list. Only call this when there is at least one pending
 * test
 */
static struct __leavemein_test * __leavemein_dequeue_done(void) {
    __leavemein_test *test;

    __leavemein_lock(&__leavemein_donelist_mutex);

    while (__leavemein_donelist == NULL) {
        __leavemein_cond_wait(&__leavemein_donelist_cond, &__leavemein_donelist_mutex);
    }

    test = __leavemein_donelist;
    __leavemein_unlock(&__leavemein_donelist_mutex);
    
    return test;
}

static void __leavemein_report(struct __leavemein_test *test) {
    __leavemein_dump_log(test);
    __leavemein_printf("Test complete: %s ", test->name);
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
    struct __leavemein_params params;
    unsigned pending = 0;
    unsigned count;
    unsigned i;

    __leavemein_mutex_init(&__leavemein_donelist_mutex);
    __leavemein_cond_init(&__leavemein_donelist_cond);
    __leavemein_parse_params(&params);

printf("start run\n");
    /* printf("start test execution"); */
    for (p = __leavemein_list; p != NULL; p = p->next) {
        count += 1;
        if (params.max_jobs != 0 && pending == params.max_jobs) {
            __leavemein_test *test;

            test = __leavemein_dequeue_done();
            __leavemein_report(test);
            pending -= 1;
        }
        
        printf("Starting test %s\n", p->name);
        __leavemein_start_one(&params, p);
        pending += 1;
    }

    for (i = 0; i < count; i++) {
        p = __leavemein_dequeue_done();
        __leavemein_report(p);
    }

    printf("Ran %u tests\n", count);
    __leavemein_exit(false);
}
#endif /* LEAVEMEIN */
#endif /* _LEAVEIN_H_ */
