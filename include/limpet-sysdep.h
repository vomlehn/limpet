/*
 * Things needed to provide definitions for use in the system-dependent
 * header file. It should be included at the beginning of that header file.
 *
 *  ========================================================================
 * |                            NOTE                                        |
 * |                                                                        |
 * | The struct __limpet_sysdep must be defined before #including this   |
 * | file.                                                                  |
 *  ========================================================================
 */

#ifndef __LIMPET_SYSDEP_H_
#define __LIMPET_SYSDEP_H_

/*
 * Number of elements in array a.
 */
#define __LIMPET_ARRAY_SIZE(a)   (sizeof(a) / sizeof ((a)[0]))

/*
 * GNU C++ seems to think some functions are unused, even though they aren't.
 * This is used to supress the error messages.
 */
#define __LIMPET_UNUSED  __attribute((unused))

#define __LIMPET_DEFAULT_TIMEOUT     30.0

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
struct __limpet_params {
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
struct __limpet_test {
    struct __limpet_test     *next;
    struct __limpet_test     *done;
    bool                        skipped;
    const char *                name;
    void                        (*func)(void);
    struct __limpet_params   *params;
    struct __limpet_sysdep   sysdep;
};

/*
 * Functions available for use by the system-dependent definitions
 */
static void __limpet_inc_passed(void);
static void __limpet_inc_failed(void);
static void __limpet_enqueue_done(struct __limpet_test *test);

/*
 * Define a constant value for initializing __limpet_sysdep
 *
 *      #define __LIMPET_SYSDEP_INIT { \
 *          }
*/

/*
 * Define the following system-dependent functions after #including
 * limpet-sysdep.h
 */
static void __limpet_exit(bool is_error) __attribute((noreturn));
static void __limpet_fail(const char *fmt, ...) __attribute((noreturn));
static void __limpet_warn(const char *fmt, ...);
static void __limpet_printf(const char *fmt, ...);

struct __limpet_mutex;
static void __limpet_mutex_init(struct __limpet_mutex *mutex);
static void __limpet_mutex_lock(struct __limpet_mutex *mutex);
static void __limpet_mutex_unlock(struct __limpet_mutex *mutex);

struct __limpet_cond;
static void __limpet_cond_init(struct __limpet_cond *cond);
static void __limpet_cond_signal(struct __limpet_cond *cond);
static void __limpet_cond_wait(struct __limpet_cond *cond,
    struct __limpet_mutex *mutex);

static const char *__limpet_get_maxjobs(void);
static const char *__limpet_get_runlist(void);
static const char *__limpet_get_timeout(void);

static void __limpet_parse_done(void);

static ssize_t __limpet_dump_log(struct __limpet_test *test);
static void __limpet_start_one(struct __limpet_test *test);
static void __limpet_cleanup_test(struct __limpet_test *test);
static void __limpet_print_status(struct __limpet_test *test);

static void __limpet_pre_start(struct __limpet_test *test);
static void __limpet_post_start(struct __limpet_test *test);
static void __limpet_print_test_trailer(struct __limpet_test *test);
#endif /* __LIMPET_SYSDEP_H_ */
