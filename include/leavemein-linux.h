/*
 * Definitions for the Linux-based Leavemein
 */

#ifndef _LEAVEIN_TEST_LINUX_H_
#define _LEAVEIN_TEST_LINUX_H_

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <pty.h>
#include <stdarg.h>
#include <stdbool.h>
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
        .bad_errno = 0,                         \
    }

/*
 * System-dependent per-test information.
 * log_fd - file descriptor for the log file.
 * test_fd - file descriptor on which will be written output from the test
 * bad_errno - If we fail and have an errno value, it will be stored here.
 */
struct __leavemein_sysdep {
    int     log_fd;
    int     raw_pty;
    int     tty_pty;
    int     bad_errno;
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
 * Define the initialized for a structure holding a mutex and the mutext struct itself
 */
#define __LEAVEMEIN_MUTEX_INIT   { .mutex = PTHREAD_MUTEX_INITIALIZER, }

struct __leavemein_mutex {
    pthread_mutex_t mutex;
};   

/*
 * print an error message like printf()
 */
static void __leavemein_fail(const char *fmt, ...) {
    va_list ap;
    int rc;

    va_start(ap, fmt);  
    rc = vfprintf(stderr, fmt, ap);
    if (rc < 0) {
        /* Unable to print the error message, give up */
        exit(EXIT_FAILURE);
    }
    va_end(ap);
}

#define __leavemein_fail_errno_arg(err, fmt, ...)    do {        \
        __leavemein_fail(fmt ": %s\n", ##__VA_ARGS__, strerror(errno)); \
    } while (0)

#define __leavemein_fail_errno(fmt, ...)    do {        \
        __leavemein_fail_errno_arg(errno, fmt, ##__VA_ARGS__); \
    } while (0)

/*
 * Update the status upon completion of a test
 * is_error - true if test failed, false otherwise
 */
static void __leavemein_update_status(bool is_error) {
}

/*
 * Function called by tests to terminate execution
 *  is_error - true if an error occured, false otherwise
 */
static void __leavemein_test_exit(bool is_error) {
    if (is_error) {
        exit(EXIT_FAILURE);
    } else {
        exit(EXIT_SUCCESS);
    }
}

/*
 * Function called by tests to terminate execution
 *  is_error - true if an error occured, false otherwise
 */
static void __leavemein_run_exit(bool is_error) {
    __leavemein_update_status(is_error);

    if (is_error) {
        exit(EXIT_FAILURE);
    } else {
        exit(EXIT_SUCCESS);
    }
}

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
    size_t new_runlist_size;

    testname_size = start - end + 1;        /* string plus terminator NUL */
    testname = (char *)malloc(testname_size);
    if (testname == NULL) {
        __leavemein_fail("Out of memory allocating %zu bytes for test name\n",
            testname_size);
        return false;
    }

    strncpy(testname, start, testname_size);

    new_runlist_size = sizeof(params->runlist[0]) * (params->n_runlist);
    params->runlist = (char **)realloc(params->runlist, new_runlist_size);
    if (params->runlist == NULL) {
        __leavemein_fail("Unable to realloc %zu bytes\n", new_runlist_size);
        return false;
    }

    params->runlist[params->n_runlist] = testname;
    params->n_runlist += 1;
    return true;
}

/*
 * Parse the runlist, if any
 * params - Pointer to the configuration structure
 *
 * Returns: true on success, false otherwise
 */
static bool parse_runlist(struct __leavemein_params *params) {
    const char *runlist_env;

    runlist_env = getenv(__LEAVEMEIN_RUNLIST);

    if (runlist_env != NULL) {
        const char *start;
        const char *colon;

        params->runlist = (char **)malloc(0);
        if (params->runlist == NULL) {
            __leavemein_fail("Out of memory allocating runlist\n");
            return false;
        }

        start = runlist_env;

        for (colon = strchr(start, ':'); colon != NULL;
            start = colon + 1, colon = strchr(start, ':')) {

            if (start == colon) {
                __leavemein_fail("Zero length test name invalid in %s\n",
                    __LEAVEMEIN_RUNLIST);
                return false;
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

    memset(params, 0, sizeof(*params));

    max_jobs_env = getenv(__LEAVEMEIN_MAX_JOBS);
    if (max_jobs_env != NULL) {
        char *endptr;
        params->max_jobs = strtoul(max_jobs_env, &endptr, 0);
        if (*max_jobs_env == '\0' || *endptr != '\0') {
            __leavemein_fail("Invalid value specified for %s\n",
                __LEAVEMEIN_MAX_JOBS);
            return false;
        }
    }

    if (!parse_runlist(params)) {
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
            return false;
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
        return false;
    }

    rc = openpty(&sysdep->raw_pty, &sysdep->tty_pty, NULL, &termios, &winsize);
    if (rc == -1) {
        __leavemein_fail_errno("Unable to create pty");
        return false;
    }

    return true;
}

static bool __leavemein_test_setup(struct __leavemein_test *test) {
    size_t i;

    test->sysdep.log_fd = open("/tmp/leavemein.log",
        O_TMPFILE | O_CREAT | O_EXCL | O_RDWR,
        S_IRUSR | S_IWUSR);
    if (test->sysdep.log_fd == -1) {
        __leavemein_fail_errno("Unable to make log file");
        test->sysdep.bad_errno = errno;
        return false;
    }

    if (!__leavemein_make_pty(&test->sysdep)) {
        return false;
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
            return false;
        }
    }

    return true;
}

/* FIXME: remove this
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_init(pthread_mutex_t * mutex,
       const pthread_mutexattr_t * attr);
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t foo_mutex = PTHREAD_MUTEX_INITIALIZER;

void foo()
{
    pthread_mutex_lock(&foo_mutex);
    pthread_mutex_unlock(&foo_mutex);
}
*/

/*
 * Run one test
 *  test - Pointer to a __leavemein_params for the test to run
 * Returns: true if the test was started, false otherwise
 */
static bool __leavemein_run_one(struct __leavemein_test *test) {
    __leavemein_test_setup(test);

    return false;
}

static bool __leavemein_mutex_init(struct __leavemein_mutex *mutex) {
    int rc;

    rc = pthread_mutex_init(&mutex->mutex, NULL);
    if (rc != 0) {
        __leavemein_fail_errno_arg(rc, "Failed to lock mutex");
        return false;
    }

    return true;
}

static bool __leavemein_lock(struct __leavemein_mutex *mutex) {
    int rc;

    rc = pthread_mutex_lock(&mutex->mutex);
    if (rc != 0) {
        __leavemein_fail_errno_arg(rc, "Failed to lock mutex");
        return false;
    }

    return true;
}

static bool __leavemein_unlock(struct __leavemein_mutex *mutex) {
    int rc;

    rc = pthread_mutex_unlock(&mutex->mutex);
    if (rc != 0) {
        __leavemein_fail_errno_arg(rc, "Failed to lock mutex");
        return false;
    }

    return true;
}
#endif /* _LEAVEIN_TEST_LINUX_H_ */
