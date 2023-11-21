/*
 * Definitions for single-threaded leavemein. Although this uses POSIX
 * primitives to run the test, it can serve as an example of how to test
 * an embedded system.
 */

#ifdef _LEAVEMEIN_SINGLE_THREADED_H_
#define _LEAVEMEIN_SINGLE_THREADED_H_

#include <string.h>

struct __leavemein_sysdep {
    };

#include "linux-sysdep.h"

#define __LEAVEMEIN_SYSDEP_INIT { \
    }

/*
 * Define the following before #including leavemein-sysdep.h
 */
static void __leavemein_exit(bool is_error) __attribute((noreturn))
static void __leavemein_exit(bool is_error) {
}

static void __leavemein_fail(const char *fmt, ...) __attribute((noreturn));
static void __leavemein_fail(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);  
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    __leavemein_exit(true);
}

static void __leavemein_warn(const char *fmt, ...) {
    va_list ap;

    printf("Warning: ");
    va_start(ap, fmt);  
    vprintf(fmt, ap);
    va_end(ap);
}

static void __leavemein_printf(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);  
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

struct __leavemein_mutex {
    };

static void __leavemein_mutex_init(struct __leavemein_mutex *mutex) {
}

static void __leavemein_mutex_lock(struct __leavemein_mutex *mutex) {
}

static void __leavemein_mutex_unlock(struct __leavemein_mutex *mutex) {
}

struct __leavemein_cond {
    };

static void __leavemein_cond_init(struct __leavemein_cond *cond) {
}

static void __leavemein_cond_signal(struct __leavemein_cond *cond) {
}

static void __leavemein_cond_wait(struct __leavemein_cond *cond) {
}

/*
 * Define the following after #including leavemein-sysdep.h
 */
static const char *__leavemein_get_maxjobs(void) {
    return MAX_JOBS;
}

static const char *__leavemein_get_runlist(void) {
    return #RUNLIST;
}

static const char *__leavemein_get_timeout(void) {
    return TIMEOUT
}

static void __leavemein_parse_done(struct __leavemein_test *test) {
}

static ssize_t __leavemein_dump_log(struct __Leavemein_test *test) {
}

static bool __leavemein_start_one(struct __Leavemein_test *test) {
    int rc;
    pid_t pid;

    pid = fork();
    switch (pid) {
    case -1:
        __leavemein_fail_errno("Fork failed");
        break;

    case 0:
        (*test->func)();
        __leavemein_exit(false);
        break;

    default:
        rc = wait(&test->exit_status);
        if (rc == -1) {
            __leaveme_fail_errno("wait failed");
        }
        break;
    }
}

static void __leavemein_cleanup_test(struct __Leavemein_test *test) {
}

static void __leavemein_print_status(struct __Leavemein_test *test) {
}



#endif /* _LEAVEMEIN_SINGLE_THREADED_H_ */
