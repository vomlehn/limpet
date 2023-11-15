/*
 * Definitions for the Linux-based Leavemein
 */

#ifndef _LEAVEIN_TEST_LINUX_H_
#define _LEAVEIN_TEST_LINUX_H_

#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <pty.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

/*
 * Value to use for __leavemein_sysdep initiatization
 */
#define __LEAVEMEIN_SYSDEP_INIT         {       \
        .log_fd = -1,                           \
        .raw_pty = -1,                          \
        .tty_pty = -1,                          \
        .pid = -1,                              \
        .thread = 0,                            \
        .exit_status = 0,                       \
    }

/*
 * System-dependent per-test information.
 * log_fd - file descriptor for the log file.
 * test_fd - file descriptor on which will be written output from the test
 * bad_errno - If we fail and have an errno value, it will be stored here.
 */
struct __leavemein_sysdep {
    int         log_fd;
    int         raw_pty;
    int         tty_pty;
    pid_t       pid;
    pthread_t   thread;
    int         exit_status;
};

#include <leavemein-sysdep.h>

#define ARRAY_SIZE(a)   (sizeof(a) / sizeof ((a)[0]))

/*
 * Environment variables available to configure executation are:
 * LEAVEMEIN_MAX_JOBS   Specifies the maximum number of threads running at a
 *      time. If this is not set or is zero, there is no limit
 * LEAVEMEIN_RUNLIST    A list of names of tests to be run, separated by
 *      colons. If this is not set, all tests will be run. Note: If this is set
 *      to an empty string, no tests will be run
 */
#define __LEAVEMEIN_MAX_JOBS  "LEAVEMEIN_MAX_JOBS"
#define __LEAVEMEIN_RUNLIST   "LEAVEMEIN_RUNLIST"
#define __LEAVEMEIN_TIMEOUT   "LEAVEMEIN_TIMEOUT"

/*
 * List of all environment variables to eliminate before running the test
 */
static const char *__leavemein_envvars[] = {
    __LEAVEMEIN_MAX_JOBS,
    __LEAVEMEIN_RUNLIST,
    __LEAVEMEIN_TIMEOUT,
};

/*
 * Define initializatoin for a structure holding a mutex and the mutex struct itself
 */
#define __LEAVEMEIN_MUTEX_INIT   { .mutex = PTHREAD_MUTEX_INITIALIZER, }

struct __leavemein_mutex {
    pthread_mutex_t mutex;
};

/*
 * Define initialize for a struct with a conditional variable and a mutext
 */
#define __LEAVEMEIN_COND_INIT   { .cond = PTHREAD_COND_INITIALIZER, \
                                    .mutex = PTHREAD_MUTEX_INITIALIZER, }

struct __leavemein_cond {
    pthread_cond_t  cond;
};

/*
 * print an error message like printf()
 */
static void __leavemein_fail(const char *fmt, ...) __attribute((noreturn));
static void __leavemein_fail(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);  
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    exit(EXIT_FAILURE);
}

static void __leavemein_printf(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);  
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

#define __leavemein_fail_with(err, fmt, ...)    do {        \
        __leavemein_fail(fmt ": %s\n", ##__VA_ARGS__, strerror(errno)); \
    } while (0)

#define __leavemein_fail_errno(fmt, ...)    do {        \
        __leavemein_fail_with(errno, fmt, ##__VA_ARGS__); \
    } while (0)

/*
 * Add a test name to the run list
 * start - pointer to the beginning of the test name
 * end - pointer one past the end of the test name
 * params - pointer to the structure storing the configuration
 *
 * Returns: true if successful, false otherwise.
 */
static bool __leavemein_add_testname(struct __leavemein_params *params,
    const char *start, const char *end) {
    size_t testname_size;
    char *testname;
    size_t new_skiplist_size;

    testname_size = start - end + 1;        /* string plus terminator NUL */
    testname = (char *)malloc(testname_size);
    if (testname == NULL) {
        __leavemein_fail("Out of memory allocating %zu bytes for test name\n",
            testname_size);
        return false;
    }

    strncpy(testname, start, testname_size);

    new_skiplist_size = sizeof(params->skiplist[0]) * (params->n_skiplist);
    params->skiplist = (char **)realloc(params->skiplist, new_skiplist_size);
    if (params->skiplist == NULL) {
        __leavemein_fail("Unable to realloc %zu bytes\n", new_skiplist_size);
    }

    params->skiplist[params->n_skiplist] = testname;
    params->n_skiplist += 1;
    return true;
}

/*
 * Parse the skiplist, if any
 * params - Pointer to the configuration structure
 *
 * Returns: true on success, false otherwise
 */
static bool parse_skiplist(struct __leavemein_params *params) {
    const char *skiplist_env;

    skiplist_env = getenv(__LEAVEMEIN_RUNLIST);

    if (skiplist_env != NULL) {
        const char *start;
        const char *colon;

        params->skiplist = (char **)malloc(0);
        if (params->skiplist == NULL) {
            __leavemein_fail("Out of memory allocating skiplist\n");
        }

        start = skiplist_env;

        for (colon = strchr(start, ':'); colon != NULL;
            start = colon + 1, colon = strchr(start, ':')) {

            if (start == colon) {
                __leavemein_fail("Zero length test name invalid in %s\n",
                    __LEAVEMEIN_RUNLIST);
            }

            if (!__leavemein_add_testname(params, start, colon)) {
                return false;
            }

        }

        if (*start == '\0') {
            size_t name_len;

            name_len = strlen(start);
            if (!__leavemein_add_testname(params, start, start + name_len)) {
                return false;
            }
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
static bool __leavemein_parse_params(struct __leavemein_params *params) {
    const char *max_jobs_env;
    const char *timeout;
    size_t i;

    memset(params, 0, sizeof(*params));

    max_jobs_env = getenv(__LEAVEMEIN_MAX_JOBS);
    if (max_jobs_env != NULL) {
        char *endptr;
        params->max_jobs = strtoul(max_jobs_env, &endptr, 0);
        if (*max_jobs_env == '\0' || *endptr != '\0') {
            __leavemein_fail("Invalid value specified for %s\n",
                __LEAVEMEIN_MAX_JOBS);
        }
    }

    if (!parse_skiplist(params)) {
        return false;
    }

    timeout = getenv(__LEAVEMEIN_TIMEOUT);
    if (timeout == NULL) {
        params->timeout = __LEAVEMEIN_DEFAULT_TIMEOUT;
    } else {
        char *endptr;

        params->timeout = strtof(timeout, &endptr);

        if (*timeout == '\0' || *endptr != '\0') {
            __leavemein_fail("Bad timeout value\n", timeout);
        }
    }

    /*
     * Remove things in the environment specific to leavmein
     */
    for (i = 0; i < ARRAY_SIZE(__leavemein_envvars); i++) {
        const char *envvar;
        envvar = __leavemein_envvars[i];

        if (unsetenv(envvar) == -1) {
            __leavemein_fail("Unable to unset environment variable %s\n",
                envvar);
        }
    }

    return true;
}

static bool __leavemein_make_pty(struct __leavemein_sysdep *sysdep) {
    struct termios termios;
    struct winsize winsize;
    int rc;

    rc = tcgetattr(0, &termios);
    if (rc == -1) {
        __leavemein_fail_errno("Unable to get terminal characteristics");
    }

    rc = ioctl(0, TIOCGWINSZ, &winsize);
    if (rc == -1) {
        __leavemein_fail_errno("Unable to get windows size");
    }

    rc = openpty(&sysdep->raw_pty, &sysdep->tty_pty, NULL, &termios, &winsize);
    if (rc == -1) {
        __leavemein_fail_errno("Unable to create pty");
    }

    return true;
}

static bool __leavemein_thread_setup(struct __leavemein_test *test) {
    static const char tmpfile_name_template[] = "/tmp/logfileXXXXXX";
    char tmpfile_name[sizeof(tmpfile_name_template)];

    test->sysdep.log_fd = open(tmpfile_name_template,
        O_TMPFILE | O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);

    if (test->sysdep.log_fd == -1 && errno == EINVAL) {
        strncpy(tmpfile_name, tmpfile_name_template, sizeof(tmpfile_name));
        test->sysdep.log_fd = mkstemp(tmpfile_name);
    }

    if (test->sysdep.log_fd == -1) {
        __leavemein_fail_errno("Unable to make log file");
    }

    if (!__leavemein_make_pty(&test->sysdep)) {
        return false;
    }

    return true;
}

/*
 * Set up the new test process
 */
static void __leavemein_setup_one(struct __leavemein_sysdep *sysdep) {
    if (dup2(sysdep->tty_pty, 0) == -1) {
        __leavemein_fail_errno("dup2(%d, %d)", sysdep->tty_pty, 0);
    }
    if (dup2(0, 1) == -1) {
        __leavemein_fail_errno("dup2(%d, %d)", 0, 1);
    }
    if (dup2(0, 2) == -1) {
        __leavemein_fail_errno("dup2(%d, %d)", 0, 2);
    }

    if (close(sysdep->tty_pty) == -1) {
        __leavemein_fail_errno("close(tty_pty %d)", sysdep->tty_pty);
    }
    if (close(sysdep->raw_pty) == -1) {
        __leavemein_fail_errno("close(raw_pty %d)", sysdep->raw_pty);
    }
    if (close(sysdep->log_fd) == -1) {
        __leavemein_fail_errno("close(raw_pty %d)", sysdep->log_fd);
    }
}
    
/*
 * Copy from one file descriptor to another
 * in_fd - Input file descriptor
 * out_fd - Output file descriptor
 *
 * Returns: true on success, false on failure
 */
static bool copy_file(int in_fd, int out_fd) {
    char buf[4096];
    ssize_t zrc = 0;

    for (zrc = read(in_fd, buf, sizeof(buf)); zrc > 0;
        zrc = read(in_fd, buf, sizeof(buf))) {
        zrc = write(out_fd, buf, zrc);
        if (zrc < 0) {
            return false;
        }
    }

    if (zrc != 0) {
        return false;
    }

    return true;
}

static void __leavemein_log_output(struct __leavemein_test *test) {
    if (!copy_file(test->sysdep.raw_pty, test->sysdep.log_fd)) {
        __leavemein_fail_errno("Copy to log file failed");
    }
}

static void __leavemein_dump_log(struct __leavemein_test *test) {
    if (lseek(test->sysdep.log_fd, 0, SEEK_SET) == (off_t) -1) {
        __leavemein_fail_errno("lseek failed on fd %d", test->sysdep.log_fd);
    }

    if (!copy_file(test->sysdep.log_fd, 1)) {
        __leavemein_fail_errno("Log dump failed");
    }
}

static void *__leavemein_run_one(void *arg) {
    struct __leavemein_test *test = (struct __leavemein_test *)arg;
    __leavemein_thread_setup(test);

    if (fflush(stdout) == -1) {
        __leavemein_fail_errno("fflush(stdout) failed");
    }

    test->sysdep.pid = fork();
    switch (test->sysdep.pid) {
    case -1:
        __leavemein_fail_errno("Fork failed");
        break;

    case 0:
        __leavemein_setup_one(&test->sysdep);
        (*test->func)();
        exit(EXIT_SUCCESS);
        break;

    default:
        __leavemein_log_output(test);
        break;
    }

// FIXME: check this return value
    return test;
}

/*
 * Run one test
 *  test - Pointer to a __leavemein_params for the test to run
 * Returns: true if the test was started, false otherwise
 */
static bool __leavemein_start_one(struct __leavemein_params * params,
    struct __leavemein_test *test) {
    struct timeval timeout;
    fd_set rfds;
    int nfds;
    int pid_fd;
    int rc;

    rc = pthread_create(&test->sysdep.thread, NULL, __leavemein_run_one, test);
    if (rc != 0) {
        __leavemein_fail_with(rc, "Failed to create thread");
    }

    timeout.tv_sec = (time_t)params->timeout;
    timeout.tv_usec = (suseconds_t)((params->timeout - timeout.tv_sec) * 1000000);

    /*
     * Get a file descriptor for this pid so we can use select() to wait for it to
     * exit
     */
    pid_fd = syscall(SYS_pidfd_open, test->sysdep.pid, 0);
    nfds = pid_fd + 1;

    do {
        FD_ZERO(&rfds);
        FD_SET(pid_fd, &rfds);
        rc = select(nfds, &rfds, NULL, NULL, &timeout);
        if (rc == -1) {
            __leavemein_fail_errno("Select failed");
        }
    } while (rc != 0 && !FD_ISSET(pid_fd, &rfds));

    __leavemein_enqueue_done(test);

    return true;
}

static bool __leavemein_mutex_init(struct __leavemein_mutex *mutex) {
    int rc;

    rc = pthread_mutex_init(&mutex->mutex, NULL);
    if (rc != 0) {
        __leavemein_fail_with(rc, "Failed to initialize mutex");
    }

    return true;
}

static bool __leavemein_mutex_lock(struct __leavemein_mutex *mutex) {
    int rc;

    rc = pthread_mutex_lock(&mutex->mutex);
    if (rc != 0) {
        __leavemein_fail_with(rc, "Failed to lock mutex");
    }

    return true;
}

static bool __leavemein_mutex_unlock(struct __leavemein_mutex *mutex) {
    int rc;

    rc = pthread_mutex_unlock(&mutex->mutex);
    if (rc != 0) {
        __leavemein_fail_with(rc, "Failed to unlock mutex");
    }

    return true;
}

static bool __leavemein_cond_init(struct __leavemein_cond *cond) {
    int rc;

    rc = pthread_cond_init(&cond->cond, NULL);
    if (rc != 0) {
        __leavemein_fail_with(rc, "Failed to initialize conditional variable");
    }

    return true;
}

static bool __leavemein_cond_signal(struct __leavemein_cond *cond) {
    int rc;

    rc = pthread_cond_signal(&cond->cond);
    if (rc != 0) {
        __leavemein_fail_with(rc, "Failed to signal conditional variable");
    }

    return true;
}

static bool __leavemein_cond_wait(struct __leavemein_cond *cond,
    struct __leavemein_mutex *mutex) {
    int rc;

    rc = pthread_cond_wait(&cond->cond, &mutex->mutex);
    if (rc != 0) {
        __leavemein_fail_with(rc, "Failed to wait conditional variable");
    }

    return true;
}

static void __leavemein_print_status(__leavemein_test *test) {
    int status = test->sysdep.exit_status;

    if (WIFEXITED(status)) {
        int exit_status;

        exit_status = WEXITSTATUS(status);

        if (exit_status == 0) {
            __leavemein_printf("exit code %d: SUCCESS", exit_status);
            __leavemein_inc_passed();
        } else {
            __leavemein_printf("exit code %d: FAILURE", exit_status);
            __leavemein_inc_failed();
        }
    } else if (WIFSIGNALED(status)) {
        __leavemein_printf("signal %d%s: FAILURE", WTERMSIG(status),
            WCOREDUMP(status) ? " (core dumped)" : "");
        __leavemein_inc_failed();
    } else if (WIFSTOPPED(status)) {
        __leavemein_printf("stopped, signal %d: FAILURE", WSTOPSIG(status));
        __leavemein_inc_failed();
    } else if (WIFCONTINUED(status)) {
        __leavemein_printf("continued: %s\n", "FAILURE");
        __leavemein_inc_failed();
    } else {
        __leavemein_printf("unknown reason: %s", "FAILURE");
        __leavemein_inc_failed();
    }
}   

static void __leavemein_exit(bool is_error) {
    exit(is_error ? EXIT_FAILURE : EXIT_SUCCESS);
}

#endif /* _LEAVEIN_TEST_LINUX_H_ */
