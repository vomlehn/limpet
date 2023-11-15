/*
 * Things needed to provode system-dependent leavemein header file
 * definitions.
 * 
 */

#ifndef __LEAVEMEIN_SYSDEP_H_
#define __LEAVEMEIN_SYSDEP_H_

#define __LEAVEMEIN_DEFAULT_TIMEOUT     3.0

/*
 * Parameters of the test run.
 * max_jobs - Maximum number of simultaneous threads. A zero value means
 *  there is no limit.
 * n_skiplist - Number of items in skiplist. If this is zero and skiplist is
 *      NULL, run all tests. If this is zero and skiplist is not NULL but empty,
 *      run nothing.
 * skiplist - List of tests to run
 * timeout - Number of seconds to allow each test to run
 */
struct __leavemein_params {
    unsigned max_jobs;
    size_t n_skiplist;
    char **skiplist;
    float timeout;
};

/*
 * Per-test information. This part is common to all implementations
 *  next - Pointer to next test to run
 *  done - Pointer to next completed test
 *  name - Name of the test
 *  func - Function to execute the test
 *  next - Next item on the list of tests, or NULL at the end.
 *  sysdep - System-dependent information
 */
struct __leavemein_test {
    struct __leavemein_test     *next;
    struct __leavemein_test     *done;
    const char *                name;
    void                        (*func)(void);
    struct __leavemein_sysdep   sysdep;
};

static void __leavemein_enqueue_done(struct __leavemein_test *test);
static struct __leavemein_test * __leavemein_dequeue_done(void);
static void __leavemein_inc_passed(void);
static void __leavemein_inc_failed(void);
#endif /* __LEAVEMEIN_SYSDEP_H_ */
