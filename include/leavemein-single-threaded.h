#ifdef _LEAVEMEIN_SINGLE_THREADED_H_
#define _LEAVEMEIN_SINGLE_THREADED_H_

#include <string.h>

/*
 * Define the following before #including leavemein-sysdep.h
 */
static void __leavemein_exit(bool is_error) __attribute((noreturn))
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
static void __leavemein_cond_wait(struct __leavemein_cond *cond);

struct __leavemein_sysdep;

/*
 * Include definitions that are available for system-dependent definitions
 *
 *      #include "leavemein-sysdep.h"
 *
 * Define a constant value for initializing __leavemein_sysdep
 *
 *      #define __LEAVEMEIN_SYSDEP_INIT { \
 *          }
*/

/*
 * Define the following after #including leavemein-sysdep.h
 */
static const char *__leavemein_get_maxjobs(void);
static const char *__leavemein_get_runlist(void);
static const char *__leavemein_get_timeout(void);

static void __leavemein_parse_done(struct __leavemein_test *test);

static ssize_t __leavemein_dump_log(struct __Leavemein_test *test);
static bool __leavemein_start_one(struct __Leavemein_test *test);
static void __leavemein_cleanup_test(struct __Leavemein_test *test);
static void __leavemein_print_status(struct __Leavemein_test *test);
#endif /* _LEAVEMEIN_SINGLE_THREADED_H_ */
