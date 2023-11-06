/*
 * Definitions for the Linux-based Leavemein
 */

#ifndef _LEAVEIN_TEST_LINUX_
#define _LEAVEIN_TEST_LINUX_

#include <stdbool.h>
#include <stdlib.h>

/*
 * There doesn't appear to be any non-intrusive way for the functions in
 * one file to call functions in another file. So, we use environment
 * variables. This means we will be doing memory allocations on start up,
 * but not afterwards. Since there is no locking available, we rely on
 * the assumption that constructors are called in a single-threaded
 * fashion. This should be a pretty good assumption.
 *
 * Environment variables used are:
 * __LEAVEMEIN_STATUS   The number of tests run and the number of failures as
 *                      decimal numeric values, separated by commas
 * __LEAVEMEIN_FD       A file descriptor used to ???
 * ...
 */
#define __LEAVEMEIN_STATUS  "__LEAVEMEIN_STATUS"
#define __LEAVEMEIN_FD      "__LEAVEMEIN_FD"

/*
 * System-dependent per-test information.
 *  log_fd - file descriptor for the log file.
 */
struct __leavein_sysdep {
    int     log_fd;
};

/*
 * Run one test
 *  test - Pointer to a __leavein_common for the test to run
 * Returns: true if the test was started, false otherwise
 */
static bool __leavemein_run_one(struct __leavemein_common *test) {
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
        exit(EXIT_SUCCESS);
    } else {
        exit(EXIT_FAILURE);
    }
}

/*
 * Function called by tests to terminate execution
 *  is_error - true if an error occured, false otherwise
 */
static void __leavemein_run_exit(bool is_error) {
    __leavemein_update_status(is_error);

    if (is_error) {
        exit(EXIT_SUCCESS);
    } else {
        exit(EXIT_FAILURE);
    }
}
#endif /* _LEAVEIN_TEST_LINUX_ */
