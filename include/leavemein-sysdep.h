/*
 * Things needed to provide definitions for use in the system-dependent
 * header file. It should be included at the beginning of that header file.
 *
 *  ========================================================================
 * |                            NOTE                                        |
 * |                                                                        |
 * | The struct __leavemein_sysdep must be defined before #including this   |
 * | file.                                                                  |
 *  ========================================================================
 */

#ifndef __LEAVEMEIN_SYSDEP_H_
#define __LEAVEMEIN_SYSDEP_H_

/*
 * GNU C++ seems to think some functions are unused, even though they aren't.
 * This is used to supress the error messages.
 */
#define __LEAVEMEIN_UNUSED  __attribute((unused))

#define __LEAVEMEIN_DEFAULT_TIMEOUT     30.0

/*
 * Parameters of the test run.
 * max_jobs - Maximum number of simultaneous threads. A zero value means
 *  there is no limit.
 * n_runlist - Number of items in runlist. If this is zero and runlist is
 *      NULL, run all tests. If this is zero and runlist is not NULL but
 *      empty, run nothing.
 * runlist - List of tests to run
 * timeout - Number of seconds to allow each test to run
 */
struct __leavemein_params {
    unsigned max_jobs;
    size_t n_runlist;
    char **runlist;
    float timeout;
};

/*
 * Per-test information. This part is common to all implementations
 *  next - Pointer to next test to run
 *  done - Pointer to the next completed test
 *  skipped - true if this test was skipped
 *  name - Name of the test
 *  func - Function to execute the test
 *  next - Next item on the list of tests, or NULL at the end.
 *  sysdep - System-dependent information
 */
struct __leavemein_test {
    struct __leavemein_test     *next;
    struct __leavemein_test     *done;
    bool                        skipped;
    const char *                name;
    void                        (*func)(void);
    struct __leavemein_params   *params;
    struct __leavemein_sysdep   sysdep;
};

/*
 * Functions available for use by the system-dependent definitions
 */
static void __leavemein_inc_passed(void);
static void __leavemein_inc_failed(void);
static void __leavemein_enqueue_done(struct __leavemein_test *test);

/*
 * Define a constant value for initializing __leavemein_sysdep
 *
 *      #define __LEAVEMEIN_SYSDEP_INIT { \
 *          }
*/

/*
 * Define the following system-dependent functions after #including
 * leavemein-sysdep.h
 */
static void __leavemein_exit(bool is_error) __attribute((noreturn));
static void __leavemein_fail(const char *fmt, ...) __attribute((noreturn));
static void __leavemein_warn(const char *fmt, ...);
static void __leavemein_printf(const char *fmt, ...);

struct __leavemein_mutex;
static void __leavemein_mutex_init(struct __leavemein_mutex *mutex);
static void __leavemein_mutex_lock(struct __leavemein_mutex *mutex);
static void __leavemein_mutex_unlock(struct __leavemein_mutex *mutex);

struct __leavemein_cond;
static void __leavemein_cond_init(struct __leavemein_cond *cond);
static void __leavemein_cond_signal(struct __leavemein_cond *cond);
static void __leavemein_cond_wait(struct __leavemein_cond *cond,
    struct __leavemein_mutex *mutex);

static const char *__leavemein_get_maxjobs(void);
static const char *__leavemein_get_runlist(void);
static const char *__leavemein_get_timeout(void);

static void __leavemein_parse_done(void);

static ssize_t __leavemein_dump_log(struct __leavemein_test *test);
static void __leavemein_start_one(struct __leavemein_test *test);
static void __leavemein_cleanup_test(struct __leavemein_test *test);
static void __leavemein_print_status(struct __leavemein_test *test);

static void __leavemein_pre_start(struct __leavemein_test *test);
static void __leavemein_post_start(struct __leavemein_test *test);
#endif /* __LEAVEMEIN_SYSDEP_H_ */
