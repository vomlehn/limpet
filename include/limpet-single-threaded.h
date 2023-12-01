/*
 * Definitions for single-threaded limpet. Although this uses POSIX
 * primitives to run the test, it can serve as an example of how to test
 * an embedded system.
 */

#ifndef _LIMPET_SINGLE_THREADED_H_
#define _LIMPET_SINGLE_THREADED_H_

#include <sys/param.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

struct __limpet_sysdep {
    pid_t   pid;
    bool    timedout;
    int     exit_status;
};

#include "limpet-sysdep.h"
#include "limpet-posix.h"

#define __LIMPET_STRINGIZE(token)    #token

#define __LIMPET_SYSDEP_INIT { \
        .pid = -1, \
        .timedout = false, \
        .exit_status = -1, \
    }

/*
 * Printing and exit functions
 */
#define __limpet_fail_with(err, fmt, ...)    do {        \
        __limpet_fail(fmt ": %s\n", ##__VA_ARGS__, strerror(err)); \
    } while (0)

#define __limpet_fail_errno(fmt, ...)    do {        \
        __limpet_fail_with(errno, fmt, ##__VA_ARGS__); \
    } while (0)

#define __limpet_warn_with(err, fmt, ...)    do {        \
        __limpet_warn(fmt ": %s\n", ##__VA_ARGS__, strerror(err)); \
    } while (0)

#define __limpet_warn_errno(fmt, ...)    do {        \
        __limpet_warn_with(errno, fmt, ##__VA_ARGS__); \
    } while (0)
static void __limpet_exit(bool is_error) __attribute((noreturn));
static void __limpet_exit(bool is_error) {
    exit(is_error ? EXIT_FAILURE : EXIT_SUCCESS);
}

static void __limpet_fail(const char *fmt, ...) __attribute((noreturn));
static void __limpet_fail(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);  
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    __limpet_exit(true);
}

static void __limpet_warn(const char *fmt, ...) __attribute((unused));
static void __limpet_warn(const char *fmt, ...) {
    va_list ap;

    printf("Warning: ");
    va_start(ap, fmt);  
    vprintf(fmt, ap);
    va_end(ap);
}

static void __limpet_printf(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);  
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

struct __limpet_mutex {
    };

static void __limpet_mutex_init(struct __limpet_mutex *mutex) {
}

static void __limpet_mutex_lock(struct __limpet_mutex *mutex) {
}

static void __limpet_mutex_unlock(struct __limpet_mutex *mutex) {
}

struct __limpet_cond {
    };

static void __limpet_cond_init(struct __limpet_cond *cond) {
}

static void __limpet_cond_signal(struct __limpet_cond *cond) {
}

static void __limpet_cond_wait(struct __limpet_cond *cond,
    struct __limpet_mutex *mutex) {
}

/*
 * Define the following after #including limpet-sysdep.h
 */
static const char *__limpet_get_maxjobs(void) {
#ifdef __LIMPET_MAX_JOBS
    return __LIMPET_STRINGIZE(LIMPET_MAX_JOBS);
#else
    return NULL;
#endif
}

static const char *__limpet_get_runlist(void) {
#ifdef LIMPET_RUNLIST
    return __LIMPET_STRINGIZE(LIMPET_RUNLIST);
#else
    return NULL;
#endif
}

static const char *__limpet_get_timeout(void) {
#ifdef __LIMPET_TIMEOUT
    return __LIMPET_STRINGIZE(LIMPET_TIMEOUT);
#else
    return NULL;
#endif
}

static void __limpet_parse_done() {
}

/*
 * We don't store the log, so nothing to do other than report we printed
 * the log.
 */
static ssize_t __limpet_dump_stored_log(struct __limpet_test *test) {
    return 1;
}

/*
 * Wait for the process to terminate
 */
static void __limpet_wait(struct __limpet_test *test) {
    struct timeval timeout;
    struct timeval abs_timeout;
    struct timeval delta_timeout;
    struct timeval now;
    struct timeval *tv;
    fd_set rfds;
    int nfds;
    int pid_fd;
    int rc;

    timeout.tv_sec = (time_t)test->params->timeout;
    timeout.tv_usec = (suseconds_t)((test->params->timeout - timeout.tv_sec) *
        1000000);
    rc = gettimeofday(&abs_timeout, NULL);
    if (rc == -1) {
        __limpet_fail_errno("gettimeofday failed");
    }
    timeradd(&abs_timeout, &timeout, &abs_timeout);

    /*
     * Get a file descriptor for this pid so we can use select() to wait for
     * it to exit
     */
    pid_fd = syscall(SYS_pidfd_open, test->sysdep.pid, 0);
    if (pid_fd == -1) {
        __limpet_fail_errno("pidfd_open failed for pid %d",
            test->sysdep.pid);
    }

    nfds = 0;

    FD_ZERO(&rfds);

    FD_SET(pid_fd, &rfds);
    nfds = MAX(nfds, pid_fd + 1);

    rc = gettimeofday(&now, NULL);
    if (rc == -1) {
        __limpet_fail("gettimeofday failed");
    }
    if (timercmp(&abs_timeout, &now, >)) {
        timersub(&abs_timeout, &now, &delta_timeout);
    } else {
        timerclear(&delta_timeout);
    }
    tv = &delta_timeout;

    rc = select(nfds, &rfds, NULL, NULL, tv);
    if (rc == -1) {
        __limpet_fail_errno("Select failed");
    }

    if (rc == 0) {
        test->sysdep.timedout = true;
        rc = kill(test->sysdep.pid, SIGKILL);
        if (rc == -1) {
            __limpet_warn("Unable to kill PID %d\n",
                test->sysdep.pid);
        }
    }

    rc = waitpid(test->sysdep.pid, &test->sysdep.exit_status, 0);
    if (rc == -1) {
        __limpet_fail_errno("waitpid failed");
    }

    if (close(pid_fd) == -1) {
        __limpet_fail_errno("close(pid_fd) failed");
    }
}

/*
 * Run the test as a subprocess
 */
static void __limpet_start_one(struct __limpet_test *test)
    __LIMPET_UNUSED;
static void __limpet_start_one(struct __limpet_test *test) {
    pid_t pid;

    if (fflush(stdout) == -1) {
        __limpet_fail_errno("fflush(stdout) failed");
    }

    pid = fork();
    switch (pid) {
    case -1:
        __limpet_fail_errno("Fork failed");
        break;

    case 0:
        (*test->func)();
        __limpet_exit(false);
        break;

    default:
        /*
         * Set the process ID and let the wait begin
         */
        test->sysdep.pid = pid;
        break;
    }

    __limpet_wait(test);

    if (test->sysdep.timedout) {
        __limpet_inc_failed();
        __limpet_enqueue_done(test);
    } else if (!WIFEXITED(test->sysdep.exit_status) ||
        WEXITSTATUS(test->sysdep.exit_status) != 0) {
            __limpet_inc_failed();
            __limpet_enqueue_done(test);
    } else {
            __limpet_inc_passed();
            __limpet_enqueue_done(test);
    }
}

static void __limpet_cleanup_test(struct __limpet_test *test) {
}

/*
 * Print a string right before the test starts
 */
static bool __limpet_pre_start(struct __limpet_test *test,
    const char *sep) {
    __limpet_print_test_header(test, sep);
    return true;
}

static void __limpet_post_start(struct __limpet_test *test) {
    __limpet_print_test_trailer(test);
}

static bool __limpet_pre_stored(struct __limpet_test *test,
    const char *sep) {
    return false;
}

static void __limpet_post_stored(struct __limpet_test *test) {
}
#endif /* _LIMPET_SINGLE_THREADED_H_ */
