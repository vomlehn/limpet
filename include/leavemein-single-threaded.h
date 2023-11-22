/*
 * Definitions for single-threaded leavemein. Although this uses POSIX
 * primitives to run the test, it can serve as an example of how to test
 * an embedded system.
 */

#ifndef _LEAVEMEIN_SINGLE_THREADED_H_
#define _LEAVEMEIN_SINGLE_THREADED_H_

#include <sys/param.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

struct __leavemein_sysdep {
    pid_t   pid;
    bool    timedout;
    int     exit_status;
};

#include "leavemein-sysdep.h"

#define __LEAVEMEIN_STRINGIZE(token)    #token

#define __LEAVEMEIN_SYSDEP_INIT { \
        .pid = -1, \
        .timedout = false, \
        .exit_status = -1, \
    }

/*
 * Printing and exit functions
 */
#define __leavemein_fail_with(err, fmt, ...)    do {        \
        __leavemein_fail(fmt ": %s\n", ##__VA_ARGS__, strerror(err)); \
    } while (0)

#define __leavemein_fail_errno(fmt, ...)    do {        \
        __leavemein_fail_with(errno, fmt, ##__VA_ARGS__); \
    } while (0)

#define __leavemein_warn_with(err, fmt, ...)    do {        \
        __leavemein_warn(fmt ": %s\n", ##__VA_ARGS__, strerror(err)); \
    } while (0)

#define __leavemein_warn_errno(fmt, ...)    do {        \
        __leavemein_warn_with(errno, fmt, ##__VA_ARGS__); \
    } while (0)
static void __leavemein_exit(bool is_error) __attribute((noreturn));
static void __leavemein_exit(bool is_error) {
    exit(is_error ? EXIT_FAILURE : EXIT_SUCCESS);
}

static void __leavemein_fail(const char *fmt, ...) __attribute((noreturn));
static void __leavemein_fail(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);  
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    __leavemein_exit(true);
}

static void __leavemein_warn(const char *fmt, ...) __attribute((unused));
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

static void __leavemein_cond_wait(struct __leavemein_cond *cond,
    struct __leavemein_mutex *mutex) {
}

/*
 * Define the following after #including leavemein-sysdep.h
 */
static const char *__leavemein_get_maxjobs(void) {
#ifdef __LEAVEMEIN_MAX_JOBS
    return __LEAVEMEIN_STRINGIZE(LEAVEMEIN_MAX_JOBS);
#else
    return NULL;
#endif
}

static const char *__leavemein_get_runlist(void) {
#ifdef LEAVEMEIN_RUNLIST
    return __LEAVEMEIN_STRINGIZE(LEAVEMEIN_RUNLIST);
#else
    return NULL;
#endif
}

static const char *__leavemein_get_timeout(void) {
#ifdef __LEAVEMEIN_TIMEOUT
    return __LEAVEMEIN_STRINGIZE(LEAVEMEIN_TIMEOUT);
#else
    return NULL;
#endif
}

static void __leavemein_parse_done() {
}

static ssize_t __leavemein_dump_log(struct __leavemein_test *test) {
    return 1;
}

/*
 * Wait for the process to terminate
 */
static void __leavemein_wait(struct __leavemein_test *test) {
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
        __leavemein_fail_errno("gettimeofday failed");
    }
    timeradd(&abs_timeout, &timeout, &abs_timeout);

    /*
     * Get a file descriptor for this pid so we can use select() to wait for
     * it to exit
     */
    pid_fd = syscall(SYS_pidfd_open, test->sysdep.pid, 0);
    if (pid_fd == -1) {
        __leavemein_fail_errno("pidfd_open failed for pid %d",
            test->sysdep.pid);
    }

    nfds = 0;

    FD_ZERO(&rfds);

    FD_SET(pid_fd, &rfds);
    nfds = MAX(nfds, pid_fd + 1);

    rc = gettimeofday(&now, NULL);
    if (rc == -1) {
        __leavemein_fail("gettimeofday failed");
    }
    if (timercmp(&abs_timeout, &now, >)) {
        timersub(&abs_timeout, &now, &delta_timeout);
    } else {
        timerclear(&delta_timeout);
    }
    tv = &delta_timeout;

    rc = select(nfds, &rfds, NULL, NULL, tv);
    if (rc == -1) {
        __leavemein_fail_errno("Select failed");
    }

    if (rc == 0) {
        test->sysdep.timedout = true;
        rc = kill(test->sysdep.pid, SIGKILL);
        if (rc == -1) {
            __leavemein_warn("Unable to kill PID %d\n",
                test->sysdep.pid);
        }
    }

    rc = waitpid(test->sysdep.pid, &test->sysdep.exit_status, 0);
    if (rc == -1) {
        __leavemein_fail_errno("waitpid failed");
    }

    if (close(pid_fd) == -1) {
        __leavemein_fail_errno("close(pid_fd) failed");
    }
}

/*
 * Run the test as a subprocess
 */
static void __leavemein_start_one(struct __leavemein_test *test)
    __LEAVEMEIN_UNUSED;
static void __leavemein_start_one(struct __leavemein_test *test) {
    pid_t pid;

    if (fflush(stdout) == -1) {
        __leavemein_fail_errno("fflush(stdout) failed");
    }

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
        /*
         * Set the process ID and let the wait begin
         */
        test->sysdep.pid = pid;
        break;
    }

    __leavemein_wait(test);

    if (test->sysdep.timedout) {
        __leavemein_inc_failed();
        __leavemein_enqueue_done(test);
    } else if (!WIFEXITED(test->sysdep.exit_status) ||
        WEXITSTATUS(test->sysdep.exit_status) != 0) {
            __leavemein_inc_failed();
            __leavemein_enqueue_done(test);
    } else {
            __leavemein_inc_passed();
            __leavemein_enqueue_done(test);
    }
}

static void __leavemein_cleanup_test(struct __leavemein_test *test) {
}

static void __leavemein_print_status(struct __leavemein_test *test) {
}

static void __leavemein_pre_start(struct __leavemein_test *test) {
    printf("PRE_START\n");
}

static void __leavemein_post_start(struct __leavemein_test *test) {
    printf("PRE_START\n");
}
#endif /* _LEAVEMEIN_SINGLE_THREADED_H_ */
