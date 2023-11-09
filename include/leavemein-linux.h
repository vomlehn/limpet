/*
 * Definitions for the Linux-based Leavemein
 */

#ifndef _LEAVEIN_TEST_LINUX_H_
#define _LEAVEIN_TEST_LINUX_H_

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Value to use for __leavemein_sysdep initiatization
 */
#define __LEAVEMEIN_SYSDEP_INIT         {   \
        .log_fd = 0,                            \
        .test_fd = 0,                           \
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
    int     test_fd;
    int     bad_errno;
};

#include <leavemein-sysdep.h>

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

#define __leavemein_fail_errno(fmt, ...) {      do {        \
        __Leavemein_fail(fmt ## ": %s", strerror(error));   \
    } while (0)

static bool __leavemein_test_setup(struct __leavemein_test *test) {
    test->sysdep.log_fd = open("/tmp/leavemein.log",
        O_TMPFILE | O_CREAT | O_EXCL | O_RDWR,
        S_IRUSR | S_IWUSR);
    if (test->sysdep.log_fd == -1) {
        test->sysdep.bad_errno = errno;
        return false;
    }

    return true;
}

/*
 * Run one test
 *  test - Pointer to a __leavemein_params for the test to run
 * Returns: true if the test was started, false otherwise
 */
static bool __leavemein_run_one(struct __leavemein_test *test) {
    __leavemein_test_setup(test);

    return false;
}

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
static bool __leavemein_add_test(struct __leavemein_params *params,
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

            if (!__leavemein_add_test(params, start, colon)) {
                return false;
            }

        }

        if (*start == '\0') {
            size_t name_len;

            name_len = strlen(start);
            if (!__leavemein_add_test(params, start, start + name_len)) {
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
static bool __Leavemein_parse_params(struct __leavemein_params *params) {
    const char *max_jobs_env;

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

    return true;
}
#endif /* _LEAVEIN_TEST_LINUX_H_ */
