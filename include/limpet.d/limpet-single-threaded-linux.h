/*
 * Linux-specific single-threaded Linux version of Limpet
 */

#ifndef _LIMPET_SINGLE_THREADED_LINUX_H_
#define _LIMPET_SINGLE_THREADED_LINUX_H_

#include <fcntl.h>

/*
 * System-dependent definitions
 * pid - Process ID of forked process
 * tty - output file descriptor for the child
 * timedout - true if the process timed out
 * exit_status - child's exit status
 */
struct __limpet_sysdep {
    pid_t   pid;
    int     tty;
    bool    timedout;
    int     exit_status;
};

#include "limpet.d/limpet-single-threaded.h"

#define __LIMPET_SYSDEP_INIT { \
        .pid = -1, \
        .tty = -1, \
        .timedout = false, \
        .exit_status = -1, \
    }

static void __limpet_exit(bool is_error) __attribute((noreturn));
static void __limpet_exit(bool is_error) {
    exit(is_error ? EXIT_FAILURE : EXIT_SUCCESS);
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
 * Set up the stdin file descriptor
 */
static void __limpet_make_std_fd(struct __limpet_sysdep *sysdep)
    __LIMPET_UNUSED;
static void __limpet_make_std_fd(struct __limpet_sysdep *sysdep)
    __LIMPET_UNUSED;
static void __limpet_make_std_fd(struct __limpet_sysdep *sysdep)
{
    if (!__limpet_params.verbose) {
        sysdep->tty = open("/dev/null", O_RDWR);

        if (sysdep->tty == -1) {
            __limpet_fail_errno("Unable to open /dev/null");
        }
    }
}

/*
 * Running in the context of a test child process, set up a new test
 * process
 */
static void __limpet_setup_one(struct __limpet_sysdep *sysdep)
    __LIMPET_UNUSED;
static void __limpet_setup_one(struct __limpet_sysdep *sysdep) {
    if (!__limpet_params.verbose) {
        if (dup2(sysdep->tty, 0) == -1) {
            __limpet_fail_errno("dup2(%d, %d)", sysdep->tty, 0);
        }

        if (close(sysdep->tty) == -1) {
            __limpet_fail_errno("close(tty %d)", sysdep->tty);
        }

        /*
         * Set up stdout and stderr
         */
        if (dup2(0, 1) == -1) {
            __limpet_fail_errno("dup2(%d, %d)", 0, 1);
        }
        if (dup2(0, 2) == -1) {
            __limpet_fail_errno("dup2(%d, %d)", 0, 2);
        }
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
        __limpet_make_std_fd(&test->sysdep);
        __limpet_setup_one(&test->sysdep);
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

#endif /* _LIMPET_SINGLE_THREADED_LINUX_H_ */
