/*
 * Definitions for single-threaded limpet. Although this uses POSIX
 * primitives to run the test, it can serve as an example of how to test
 * an embedded system.
 *
 * Parameters can
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

/*
 * These will have to be defined in the system-dependent single threaded
 * code.
 */
static void __limpet_wait(struct __limpet_test *test);
static void __limpet_start_one(struct __limpet_test *test);
static void __limpet_start_one(struct __limpet_test *test);
static void __limpet_cleanup_test(struct __limpet_test *test);

#include "limpet.d/limpet-sysdep.h"
#include "limpet.d/limpet-posix.h"

#define __LIMPET_STRINGIFY_HELPER(token)    #token
#define __LIMPET_STRINGIFY(token)           __LIMPET_STRINGIFY_HELPER(token)

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
#ifdef LIMPET_MAX_JOBS
    return __LIMPET_STRINGIFY(LIMPET_MAX_JOBS);
#else
    return NULL;
#endif
}

static const char *__limpet_get_runlist(void) {
#ifdef LIMPET_RUNLIST
    return __LIMPET_STRINGIFY(LIMPET_RUNLIST);
#else
    return NULL;
#endif
}

static const char *__limpet_get_timeout(void) {
#ifdef LIMPET_TIMEOUT
    return __LIMPET_STRINGIFY(LIMPET_TIMEOUT);
#else
    return NULL;
#endif
}

static const char *__limpet_get_verbose(void) {
#ifdef LIMPET_VERBOSE
    return __LIMPET_STRINGIFY(LIMPET_VERBOSE);
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
 * Can be used to print a string before generating an unstored log file, i.e.
 * one that goes straight to stdout.
 *
 * Returns: the number of characters printed.
 */
static int __limpet_pre_start(struct __limpet_test *test,
    const char *sep) {
    int n;

    n = __limpet_print_test_header(test, sep);

    return n;
}

static void __limpet_post_start(struct __limpet_test *test, int n) {
    __limpet_print_test_trailer(test, n);
}

/*
 * Can be used to print a string before dumping a stored log file to stdout.
 *
 * Returns: the number of characters printed, zero in this case
 */
static int __limpet_pre_stored(struct __limpet_test *test,
    const char *sep) {
    return 0;
}

static void __limpet_post_stored(struct __limpet_test *test, int n) {
}
#endif /* _LIMPET_SINGLE_THREADED_H_ */
